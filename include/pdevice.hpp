#pragma once
/**\brief Physical Device Object*/

#include <debug.hpp>
#include <external.hpp>

using namespace vtuto;

namespace vtuto {

class physical_device {
public:
  VkPhysicalDevice pdevice = VK_NULL_HANDLE;

public:
  physical_device(const VkInstance &instance) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count,
                               nullptr);
    //
    if (device_count == 0) {
      throw std::runtime_error(
          "Vulkan api is not supported by your hardware");
    }
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count,
                               devices.data());

    for (const auto &device : devices) {
      //
      if (is_device_suitable(device)) {
        //
        physical_dev = device;
        break;
      }
    }
    if (physical_dev == VK_NULL_HANDLE) {
      //
      throw std::runtime_error("Your device does not "
                               "respond to any of "
                               "available queueFamilies");
    }
  }
  bool is_device_suitable(VkPhysicalDevice pdev) {
    QueuFamilyIndices indices = find_family_indices(pdev);
    bool areExtensionsSupported =
        checkDeviceExtensionSupport(pdev);

    bool isSwapChainPossible = false;
    if (areExtensionsSupported) {
      SwapChainSupportDetails swapChainSupport =
          querySwapChainSupport(pdev);
      isSwapChainPossible =
          !swapChainSupport.formats.empty() &&
          !swapChainSupport.present_modes.empty();
    }
    return indices.is_complete() &&
           areExtensionsSupported && isSwapChainPossible;
  }
  QueuFamilyIndices
  find_family_indices(VkPhysicalDevice pdev) {
    //
    QueuFamilyIndices indices;
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        pdev, &familyCount, nullptr);
    //
    std::vector<VkQueueFamilyProperties> queueFamilies(
        familyCount);

    vkGetPhysicalDeviceQueueFamilyProperties(
        pdev, &familyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const auto &qfamily : queueFamilies) {
      //
      if (qfamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphics_family = i;
      }

      VkBool32 present_support = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(
          pdev, i, surface, &present_support);

      if (present_support) {
        indices.present_family = i;
      }

      if (indices.is_complete()) {
        break;
      }
      i++;
    }
    return indices;
  }
};
}
