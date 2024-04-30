#include "keyboard_movement_controller.hpp"
#include "te_frame_info.hpp"
#include <limits>

namespace te {
	KeyboardMovementController::KeyboardMovementController(GLFWwindow* window_) : window{window_} {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}

	void KeyboardMovementController::moveInPlaneXZ(FrameInfo frameInfo, TeScene::Entity gameObject, bool& hasMoved) {
		TransformComponent* objTransform = frameInfo.scene->getComponent<TransformComponent>(gameObject);

		bool escPressed = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

		if (!escPressedLastFrame && escPressed) {
			inGame = !inGame;
			glfwSetInputMode(window, GLFW_CURSOR, inGame ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
		}

		escPressedLastFrame = escPressed;

		if (inGame) {
			double mouseX, mouseY;
			glfwGetCursorPos(window, &mouseX, &mouseY);

			double mouseXDiff = mouseX - 800;
			double mouseYDiff = 450 - mouseY;

			glfwSetCursorPos(window, 800, 450);

			objTransform->rotation.y += static_cast<float>(mouseXDiff / 64);
			objTransform->rotation.x += static_cast<float>(mouseYDiff / 64);

			objTransform->rotation.x = glm::clamp(objTransform->rotation.x, -1.5f, 1.5f);
			objTransform->rotation.y = glm::mod(objTransform->rotation.y, glm::two_pi<float>());

			float yaw = objTransform->rotation.y;
			float pitch = objTransform->rotation.x;
			const glm::vec3 forwardDir{ glm::sin(yaw), 0.f, glm::cos(yaw) };
			const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
			const glm::vec3 upDir{ 0.f, -1.f, 0.f };
			glm::vec3 moveDir{ 0.f };
			float moveSpeed = 0.f;

			if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) { moveDir += forwardDir; }
			if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) { moveDir -= forwardDir; }
			if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) { moveDir += rightDir; }
			if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) { moveDir -= rightDir; }
			if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) { moveDir += upDir; }
			if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) { moveDir -= upDir; }

			if (glfwGetKey(window, keys.moveFast) == GLFW_PRESS) {
				moveSpeed = fastMoveSpeed;
			}
			else {
				moveSpeed = normalMoveSpeed;
			}

			if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
				objTransform->translation += moveSpeed * frameInfo.frameTime * glm::normalize(moveDir);
			}

			if (moveDir != glm::vec3(0, 0, 0)) hasMoved = true;

			bool spawnCubePressed = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
			if (spawnCubePressed && !spawnCubePressedLastFrame) {
				
			}
			spawnCubePressedLastFrame = spawnCubePressed;
		}
	}
}