#pragma once

#include <string>
#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "te_frame_info.hpp"
#include "te_camera.hpp"
#include "te_device.hpp"
#include "te_game_object.hpp"
#include "re_pipeline.hpp"
#include "te_model.hpp"
#include "te_descriptors.hpp"

namespace te {
	class SimpleRenderSystem {
	public:
		void renderGameObjects(te::FrameInfo& frameInfo);

		SimpleRenderSystem(TeDevice& device, VkRenderPass renderPass, std::unique_ptr<TeDescriptorSetLayout>& globalSetLayout, std::unique_ptr<TeDescriptorPool>& globalPool);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		void operator=(const SimpleRenderSystem&) = delete;
	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		TeDevice& teDevice;

		std::unique_ptr<TePipeline> tePipeline{};
		VkPipelineLayout pipelineLayout;

		std::unique_ptr<te::TeDescriptorPool>& globalPool_;
	};
}