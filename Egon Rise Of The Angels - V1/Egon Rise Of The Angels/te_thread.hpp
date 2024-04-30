#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace teutils {
	class Thread {
	public:
		static void init() {
			numUsedThreads = 0;
			threadPool = std::vector<std::thread>();
			for (size_t i = 0; i < threadPool.size(); i++) {
				threadPool[i] = std::thread(idleThread);
			}
			threadPoolRefiller = std::thread(threadPoolRefillerFunction);
		}

		static void clean() {
			threadPoolRefiller.~thread();
			for (uint32_t i = 0; i < threadPool.size(); i++) {
				threadPool[i].~thread();
			}
		}

		template <typename F, typename... Args>
		Thread(F&& func, Args&&... args) : func(std::bind(std::forward<F>(func), std::forward<Args>(args)...)) {
			std::unique_lock<std::mutex> lock1(threadPoolRefillMutex);
			if (threadPool.getSize() <= numUsedThreads) {
				for (size_t i = numUsedThreads; i < numUsedThreads + 5; i++) {
					threadPool[i] = std::thread(idleThread, i);
				}
			}
			std::unique_lock<std::mutex> lock2(queueMutex);
			tasks.push({ func, numUsedThreads });
			condition.notify_one();
			numUsedThreads++;
			lock1.unlock();
		}

		~Thread() {
			threadPool[threadIndex].~thread();
			threadPool.erase(threadPool.begin() + threadIndex);
		}

		void join() {
			if (threadPool[threadIndex].joinable()) {
				threadPool[threadIndex].join();
			}
		}
	private:
		struct Task {
			std::function<void()> func;
		};

		static void idleThread() {
			while (true) {
				std::unique_lock<std::mutex> lock1(queueMutex);

				// Wait for tasks if the queue is empty
				condition.wait(lock1, [] { return !tasks.empty(); });

				Task task = tasks.front();
				tasks.pop_back();
				lock1.unlock();

				task.func();
			}
		}

		static void threadPoolRefillerFunction() { 
			while (true) {
				std::unique_lock<std::mutex> lock1(threadPoolRefillMutex);
				if ((threadPool.size() - numUsedThreads) <= 5) {
					for (size_t i = numUsedThreads; i < numUsedThreads + 5; i++) {
						threadPool[i] = std::thread(idleThread, i);
					}
				}
				lock1.unlock();
			}
		}

		static std::vector<std::thread> threadPool;
		static size_t numUsedThreads;
		static std::thread threadPoolRefiller;
		static std::mutex threadPoolRefillMutex;
		
		static std::mutex queueMutex;
		static std::condition_variable condition;
		static std::vector<Task> tasks;
		
		std::function<void()> func;
		uint32_t threadIndex;
	};
}