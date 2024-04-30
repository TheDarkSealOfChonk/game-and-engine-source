#pragma once

#include <string>
#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "re_window.hpp"
#include "te_device.hpp"
#include "te_game_object.hpp"
#include "te_renderer.hpp"
#include "te_descriptors.hpp"
#include "te_command.hpp"
#include "te_logger.hpp"

namespace te {
	class TheEngine {
	public:
		void run();
		
		TheEngine();
		~TheEngine();
		TheEngine(const TheEngine&) = delete;
		void operator=(const TheEngine&) = delete;

		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		void loadGameObjects();
		void registerComponents();
		void registerCommmands();

		TeCommandThread commandThread{ *this };
		TeWindow teWindow{ WIDTH, HEIGHT, "engine test" };
		TeDevice teDevice{ teWindow };
		TeRenderer teRenderer{ teWindow, teDevice };
		std::unique_ptr<TeDescriptorPool> globalPool{};
		TeECS manager{};
		TeScene* scene;
		TeLogger logger{ *this };
	};
}