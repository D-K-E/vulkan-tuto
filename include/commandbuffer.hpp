// command buffer for vulkan application
#pragma once
//
#include <external.hpp>
#include <framebuffer.hpp>
#include <ldevice.hpp>
#include <pdevice.hpp>
#include <support.hpp>
#include <vbuffer.hpp>

using namespace vtuto;
namespace vtuto {
struct VkDrawInfo {
  uint32_t vertex_count;
  uint32_t instance_count;
  uint32_t first_vertex_index;
  uint32_t first_instance_index;
};
class vk_command_pool {
public:
  VkCommandPool pool;

public:
  vk_command_pool() {}
  vk_command_pool(
      const vulkan_device<VkPhysicalDevice> &physical_dev,
      vulkan_device<VkDevice> &logical_dev) {
    QueuFamilyIndices qfi =
        QueuFamilyIndices::find_family_indices(
            physical_dev.pdevice, physical_dev.surface);
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex =
        qfi.graphics_family.value();
    CHECK_VK(vkCreateCommandPool(logical_dev.device(),
                                 &commandPoolInfo, nullptr,
                                 &pool),
             "failed to create command pool");
  }
  void destroy(vulkan_device<VkDevice> &logical_dev) {
    vkDestroyCommandPool(logical_dev.device(), pool,
                         nullptr);
  }
};
template <> class vulkan_buffer<VkCommandBuffer> {
  //
public:
  VkCommandBuffer buffer;

public:
  vulkan_buffer() {}
  vulkan_buffer(
      VkCommandBuffer loc,
      vulkan_buffer<VkFramebuffer> &sc_framebuffer,
      VkRenderPass &render_pass,
      VkExtent2D swap_chain_extent,
      VkPipeline graphics_pipeline,
      int32_t render_offset_x = 0,
      int32_t render_offset_y = 0,
      VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f},
      uint clearValueCount = 1,
      VkSubpassContents subpass_contents =
          VK_SUBPASS_CONTENTS_INLINE,
      VkPipelineBindPoint graphics_pass_bind_point =
          VK_PIPELINE_BIND_POINT_GRAPHICS,
      uint32_t vertex_count = 3,
      uint32_t instance_count = 1,
      uint32_t first_vertex_index = 0,
      uint32_t first_instance_index = 0)
      : buffer(loc) {
    mk_cmd_buffer(
        sc_framebuffer, render_pass, swap_chain_extent,
        graphics_pipeline, render_offset_x, render_offset_y,
        clearColor, clearValueCount, subpass_contents,
        graphics_pass_bind_point, vertex_count,
        instance_count, first_vertex_index,
        first_instance_index);
  }
  vulkan_buffer(
      VkCommandBuffer loc,
      vulkan_buffer<VkFramebuffer> &sc_framebuffer,
      VkRenderPass &render_pass,
      VkExtent2D swap_chain_extent,
      VkPipeline graphics_pipeline,
      VkCommandBufferBeginInfo beginInfo,
      VkRenderPassBeginInfo renderPassInfo,
      VkDrawInfo drawInfo,
      VkSubpassContents subpass_contents =
          VK_SUBPASS_CONTENTS_INLINE,
      VkPipelineBindPoint graphics_pass_bind_point =
          VK_PIPELINE_BIND_POINT_GRAPHICS)
      : buffer(loc) {
    //
    mk_cmd_buffer(sc_framebuffer, render_pass,
                  swap_chain_extent, graphics_pipeline,
                  beginInfo, renderPassInfo, drawInfo,
                  subpass_contents,
                  graphics_pass_bind_point);
  }
  void mk_cmd_buffer(
      vulkan_buffer<VkFramebuffer> &sc_framebuffer,
      VkRenderPass &render_pass,
      VkExtent2D swap_chain_extent,
      VkPipeline graphics_pipeline,
      int32_t render_offset_x = 0,
      int32_t render_offset_y = 0,
      VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f},
      uint clearValueCount = 1,
      VkSubpassContents subpass_contents =
          VK_SUBPASS_CONTENTS_INLINE,
      VkPipelineBindPoint graphics_pass_bind_point =
          VK_PIPELINE_BIND_POINT_GRAPHICS,
      uint32_t vertex_count = 3,
      uint32_t instance_count = 1,
      uint32_t first_vertex_index = 0,
      uint32_t first_instance_index = 0) {

    // 1. create command buffer info
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    CHECK_VK(vkBeginCommandBuffer(buffer, &beginInfo),
             "failed to begin recording commands");

    // 2. create render pass info
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType =
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass;
    renderPassInfo.framebuffer = sc_framebuffer.buffer;
    renderPassInfo.renderArea.offset = {render_offset_x,
                                        render_offset_y};
    renderPassInfo.renderArea.extent = swap_chain_extent;
    renderPassInfo.clearValueCount = clearValueCount;
    renderPassInfo.pClearValues = &clearColor;

    // 3. give command to start rendering pass
    vkCmdBeginRenderPass(buffer, &renderPassInfo,
                         subpass_contents);

    // 4. bind pipeline to command buffer
    vkCmdBindPipeline(buffer, graphics_pass_bind_point,
                      graphics_pipeline);

    // 5. draw given command buffer
    vkCmdDraw(buffer, vertex_count, instance_count,
              first_vertex_index, first_instance_index);
    vkCmdEndRenderPass(buffer);
    CHECK_VK(vkEndCommandBuffer(buffer),
             "failed to register command buffer");
  }
  void mk_cmd_buffer(
      vulkan_buffer<VkFramebuffer> &sc_framebuffer,
      VkRenderPass &render_pass,
      VkExtent2D swap_chain_extent,
      VkPipeline graphics_pipeline,
      VkCommandBufferBeginInfo beginInfo,
      VkRenderPassBeginInfo renderPassInfo,
      VkDrawInfo drawInfo,
      VkSubpassContents subpass_contents =
          VK_SUBPASS_CONTENTS_INLINE,
      VkPipelineBindPoint graphics_pass_bind_point =
          VK_PIPELINE_BIND_POINT_GRAPHICS) {
    //
    // 1. create command buffer info
    CHECK_VK(vkBeginCommandBuffer(buffer, &beginInfo),
             "failed to begin recording commands");

    // 2.
    vkCmdBeginRenderPass(buffer, &renderPassInfo,
                         subpass_contents);

    // 3.
    vkCmdBindPipeline(buffer, graphics_pass_bind_point,
                      graphics_pipeline);

    // 4.
    vkCmdDraw(buffer, drawInfo.vertex_count,
              drawInfo.instance_count,
              drawInfo.first_vertex_index,
              drawInfo.first_instance_index);

    // 5.
    vkCmdEndRenderPass(buffer);
    CHECK_VK(vkEndCommandBuffer(buffer),
             "failed to register command buffer");
  }
};
}
