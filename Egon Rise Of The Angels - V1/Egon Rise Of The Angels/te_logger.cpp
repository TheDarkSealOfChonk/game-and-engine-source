#include "war_sim.hpp"
#include "te_logger.hpp"

namespace te {
	TeLogger::TeLogger(TheEngine& env_) : env{env_} {}

	void TeLogger::run() {
        logMutex.lock();
        if (shouldLog) {
            printf("Has camera moved since last log: %s\n", hasMovedSinceLastLog ? "true" : "false");
            printf("Entities:\n");
            for (auto& entity : env.scene->getEntities()) {
                printf("Entity: %s\n", env.scene->getEntityName(entity).c_str());
                auto transform = env.scene->getComponent<TransformComponent>(entity);
                if (transform) {
                    printf("Transform: %f %f %f\n", transform->translation.x, transform->translation.y, transform->translation.z);
                }
                auto mesh = env.scene->getComponent<ModelComponent>(entity);
                printf("Mesh: %s\n", mesh ? "true" : "false");
            }

            printf("Registered commands:\n");
            for (auto& command : env.commandThread.getCommands()) {
                printf("Command: %s\n", command.first.c_str());
            }

            shouldLog = false;
            hasMovedSinceLastLog = false;
        }
        logMutex.unlock();
	}

	void TeLogger::log() {
        logMutex.lock();
        shouldLog = true;
        logMutex.unlock();
	}
}