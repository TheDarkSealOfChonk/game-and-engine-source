//#define DEBUG

#include <stdexcept>
#include <memory>
#include <array>
#include <chrono>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "war_sim.hpp"

#include "keyboard_movement_controller.hpp"
#include "te_camera.hpp"
#include "te_buffer.hpp"
#include "te_frame_info.hpp"
#include "simple_render_system.hpp"
#include "te_game_object.hpp"
#include "te_texture.hpp"
#include "te_physics.hpp"

namespace te {
    using id_t = unsigned int;

    TheEngine::TheEngine() {
        globalPool =
            TeDescriptorPool::Builder(teDevice)
            .setMaxSets(TeSwapChain::MAX_FRAMES_IN_FLIGHT * 2)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, TeSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, TeSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
    }

    TheEngine::~TheEngine() {}

    void TheEngine::run() {
        auto sceneIndex = manager.createScene();
        scene = manager.getScene(sceneIndex);

        std::vector<std::unique_ptr<TeBuffer>> uboBuffers(TeSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<TeBuffer>(
                teDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout =
            TeDescriptorSetLayout::Builder(teDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        Texture texture{ teDevice, "textures\\texture.jpg" };

        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = texture.getSampler();
        imageInfo.imageView = texture.getImageView();
        imageInfo.imageLayout = texture.getImageLayout();

        std::vector<VkDescriptorSet> globalDescriptorSets(TeSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            TeDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .build(globalDescriptorSets[i]);
        }

        TeCamera camera{};
        auto currentTime = std::chrono::high_resolution_clock::now();
        float aspect = teRenderer.getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
        
        // systems
        KeyboardMovementController cameraController{ teWindow.getGLFWwindow() };
        SimpleRenderSystem simpleRenderSystem{
            teDevice,
            teRenderer.getSwapChainRenderPass(),
            globalSetLayout,
            globalPool
        };

        registerComponents();

        loadGameObjects();

        registerCommmands();

        while (!glfwWindowShouldClose(teWindow.getGLFWwindow())) {
            auto commandBuffer = teRenderer.beginFrame();

            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime =
                std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            int frameIndex = teRenderer.getFrameIndex();
            FrameInfo frameInfo{
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex],
                scene
            };

            float oldAspect = aspect;
            aspect = teRenderer.getAspectRatio();
            if (oldAspect != aspect) {
                camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
            }

            if (commandBuffer) {
                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();
                logger.run();
                cameraController.moveInPlaneXZ(frameInfo, scene->getEntityByName("camera_1"), logger.hasMovedSinceLastLog);
                TransformComponent* viewerObjectTransform = scene->getComponent<TransformComponent>(scene->getEntityByName("camera_1"));
                camera.setViewYXZ(viewerObjectTransform->translation, viewerObjectTransform->rotation);

                // render
                teRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo);
                teRenderer.endSwapChainRenderPass(commandBuffer);
                teRenderer.endFrame();
            }

            vkDeviceWaitIdle(teDevice.device());
        }
        printf("press enter to exit\n");
    }

    void TheEngine::registerComponents() {
		// scene.registerComponent<TransformComponent>();
		// scene.registerComponent<ModelComponent>();
	}

    void TheEngine::loadGameObjects() {
        std::shared_ptr<TeModel> floorModel = TeModel::createModelFromFile(teDevice, "models\\quad.obj");
        //std::shared_ptr<TePhysics::Plane> floorPhysPlane = std::make_shared<TePhysics::Plane>(glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 0.f), 10.f, 10.f);
        auto floor = scene->createEntity("floor_1");
        scene->addComponent<ModelComponent>(floor, { floorModel, });
        scene->addComponent<TransformComponent>(floor, { glm::vec3(0.f, 0.f, 0.f), glm::vec3(10.f, 10.f, 10.f), { 0.f, 0.f, 0.f } });
        //scene->addComponent<PhysicsPlaneComponent>(floor, { floorPhysPlane, });

        auto viewerObject = scene->createEntity("camera_1");
        scene->addComponent<TransformComponent>(viewerObject, TransformComponent());

        //std::shared_ptr<TeModel> cubeModel = TeModel::createModelFromFile(teDevice, "models\\cube.obj");
        //auto cube = scene->createEntity("cube_1");
        //scene->addComponent<ModelComponent>(cube, { cubeModel, });
        //scene->addComponent<TransformComponent>(cube, { glm::vec3(2.f, 2.f, 2.f), glm::vec3(1.f, 1.f, 1000.f), { 0.f, 0.f, 0.f } });

        // scene.saveEntityToFile(cube, "entities\\cube.ent");
    }

    void TheEngine::registerCommmands() {
        // spawn command
        std::function<const char* (std::vector<std::string>, TheEngine&)> spawnFunction = &TeCommandThread::command_spawn;
        commandThread.registerCommand(spawnFunction, "spawn");

        // log command
        std::function<const char* (std::vector<std::string>, TheEngine&)> logFunction = &TeCommandThread::command_log;
        commandThread.registerCommand(logFunction, "log");
    }
}