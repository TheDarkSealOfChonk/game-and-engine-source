#include "te_renderer.hpp"
#include <stdexcept>
#include <memory>
#include <array>

#include "re_window.hpp"
#include "te_device.hpp"
#include "te_swap_chain.hpp"
#include "re_pipeline.hpp"
#include "te_model.hpp"

namespace te {

	TeRenderer::TeRenderer(TeWindow& window, TeDevice& device) : teWindow{ window }, teDevice{ device } {
		recreateSwapChain();
		createCommandBuffers();
	}
	TeRenderer::~TeRenderer() { freeCommandBuffers(); }

	void TeRenderer::createCommandBuffers() {
		commandBuffers.resize(TeSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = teDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(teDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error{ "failed to allocate command buffers!" };
		}
	}

	void TeRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(teDevice.device(), teDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	void TeRenderer::recreateSwapChain() {
		auto extent = teWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = teWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(teDevice.device());
		if (teSwapChain == nullptr) {
			teSwapChain = std::make_unique<TeSwapChain>(teDevice, extent);
		}
		else {
			std::shared_ptr<TeSwapChain> oldSwapChain = std::move(teSwapChain);
			teSwapChain = std::make_unique<TeSwapChain>(teDevice, extent, oldSwapChain);
			if (!(*oldSwapChain).compareSwapFormats(*teSwapChain.get())) {
				throw std::runtime_error("swap chains not compatable!");
			}
		}
		// createPipeline();
	}

	VkCommandBuffer TeRenderer::beginFrame() {
		assert(!isFrameStarted && "frame allready started!");

		auto result = teSwapChain->acquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = true;
		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error{ "failed to begin command buffers? i dont even know what that means god damit!" };
		}
		return commandBuffer;
	}

	void TeRenderer::endFrame() {
		assert(isFrameStarted && "cannot end a frame that was not started! this could only be because of bad code so f you myself!");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error{ "failed to record command buffer!" };
		}
		auto result = teSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
			teWindow.wasWindowResized()) {
			teWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % TeSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void TeRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "cannot begin swap chain render pass on a frame that was not started! this could only be because of bad code so f you myself!");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = teSwapChain->getRenderPass();
		renderPassInfo.framebuffer = teSwapChain->getFrameBuffer(currentImageIndex);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = teSwapChain->getSwapChainExtent();
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues.data();
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(teSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(teSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, teSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void TeRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "cannot end swap chain render pass on a frame that was not started! this could only be because of bad code so f you myself!");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");

		vkCmdEndRenderPass(commandBuffer);
	}
}