// image view in vulkan app
#pragma once
#include <debug.hpp>
#include <external.hpp>
#include <ldevice.hpp>
#include <pdevice.hpp>
#include <support.hpp>
#include <swapchain.hpp>
#include <utils.hpp>

using namespace vtuto;

namespace vtuto {
//
class image_view {
public:
  std::vector<VkImageView> views;

public:
  image_view() {}
  image_view(
      const std::vector<VkImage> &images,
      VkFormat image_format,
      vulkan_device<VkDevice> &logical_dev,
      VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D,
      unsigned int base_mipmap_index = 0,
      unsigned int mipmap_count = 1,
      unsigned int array_layer_index = 0,
      unsigned int array_layer_count = 1) {
    views.resize(images.size());
    for (std::size_t i = 0; i < images.size(); i++) {
      //
      VkImageViewCreateInfo createInfo{};
      createInfo.sType =
          VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = images[i];
      /** view type can be 1d texture, 2d texture, 3d
       * textures and cubemaps*/
      createInfo.viewType = view_type;
      createInfo.format = image_format;

      /** vec.xx == vec2(vec.x, vec.x) */
      createInfo.components.r =
          VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g =
          VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b =
          VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a =
          VK_COMPONENT_SWIZZLE_IDENTITY;

      createInfo.subresourceRange.aspectMask =
          VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel =
          base_mipmap_index;
      createInfo.subresourceRange.levelCount = mipmap_count;
      createInfo.subresourceRange.baseArrayLayer =
          array_layer_index;
      createInfo.subresourceRange.layerCount =
          array_layer_count;
      CHECK_VK(vkCreateImageView(logical_dev.device(),
                                 &createInfo, nullptr,
                                 &views[i]),
               "failed to create image view");
    }
  }
  std::size_t size() { return views.size(); }
  VkImageView operator[](std::size_t i) { return views[i]; }
};
}
