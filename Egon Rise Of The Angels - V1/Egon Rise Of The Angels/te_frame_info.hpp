#pragma once

#include "te_camera.hpp"
#include "te_game_object.hpp"
#include <unordered_map>

// lib
#include <vulkan/vulkan.h>

namespace te {
	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
	};

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		TeCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		te::TeScene* scene;
	};
}  // namespace te