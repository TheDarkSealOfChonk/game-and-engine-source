#pragma once

#include "te_device.hpp"
#include "te_buffer.hpp"
// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace te {

    class TeDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(TeDevice& teDevice) : teDevice{ teDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<TeDescriptorSetLayout> build() const;

        private:
            TeDevice& teDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        TeDescriptorSetLayout(
            TeDevice& teDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~TeDescriptorSetLayout();
        TeDescriptorSetLayout(const TeDescriptorSetLayout&) = delete;
        TeDescriptorSetLayout& operator=(const TeDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        TeDevice& teDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class TeDescriptorWriter;
    };

    class TeDescriptorPool {
    public:
        class Builder {
        public:
            Builder(TeDevice& teDevice) : teDevice{ teDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<TeDescriptorPool> build() const;

        private:
            TeDevice& teDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        TeDescriptorPool(
            TeDevice& teDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~TeDescriptorPool();
        TeDescriptorPool(const TeDescriptorPool&) = delete;
        TeDescriptorPool& operator=(const TeDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

        VkDescriptorPool internalPool() { return descriptorPool; }

    private:
        TeDevice& teDevice;
        VkDescriptorPool descriptorPool;

        friend class TeDescriptorWriter;
    };

    class TeDescriptorWriter {
    public:
        TeDescriptorWriter(TeDescriptorSetLayout& setLayout, TeDescriptorPool& pool);

        TeDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        TeDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        TeDescriptorSetLayout& setLayout;
        TeDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace te