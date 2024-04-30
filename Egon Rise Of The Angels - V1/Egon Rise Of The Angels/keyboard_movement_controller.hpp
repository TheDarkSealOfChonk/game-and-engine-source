#pragma once

#include "te_game_object.hpp"
#include "te_frame_info.hpp"
#include "re_window.hpp"
#include "te_physics.hpp"
#include "te_command.hpp"

namespace te {
    class KeyboardMovementController {
    public:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_SPACE;
            int moveDown = GLFW_KEY_LEFT_SHIFT;
            int moveFast = GLFW_KEY_LEFT_CONTROL;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
            int toggleGame = GLFW_KEY_ESCAPE;
            int spawnCube = GLFW_KEY_C;
        };

        KeyboardMovementController(GLFWwindow* window);

        void moveInPlaneXZ(FrameInfo frameInfo, TeScene::Entity gameObject, bool& hasMoved);

        GLFWwindow* window;
        KeyMappings keys{};
        float normalMoveSpeed{ 3.f };
        float fastMoveSpeed{ 24.f };
        float lookSpeed{ 1.5f };
        double lastMouseX{ 0.0 };
        double lastMouseY{ 0.0 };
        bool inGame{ true };
        bool escPressedLastFrame{ false };
        bool spawnCubePressedLastFrame{ false };
    };
}