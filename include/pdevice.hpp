#pragma once
/**\brief Physical Device Object*/

#include <debug.hpp>
#include <external.hpp>
#include <support.hpp>

using namespace vtuto;

namespace vtuto {

class physical_device {
public:
  VkPhysicalDevice pdevice = VK_NULL_HANDLE;
  VkSurfaceKHR surface;
  VkInstance *instance_ptr;

public:
  physical_device(VkInstance *ins, GLFWwindow *window)
      : instance_ptr(ins) {
    // 1. create surface
    createSurface(window);

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance(), &device_count,
                               nullptr);
    //
    if (device_count == 0) {
      throw std::runtime_error(
          "Vulkan api is not supported by your hardware");
    }
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance(), &device_count,
                               devices.data());

    for (const auto &device : devices) {
      //
      if (is_device_suitable(device)) {
        //
        pdevice = device;
        break;
      }
    }
    if (pdevice == VK_NULL_HANDLE) {
      //
      throw std::runtime_error("Your device does not "
                               "respond to any of "
                               "available queueFamilies");
    }
  }
  ~physical_device() {
    vkDestroySurfaceKHR(instance(), surface, nullptr);
  }
  bool is_device_suitable(VkPhysicalDevice pdev) {
    QueuFamilyIndices indices =
        QueuFamilyIndices::find_family_indices(pdev,
                                               surface);
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
  void createSurface(GLFWwindow *window) {
    CHECK_VK(glfwCreateWindowSurface(instance(), window,
                                     nullptr, &surface),
             "failed to create window surface");
  }
  VkInstance instance() { return *instance_ptr; }
  bool checkDeviceExtensionSupport(VkPhysicalDevice pdev) {
    //
    uint32_t ext_count = 0;
    vkEnumerateDeviceExtensionProperties(
        pdev, nullptr, &ext_count, nullptr);

    std::vector<VkExtensionProperties> available_exts(
        ext_count);

    vkEnumerateDeviceExtensionProperties(
        pdev, nullptr, &ext_count, available_exts.data());

    std::set<std::string> requested_extensions(
        device_extensions.begin(), device_extensions.end());

    for (const auto &ext : available_exts) {
      requested_extensions.erase(ext.extensionName);
    }
    return requested_extensions.empty();
  }
  SwapChainSupportDetails
  querySwapChainSupport(VkPhysicalDevice pdev) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        pdev, surface, &details.capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        pdev, surface, &format_count, nullptr);

    if (format_count != 0) {
      details.formats.resize(format_count);
      vkGetPhysicalDeviceSurfaceFormatsKHR(
          pdev, surface, &format_count,
          details.formats.data());
    }
    uint32_t present_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        pdev, surface, &present_count, nullptr);

    if (present_count != 0) {
      details.present_modes.resize(present_count);
      vkGetPhysicalDeviceSurfacePresentModesKHR(
          pdev, surface, &present_count,
          details.present_modes.data());
    }
    return details;
  }
};
}
