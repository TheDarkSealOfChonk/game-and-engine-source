#pragma once

namespace te {
	class TheEngine;

	class TeLogger {
	public:
		TeLogger(TheEngine& env_);

		void run();
		void log();

		bool hasMovedSinceLastLog = false;
	private:
		TheEngine& env;
		std::mutex logMutex;
		bool shouldLog = false;
	};
}