#pragma once
#include <vector>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "te_device.hpp"
#include "te_model.hpp"
#include "te_buffer.hpp"
#include <memory>

namespace te {
	class TeModel {
	public:
		struct Vertex {
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};
			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
			bool operator==(const Vertex& other) const {
				return position == other.position && color == other.color && normal == other.normal &&
					uv == other.uv;
			}
		};

		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
			void loadModel(const std::string& filepath);
		};

		TeModel(TeDevice& device, const TeModel::Builder& builder);
		~TeModel();
		TeModel(const TeModel&) = delete;
		TeModel& operator=(const TeModel&) = delete;
		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);



		static std::unique_ptr<TeModel> createModelFromFile(TeDevice& device, const std::string& filepath);
	private:
		TeDevice& teDevice;
		
		std::unique_ptr<TeBuffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer = false;
		std::unique_ptr<TeBuffer> indexBuffer;
		uint32_t indexCount;
	};
}