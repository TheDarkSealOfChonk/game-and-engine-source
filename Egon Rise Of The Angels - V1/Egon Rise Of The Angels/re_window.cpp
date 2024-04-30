#include "re_window.hpp"

namespace te {

	TeWindow::TeWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
		initWindow();
	}
	void TeWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}
	TeWindow::~TeWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	void TeWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto teWindow = reinterpret_cast<TeWindow*>(glfwGetWindowUserPointer(window));
		teWindow->framebufferResized = true;
		teWindow->width = width;
		teWindow->height = height;
	}
}