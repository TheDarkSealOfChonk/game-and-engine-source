#pragma once

#include <cassert>

#include <string>
#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

#include "re_window.hpp"
#include "te_device.hpp"
#include "te_swap_chain.hpp"

namespace te {
	class TeRenderer {
	public:
		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		bool isFrameInProgress() const { return isFrameStarted; }
		VkRenderPass getSwapChainRenderPass() { return (*teSwapChain).getRenderPass(); }
		float getAspectRatio() const { return (*teSwapChain).extentAspectRatio(); }
		int getFrameIndex() const {
			assert(isFrameStarted && "cannot get frame index when frame not in progress!");
			return currentFrameIndex; 
		}

		VkCommandBuffer getCurrentCommandBuffer() {
			assert(isFrameStarted && "atempted to get frame buffer while frame not is progress");
			return commandBuffers[currentFrameIndex]; 
		}

		TeRenderer(TeWindow& window, TeDevice& device);
		~TeRenderer();

		TeRenderer(const TeRenderer&) = delete;
		void operator=(const TeRenderer&) = delete;
	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		TeWindow& teWindow;
		TeDevice& teDevice;
		std::unique_ptr<TeSwapChain> teSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex;
		bool isFrameStarted;
	};
}