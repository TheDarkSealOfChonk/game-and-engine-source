#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "te_model.hpp"
#include <unordered_map>
#include <memory>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <string>
#include <thread>
#include <mutex>
#include "re_pipeline.hpp"
#include "te_physics.hpp"

#include "test_class.hpp"

namespace te {
	class TeECS;

	class TeScene {
	public:
		using Entity = size_t;

		TeScene(TeECS& manager) : manager(manager) {}

		Entity createEntity(std::string);

		Entity duplicateEntity(Entity entity, std::string newName);

		void destroyEntity(TeScene::Entity& entity);

		std::string getEntityName(TeScene::Entity entity);

		TeScene::Entity getEntityByName(std::string name);

		std::vector<char> serializeEntity(TeScene::Entity entity);

		TeScene::Entity deserializeEntity(std::vector<char> data);

		TeScene::Entity loadEntityFromFile(std::string path);

		TeECS& iWouldLikeToSpeakToYourManager() { return manager; }

		void saveEntityToFile(TeScene::Entity entity, std::string path);

		template<typename T>
		T* getComponent(TeScene::Entity entity);

		template<typename T>
		void addComponent(TeScene::Entity entity, T&& component);

		template<typename T>
		void removeComponent(TeScene::Entity entity);

		template<typename T>
		std::unordered_map<TeScene::Entity, T*> getComponentInstances();

		std::vector<Entity> getEntities() { return entities; }
	private:
		std::mutex sceneMutex;

		Entity nextEntityId = 0;

		std::vector<TeScene::Entity> entities;

		std::unordered_map<Entity, std::string> entitiesToNames;
		std::unordered_map<std::string, Entity> namesToEntities;

		std::unordered_map<std::type_index, std::unordered_map<Entity, void*>> componentStorage;

		TeECS& manager;
	};

	class TeECS {
	public:
		struct RegisteredComponent {
			std::function<std::string(void*)> loggerTextFunc;
			std::function<std::vector<char>(void*)> serializeFunc;
			std::function<void* (std::vector<char>)> deserializeFunc;
			std::type_index type;
		};

		size_t getIdByType(std::type_index type);
		std::vector<RegisteredComponent>& getRegisteredComponents() { return registeredComponents_; }
		std::mutex& getMutex() { return ecsMutex_; }
		size_t typeToComponentId(std::type_index type) { return typeToComponentId_[type]; }

		std::pair<void*, size_t> deserializeComponentNOLOCK(std::vector<char> data);

		std::vector<char> serializeComponentNOLOCK(void* component, size_t id);

		std::pair<void*, size_t> deserializeComponent(std::vector<char> data);

		std::vector<char> serializeComponent(void* component, size_t id);

		std::string getComponentLoggerText(void* component, size_t id);

		size_t createScene();

		TeScene* getScene(size_t id) { return scenes_[id]; }

		std::vector<TeScene*>& getScenes() { return scenes_; }

		template<typename T>
		size_t registerComponent();
	private:
		std::vector<TeScene*> scenes_;

		std::unordered_map<std::type_index, size_t> typeToComponentId_;

		std::vector<RegisteredComponent> registeredComponents_;

		std::mutex ecsMutex_;
	};
	
	// Template function definitions
	template<typename T>
	void TeScene::addComponent(Entity entity, T&& component) {
		sceneMutex.lock();
		componentStorage[typeid(T)][entity] = new T(std::forward<T>(component));
		sceneMutex.unlock();
	}

	template<typename T>
	void TeScene::removeComponent(Entity entity) {
		sceneMutex.lock();
		auto& componentMap = componentStorage[typeid(T)];
		delete static_cast<T*>(componentMap[entity]);
		componentMap.erase(entity);
		sceneMutex.unlock();
	}

	template<typename T>
	std::unordered_map<TeScene::Entity, T*> TeScene::getComponentInstances() {
		sceneMutex.lock();
		std::unordered_map<Entity, T*> instances;
		auto& componentMap = componentStorage[typeid(T)];
		for (auto& [entity, ptr] : componentMap) {
			instances[entity] = static_cast<T*>(ptr);
		}
		sceneMutex.unlock();
		return instances;
	}

	template<typename T>
	size_t TeECS::registerComponent() {
		ecsMutex_.lock();
		size_t nextId = registeredComponents_.size() + 1;

		registeredComponents_[nextId].deserializeFunc = &T::deserialize;
		registeredComponents_[nextId].serializeFunc = &T::serialize;
		registeredComponents_[nextId].loggerTextFunc = &T::loggerText;

		registeredComponents_[nextId].type = typeid(T);
		ecsMutex_.unlock();
		return nextId;
	}

	template<typename T>
	T* TeScene::getComponent(Entity entity) {
		sceneMutex.lock();
		auto& componentMap = componentStorage[typeid(T)];
		auto it = componentMap.find(entity);
		if (it != componentMap.end()) {
			void* component = componentMap.at(entity);
			sceneMutex.unlock();
			return static_cast<T*>(component);
		}
		else {
			sceneMutex.unlock();
			return nullptr;
		}
	}

	struct TransformComponent {
		static std::string loggerText(void* component_) {
			return "TransformComponent";
		}

		static std::vector<char> serialize(void* component_) {
			std::vector<char> output{};

			TransformComponent* component = (TransformComponent*)component_;

			output.push_back(reinterpret_cast<char*>(&component->translation.x)[0]);
			output.push_back(reinterpret_cast<char*>(&component->translation.y)[1]);
			output.push_back(reinterpret_cast<char*>(&component->translation.z)[2]);

			output.push_back(reinterpret_cast<char*>(&component->scale.x)[0]);
			output.push_back(reinterpret_cast<char*>(&component->scale.y)[1]);
			output.push_back(reinterpret_cast<char*>(&component->scale.z)[2]);

			output.push_back(reinterpret_cast<char*>(&component->rotation.x)[0]);
			output.push_back(reinterpret_cast<char*>(&component->rotation.y)[1]);
			output.push_back(reinterpret_cast<char*>(&component->rotation.z)[2]);

			return output;
		}

		static void* deserialize(std::vector<char> data) {
			TransformComponent* component = new TransformComponent();

			component->translation.x = reinterpret_cast<float*>(&data[0])[0];
			component->translation.y = reinterpret_cast<float*>(&data[3])[0];
			component->translation.z = reinterpret_cast<float*>(&data[6])[0];

			component->scale.x = reinterpret_cast<float*>(&data[9])[0];
			component->scale.y = reinterpret_cast<float*>(&data[12])[0];
			component->scale.z = reinterpret_cast<float*>(&data[15])[0];

			component->rotation.x = reinterpret_cast<float*>(&data[18])[0];
			component->rotation.y = reinterpret_cast<float*>(&data[21])[0];
			component->rotation.z = reinterpret_cast<float*>(&data[24])[0];

			return (void*) component;
		}

		glm::vec3 translation{};
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{ 0.f, 0.f, 0.f };

		glm::mat3 normalMatrix() {
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);

			const glm::vec3 inverseScale = 1.f / scale;

			return glm::mat3{
				{
					inverseScale.x * (c1 * c3 + s1 * s2 * s3),
					inverseScale.x * (c2 * s3),
					inverseScale.x * (c1 * s2 * s3 - c3 * s1),
				},
				{
					inverseScale.y * (c3 * s1 * s2 - c1 * s3),
					inverseScale.y * (c2 * c3),
					inverseScale.y * (c1 * c3 * s2 + s1 * s3),
				},
				{
					inverseScale.z * (c2 * s1),
					inverseScale.z * (-s2),
					inverseScale.z * (c1 * c2),
				} };
		}

		glm::mat4 mat4() {
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			return glm::mat4{
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{translation.x, translation.y, translation.z, 1.0f} };
		}
	};

	struct ModelComponent {
		std::shared_ptr<TeModel> model{};
	};
}