#pragma once
#pragma once

#include <unordered_map>
#include <functional>
#include <string>
#include <mutex>

#include "te_game_object.hpp"

namespace te {
	class TheEngine;

	class TeCommandThread {
	public:
		TeCommandThread(TheEngine& env_);
		~TeCommandThread();
		void registerCommand(std::function<const char* (std::vector<std::string>, TheEngine&)> function, std::string name);
		const char* executeCommand(std::string name, std::vector<std::string> args);
		void threadFunction();

		std::vector<std::string> splitString(const std::string& str);

		std::unordered_map<std::string, std::function<const char* (std::vector<std::string>, TheEngine&)>> getCommands();

		static const char* command_spawn(std::vector<std::string> args, TheEngine& env);
		static const char* command_log(std::vector<std::string> args, TheEngine& env);
	private:
		std::unordered_map<std::string, std::function<const char* (std::vector<std::string>, TheEngine&)>> commands;
		std::mutex commandsMutex;

		bool shutingDown = false;
		std::mutex shutdownMutex;

		TheEngine& env;
		std::thread internalThread;
	};
}