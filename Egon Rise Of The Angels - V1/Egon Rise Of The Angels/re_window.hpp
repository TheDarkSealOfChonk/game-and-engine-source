#pragma once

#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace te {
	class TeWindow {
		public:
			TeWindow(int w, int h, std::string name);
			~TeWindow();
			void initWindow();
			GLFWwindow* window;
			TeWindow(const TeWindow&) = delete;
			TeWindow &operator=(const TeWindow&) = delete;
			VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
			bool wasWindowResized() { return framebufferResized; }
			void resetWindowResizedFlag() { framebufferResized = false; }
			GLFWwindow* getGLFWwindow() const { return window; }
		private:
			// needs update for migration away from GLFW
			static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
			int width;
			int height;
			bool framebufferResized = false;
			std::string windowName;
	};
}