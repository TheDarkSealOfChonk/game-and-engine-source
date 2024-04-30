#include "simple_render_system.hpp"
#include <stdexcept>
#include <memory>
#include <array>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "re_window.hpp"
#include "te_device.hpp"
#include "te_swap_chain.hpp"
#include "te_descriptors.hpp"
#include "re_pipeline.hpp"

namespace te {
	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.0f };
		glm::mat4 normalMatrix{ 1.0f };
	};

	SimpleRenderSystem::SimpleRenderSystem(TeDevice& device, VkRenderPass renderPass, std::unique_ptr<TeDescriptorSetLayout>& globalSetLayout, std::unique_ptr<TeDescriptorPool>& globalPool) : teDevice{ device }, globalPool_{ globalPool } {
		createPipelineLayout(globalSetLayout->getDescriptorSetLayout());
		createPipeline(renderPass);
	};

	SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(teDevice.device(), pipelineLayout, nullptr); }

	void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(teDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error{ "failed to create pipeline layout!" };
		}
	}
	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		TePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		tePipeline = std::make_unique<TePipeline>(
			teDevice,
			"shaders\\simple_shader.vert.spv",
			"shaders\\simple_shader.frag.spv",
			pipelineConfig);
	}

	void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo) {
		tePipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr);

		TeScene* scene = frameInfo.scene;
		for (auto& objModelComponentAndContainer : scene->getComponentInstances<ModelComponent>()) {
			TeScene::Entity obj = objModelComponentAndContainer.first;
			ModelComponent* objModelComponent = objModelComponentAndContainer.second;
			std::shared_ptr<TeModel> objModel = objModelComponent->model;
			TransformComponent* objTransformComponent = scene->getComponent<TransformComponent>(obj);
			if (objTransformComponent == nullptr) continue;

			SimplePushConstantData push{};
			push.modelMatrix = objTransformComponent->mat4();
			push.normalMatrix = objTransformComponent->normalMatrix();

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);

			objModel->bind(frameInfo.commandBuffer);
			objModel->draw(frameInfo.commandBuffer);
		}
	}
}