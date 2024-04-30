#include "te_command.hpp"

#include "war_sim.hpp"
#include <iostream>

namespace te {
    TeCommandThread::TeCommandThread(TheEngine& env_) : env{ env_ } {
        internalThread = std::thread(&TeCommandThread::threadFunction, this);
    }

    TeCommandThread::~TeCommandThread() {
        shutdownMutex.lock();
        shutingDown = true;
        shutdownMutex.unlock();
        internalThread.join();
    }

    void TeCommandThread::registerCommand(std::function<const char* (std::vector<std::string> args, TheEngine&)> function, std::string name) {
        commandsMutex.lock();
        commands[name] = function;
        commandsMutex.unlock();
    }

    const char* TeCommandThread::executeCommand(std::string name, std::vector<std::string> args) {
        commandsMutex.lock();
        auto function = commands.find(name);
        if (function == commands.end()) {
            commandsMutex.unlock();
            return "Command not found";
        }
        const char* result = function->second(args, env);
        commandsMutex.unlock();
        return result;
    }

    void TeCommandThread::threadFunction() {
        while (true) {
            std::string commandWithArgs;
            std::getline(std::cin, commandWithArgs);
            std::vector<std::string> seperatedCommandWithArgs{};
            std::vector<std::string> args;
            std::string commandName;

            shutdownMutex.lock();
            if (shutingDown) {
                shutdownMutex.unlock();
                break;
            }
            shutdownMutex.unlock();

            // Split command and arguments
            seperatedCommandWithArgs = splitString(commandWithArgs);

            if (seperatedCommandWithArgs.size() == 0) {
				continue;
			}

			commandName = seperatedCommandWithArgs[0];
            seperatedCommandWithArgs.erase(seperatedCommandWithArgs.begin());
			for (auto& arg : seperatedCommandWithArgs) {
				args.push_back(arg.c_str());
			}
            
            std::cout << executeCommand(commandName, args) << std::endl;
		}
    }

    std::vector<std::string> TeCommandThread::splitString(const std::string& str) {
        std::vector<std::string> words;
        std::string::size_type start = 0;
        std::string::size_type end = str.find_first_of(' ');

        while (end != std::string::npos) {
            words.push_back(str.substr(start, end - start));
            start = end + 1;
            end = str.find_first_of(' ', start);
        }

        // Push the last word
        words.push_back(str.substr(start));

        return words;
    }

    std::unordered_map<std::string, std::function<const char* (std::vector<std::string>, TheEngine&)>> TeCommandThread::getCommands() {
		return commands;
	}

    const char* TeCommandThread::command_spawn(std::vector<std::string> args, TheEngine& env) {
        if ((args.size() < 1) || (args.size() > 1)) {
            return "Usage: spawn <name>";
        }

        auto scene = env.scene;

        TransformComponent* playerTransform = scene->getComponent<TransformComponent>(scene->getEntityByName("camera_1"));

        std::shared_ptr<TeModel> cubeModel = scene->getComponent<ModelComponent>(scene->getEntityByName("cube_1"))->model;
        auto cube = scene->createEntity(args[0]);
        scene->addComponent<ModelComponent>(cube, { cubeModel, });
        scene->addComponent<TransformComponent>(cube, { playerTransform->translation, playerTransform->scale, playerTransform->rotation });

        return "Entity spawned";
    }

    const char* TeCommandThread::command_log(std::vector<std::string> args, TheEngine& env) {
        env.logger.log();
        return "";
    }
}