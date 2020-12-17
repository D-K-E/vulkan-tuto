#pragma once
#include <external.hpp>

namespace vtuto {
//
struct QueuFamilyIndices {
  //
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;
  bool is_complete() {
    return graphics_family.has_value() &&
           present_family.has_value();
  }
};
struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};
}
