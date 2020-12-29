// vulkan frame buffer object
#pragma once
#include <external.hpp>
#include <vbuffer.hpp>

using namespace vtuto;

namespace vtuto {
template <> class vulkan_buffer<VkFramebuffer> {
  vulkan_buffer() {}
  vulkan_buffer(int width, int height, int layer_nb,
          const VkRenderPass &render_pass,
          const std::vector<image_view> &views
          ) {}
};
}
