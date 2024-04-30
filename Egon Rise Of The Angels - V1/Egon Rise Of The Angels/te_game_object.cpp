#include "te_game_object.hpp"

namespace te {
    // Create entity
    TeScene::Entity TeScene::createEntity(std::string name) {
        sceneMutex.lock();
        entities.push_back(nextEntityId++);
        namesToEntities[name] = entities.back();
        entitiesToNames[entities.back()] = name;
        Entity entity = entities.back();
        printf("Created entity %s\n", entitiesToNames[entity].c_str());
        sceneMutex.unlock();
        return entity;
    }

    TeScene::Entity TeScene::duplicateEntity(TeScene::Entity entity, std::string newName) {
        sceneMutex.lock();
        if (entitiesToNames.find(entity) == entitiesToNames.end()) {
			sceneMutex.unlock();
			return -1;
		}
		entities.push_back(nextEntityId++);
        
        namesToEntities[newName] = entities.back();
		entitiesToNames[entities.back()] = newName;
        std::cout << newName << std::endl;
		Entity newEntity = entities.back();
        for (auto& [type, componentMap] : componentStorage) {
            if (componentMap.find(entity) != componentMap.end()) {
				componentMap[newEntity] = componentMap[entity];
			}
		}

		sceneMutex.unlock();
		return newEntity;
    }

    // Destroy entity
    void TeScene::destroyEntity(TeScene::Entity& entity) {
        sceneMutex.lock();
        entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
        // Clean up components
        for (auto& [_, componentMap] : componentStorage) {
            componentMap.erase(entity);
        }
        namesToEntities.erase(entitiesToNames[entity]);
        entitiesToNames.erase(entity);
        sceneMutex.unlock();
    }

    // get entity name
    std::string TeScene::getEntityName(TeScene::Entity entity) {
        sceneMutex.lock();
        std::string name = entitiesToNames[entity];
        sceneMutex.unlock();
		return name;
	}

    TeScene::Entity TeScene::getEntityByName(std::string name) {
		sceneMutex.lock();
		Entity entity = namesToEntities[name];
		sceneMutex.unlock();
		return entity;
	}

    size_t TeECS::getIdByType(std::type_index type) {
        ecsMutex_.lock();
        size_t output = typeToComponentId(type);
        ecsMutex_.unlock();
        return output;
    }

    std::pair<void*, size_t> TeECS::deserializeComponentNOLOCK(std::vector<char> data) {
		size_t id = *reinterpret_cast<size_t*>(data.data());
		auto deserializer = registeredComponents_[id].deserializeFunc;
        data.erase(data.begin(), data.begin() + sizeof(size_t));
		void* component = deserializer(data);
		return std::make_pair(component, id);
    }

    std::vector<char> TeECS::serializeComponentNOLOCK(void* component, size_t id) {
        auto serializer = registeredComponents_[id].serializeFunc;
        return serializer(component);
    }

    std::pair<void*, size_t> TeECS::deserializeComponent(std::vector<char> data) {
	    ecsMutex_.lock();
		std::pair<void*, size_t> output = deserializeComponentNOLOCK(data);
		ecsMutex_.unlock();
		return output;
	}

    std::vector<char> TeECS::serializeComponent(void* component, size_t id) {
        ecsMutex_.lock();
        std::vector<char> output = serializeComponentNOLOCK(component, id);
        ecsMutex_.unlock();
        return output;
    }

    std::string TeECS::getComponentLoggerText(void* component, size_t id) {
        ecsMutex_.lock();
		auto logger = registeredComponents_[id].loggerTextFunc;
		std::string output = logger(component);
        ecsMutex_.unlock();
        return output;
    }

    size_t TeECS::createScene() {
        ecsMutex_.lock();
		scenes_.push_back(new TeScene(*this));
		size_t output = scenes_.size() - 1;
		ecsMutex_.unlock();
		return output;
    }

    std::vector<char> TeScene::serializeEntity(TeScene::Entity entity) {
		manager.getMutex().lock();
		sceneMutex.lock();
		std::vector<char> output;
        
        std::string name = entitiesToNames[entity];
		size_t nameSize = name.size();
		output.insert(output.end(), reinterpret_cast<char*>(&nameSize), reinterpret_cast<char*>(&nameSize) + sizeof(size_t));
		output.insert(output.end(), name.begin(), name.end());
		
		for (auto& [type, componentMap] : componentStorage) {
			if (componentMap.find(entity) != componentMap.end()) {
                size_t componentId = manager.typeToComponentId(type);
				std::vector<char> componentData = manager.serializeComponentNOLOCK(componentMap[entity], componentId);
                size_t componentDataSize = componentData.size() + (1 * sizeof(size_t));

                output.insert(output.end(), reinterpret_cast<char*>(&componentDataSize), reinterpret_cast<char*>(&componentDataSize) + sizeof(size_t));
                output.insert(output.end(), reinterpret_cast<char*>(&componentId), reinterpret_cast<char*>(&componentId) + sizeof(size_t));
				output.insert(output.end(), componentData.begin(), componentData.end());
			}
		}
		sceneMutex.unlock();
        manager.getMutex().unlock();
		return output;
	}

    TeScene::Entity TeScene::deserializeEntity(std::vector<char> data) {
		std::string name;
		size_t nameSize = *reinterpret_cast<size_t*>(data.data());
		data.erase(data.begin(), data.begin() + sizeof(size_t));
		name = std::string(data.begin(), data.begin() + nameSize);
		data.erase(data.begin(), data.begin() + nameSize);
		Entity entity = createEntity(name);

        manager.getMutex().lock();
        sceneMutex.lock();
		while (!data.empty()) {
			size_t componentDataSize = *reinterpret_cast<size_t*>(data.data());
			data.erase(data.begin(), data.begin() + sizeof(size_t));

			size_t componentId = *reinterpret_cast<size_t*>(data.data());
			data.erase(data.begin(), data.begin() + sizeof(size_t));
			std::vector<char> componentData(data.begin(), data.begin() + componentDataSize - sizeof(size_t));
			data.erase(data.begin(), data.begin() + componentDataSize - sizeof(size_t));

			std::pair<void*, size_t> component = manager.deserializeComponentNOLOCK(componentData);
			
            componentStorage[manager.getRegisteredComponents()[componentId].type][entity] = component.first;
		}
		sceneMutex.unlock();
		manager.getMutex().unlock();

  		return entity;
	}

    TeScene::Entity TeScene::loadEntityFromFile(std::string path) {
        std::vector<char> data = TePipeline::readFile(path);
		return deserializeEntity(data);
	}

    void TeScene::saveEntityToFile(TeScene::Entity entity, std::string path) {
        std::vector<char> data = serializeEntity(entity);
        TePipeline::writeToFile(path, data);
    }
}