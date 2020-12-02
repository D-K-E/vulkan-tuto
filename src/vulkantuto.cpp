// main file
#include <external.hpp>
#include <utils.hpp>
//
namespace vtuto {

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

/**
  standard validation functions are defined in
  VK_LAYER_KHRONOS_validation by
  the lunar sdk
 */
std::vector<const char *> requested_validation_layers = {
    "VK_LAYER_KHRONOS_validation"};

std::vector<const char *> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

/**
  Enable validation layers for debug build. NDEBUG macro is
  a part of c++ standard.
 */

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

/**
  Create debug messenger.

 */
VkResult CreateDebugUtilsMessengerExt(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  //
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
      vkGetInstanceProcAddr(
          instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator,
                pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {
  //
  auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)
      vkGetInstanceProcAddr(
          instance, "vkDestroyDebugUtilsMessengerEXT");
  if (fn != nullptr) {
    fn(instance, debugMessenger, pAllocator);
  }
}

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

/**
  @brief Hello Triangle application object.

  The application contains most of the steps for dealing
  with
  vulkan applications.
 */
class HelloTriangle {
public:
  std::string win_title = "Vulkan Window";
  uint32_t win_width = WIDTH;
  uint32_t win_height = HEIGHT;

  /** instance of the vulkan application */
  VkInstance instance;

  /** debug callback function handler */
  VkDebugUtilsMessengerEXT debugMessenger;

  /** window surface object*/
  VkSurfaceKHR surface;

  /** physical device handler*/
  VkPhysicalDevice physical_dev = VK_NULL_HANDLE;

  /** logical device handler */
  VkDevice l_device;

  /** graphics queue */
  VkQueue graphics_queue;

  /** window surface queue*/
  VkQueue present_queue;

  /** swapchain for handling frame rate*/
  VkSwapchainKHR swap_chain;

  /** images in swap chain */
  std::vector<VkImage> swapchain_images;

  /** swapchain image format*/
  VkFormat swapchain_image_format;

  /** swapchain extent*/
  VkExtent2D swapchain_extent;

public:
  HelloTriangle() {}
  HelloTriangle(std::string wTitle, const uint32_t &w,
                const uint32_t &h)
      : win_title(wTitle), win_width(w), win_height(h) {}
  /**
    Run application.

    Steps to run the application
   */
  void run() {
    // 1. launch window
    initWindow();

    // 2. launch vulkan
    initVulkan();

    // 3. main loop
    renderLoop();

    // 4. clean up ressources
    cleanUp();
  }

private:
  GLFWwindow *window; // window for visualizing object.

  /**
    Initialize window.

    Gives window hints. Sets its size, its title, etc.
   */
  void initWindow() {
    if (glfwInit() == 0) {
      throw std::runtime_error(
          "Unable to launch glfw window");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    //
    window = glfwCreateWindow(win_width, win_height,
                              win_title.c_str(), nullptr,
                              nullptr);
  }

  /**
    Initialize vulkan.

    Steps to initialize a vulkan api
    1. Create a vulkan instance
   */
  void initVulkan() {
    //
    // 1. Create a vulkan instance
    createInstance();

    // 2. Setup debug messenger
    setupDebugMessenger();

    // 3. Create surface
    createSurface();

    // 3. Pick physical device
    pickPhysicalDevice();

    // 4. Create logical device
    createLogicalDevice();

    // 5. create swap chain
    createSwapChain();
  }

  /**
    Create a Vulkan Instance

    Creation has two steps:

    - Optional step: Application info

    - Required step: Instance info

    Application info contains regular
    information with respect to vulkan application.
    For example, name, version, whether it uses a
    specific engine etc. The information mentioned
    here can be used by the driver to optimize certain
    aspects of the runtime with respect to parameters
    entered here.

    The second info, that of instance, is gives information
    with respect to extensions, and validation layers
    we would like to use for the application instance.
   */
  void createInstance() {
    //
    // 1. Create Application info struct
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "My Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // 2. Pass info struct to instance info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType =
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // 3. Request extensions from glfw in order to visualize
    // vulkan
    // application instance
    auto extensions = getRequiredExtensions();

    //
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // 4. create debug messenger handler
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(
          requested_validation_layers.size());
      createInfo.ppEnabledLayerNames =
          requested_validation_layers.data();

      populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT
                              *)&debugCreateInfo;
    } else {
      //
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
    }

    // 5. create the instance with the given information
    CHECK_VK(
        vkCreateInstance(&createInfo, nullptr, &instance),
        "Failed to create Vulkan instance");
  }

  /**
    Rendering loop.

    Render elements to window. Acquire user input
   */
  void renderLoop() {
    //
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }
  }

  /**
    Clean up ressources.

    Destroy window, and other ressources.
   */
  void cleanUp() {
    // 1. destroy swap chain
    vkDestroySwapchainKHR(l_device, swap_chain, nullptr);

    // 2. destroy logical device
    vkDestroyDevice(l_device, nullptr);
    // 1. destroy debugging utils
    if (enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(
          instance, debugMessenger, nullptr);
    }
    // 2. destroy surface
    vkDestroySurfaceKHR(instance, surface, nullptr);

    // 3. destroy instance always last in
    // a vulkan application.
    vkDestroyInstance(instance, nullptr);

    // 4. destroy window
    glfwDestroyWindow(window);

    // 5. glfw terminate
    glfwTerminate();
  }

  /**
    Check if requested layers are available

    Validation layers come with sdk they are not supported
    by Vulkan by
    default. We check if the requested layers are found in
    the system. In
    order to do that we first check the instance's layer
    properties with \c
    vkEnumerateInstanceLayerProperties() \c function.
    Requested layers are
    in requested_validation_layers vector. We compare the
    name of the
    available layers in the instance and members of
    requested_validation_layers.
   */
  bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount,
                                       nullptr);

    std::vector<VkLayerProperties> availableLayers(
        layerCount);

    vkEnumerateInstanceLayerProperties(
        &layerCount, availableLayers.data());

    for (const char *layerName :
         requested_validation_layers) {
      //
      bool requestedLayerIsFound = false;
      for (const auto &layerProperties : availableLayers) {
        //
        std::string layer_name = layerName;
        std::string available_layer_name =
            layerProperties.layerName;
        if (layer_name == available_layer_name) {
          requestedLayerIsFound = true;
          break;
        }
      }
      if (!requestedLayerIsFound) {
        return false;
      }
    }
    //
    return true;
  }

  /**
    Specify properties of the debug messenger callback

    We add its type, requested message severities, message
    types, and we add which debug callback function is going
    to be used
   */
  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};
    createInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    //
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    //
    createInfo.pfnUserCallback = debugCallback;
  }
  void createSurface() {
    CHECK_VK(glfwCreateWindowSurface(instance, window,
                                     nullptr, &surface),
             "failed to create window surface");
  }

  /**
    Print validation layer output

    A static function that prints the output of the
    validation layer. The signature of the function fits to
    the @see populateDebugMessengerCreateInfo()
    content.
   */
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT
                    messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT
                    *pCallbackData,
                void *pUserData) {
    std::cerr << "validation layer output: "
              << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
  }

  /**
    Set @see debugMessenger up by populating its related
    info.
   */
  void setupDebugMessenger() {
    if (!enableValidationLayers)
      return;
    //
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    CHECK_VK(CreateDebugUtilsMessengerExt(
                 instance, &createInfo, nullptr,
                 &debugMessenger),
             "failed to create and setup debug messenger");
  }

  //
  std::vector<const char *> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(
        &glfwExtensionCount);

    // we reserve the size for the requested extensions
    // vector
    std::vector<const char *> extensions(
        glfwExtensions,
        glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers) {
      extensions.push_back(
          VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
  }

  /**
Find device family indices for given VkPhysicalDevice

We query the given physical device for physical device
family properties. We break away if the device has
complete
number of indices for given device family.
*/
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

  void pickPhysicalDevice() {
    //
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
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availables) {
    //
    for (const auto &aformat : availables) {
      if (aformat.format == VK_FORMAT_B8G8R8A8_SRGB &&
          aformat.colorSpace ==
              VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        // ACES - XYZ
        return aformat;
      }
    }
    return availables[0];
  }
  /**
   Choose right present mode for swapchain

   Available present modes: VK_PRESENT_MODE_*_KHR
   - IMMEDIATE :  direct rendering of image to screen
   - FIFO: Render from the front of queue and put it back of
   the queue
   - FIFO_RELAXED: Variation on FIFO. Instead of waiting for
   an empty queue,
   we render the image right away.
   - MAILBOX: variation of FIFO. When the queue is full
   replace the images
   with newer ones.
   */
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availables) {
    //
    for (const auto &apresent : availables) {
      if (apresent == VK_PRESENT_MODE_MAILBOX_KHR) {
        return apresent;
      }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D chooseSwapExtent(
      const VkSurfaceCapabilitiesKHR &capabilities) {
    //
    if (capabilities.currentExtent.width != UINT32_MAX) {
      return capabilities.currentExtent;
    } else {
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      VkExtent2D actual_extent = {
          static_cast<uint32_t>(width),
          static_cast<uint32_t>(height)};
      actual_extent.width = std::max(
          capabilities.minImageExtent.width,
          std::min(capabilities.minImageExtent.width,
                   actual_extent.width));
      actual_extent.height = std::max(
          capabilities.minImageExtent.height,
          std::min(capabilities.minImageExtent.height,
                   actual_extent.height));
      return actual_extent;
    }
  }

  void createSwapChain() {
    SwapChainSupportDetails swap_details =
        querySwapChainSupport(physical_dev);

    VkSurfaceFormatKHR surfaceFormat =
        chooseSwapSurfaceFormat(swap_details.formats);

    VkPresentModeKHR presentMode =
        chooseSwapPresentMode(swap_details.present_modes);

    VkExtent2D extent =
        chooseSwapExtent(swap_details.capabilities);

    uint32_t img_count =
        swap_details.capabilities.minImageCount + 1;
    if (swap_details.capabilities.maxImageCount > 0 &&
        img_count >
            swap_details.capabilities.maxImageCount) {
      img_count = swap_details.capabilities.maxImageCount;
    }

    // Swapchain info details
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType =
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    // surface
    createInfo.surface = surface;

    // image type, size etc
    createInfo.minImageCount = img_count;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueuFamilyIndices indices =
        find_family_indices(physical_dev);
    uint32_t qfamily_indices[] = {
        indices.graphics_family.value(),
        indices.present_family.value()};

    if (indices.graphics_family != indices.present_family) {
      createInfo.imageSharingMode =
          VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = qfamily_indices;
    } else {
      createInfo.imageSharingMode =
          VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform =
        swap_details.capabilities.currentTransform;
    createInfo.compositeAlpha =
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    // handling of used ressources
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    //
    CHECK_VK(vkCreateSwapchainKHR(l_device, &createInfo,
                                  nullptr, &swap_chain),
             "failed to create a swap chain");

    CHECK_VK(vkGetSwapchainImagesKHR(l_device, swap_chain,
                                     &img_count, nullptr),
             "failed to reserve for swapchain images");
    swapchain_images.resize(img_count);
    CHECK_VK(vkGetSwapchainImagesKHR(
                 l_device, swap_chain, &img_count,
                 swapchain_images.data()),
             "failed to set swapchain images");
    swapchain_image_format = surfaceFormat.format;
    swapchain_extent = extent;
  }

  /**
    Query supported swap chain details.

    \c vkGetPhysicalDeviceSurfaceFormatsKHR() \c for
    querying supported
    color formats (unorm, srgb, etc)

    \c vkGetPhysicalDeviceSurfacePresentModesKHR \c for
    querying supported
    data structures for presentation to be used in swap
    chain management.
   */
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
  /**
    Check if the device is suitable for implementing a swap
    chain
   */
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

  /**
    Check if device support requested extensions.

    As usual we query the device for available extensions
    then check whether what we had requested is available
    by erasing from requested extensions.
   */
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

  void createLogicalDevice() {
    //
    QueuFamilyIndices indices =
        find_family_indices(physical_dev);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphics_family.value(),
        indices.present_family.value()};

    float queuePriority = 1.0f;
    for (uint32_t qfamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType =
          VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = qfamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    //
    VkPhysicalDeviceFeatures deviceFeature{};

    //
    VkDeviceCreateInfo createInfo{};
    createInfo.sType =
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeature;
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(device_extensions.size());
    createInfo.ppEnabledExtensionNames =
        device_extensions.data();

    //
    if (enableValidationLayers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(
          requested_validation_layers.size());
      createInfo.ppEnabledLayerNames =
          requested_validation_layers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }
    CHECK_VK(vkCreateDevice(physical_dev, &createInfo,
                            nullptr, &l_device),
             "failed to create a logical device given "
             "create info params");

    //
    vkGetDeviceQueue(l_device,
                     indices.graphics_family.value(), 0,
                     &graphics_queue);
    vkGetDeviceQueue(l_device,
                     indices.present_family.value(), 0,
                     &present_queue);
  }
};

extern "C" int main() {
  std::string wtitle = "Vulkan Window Title";
  HelloTriangle hello(wtitle, (uint32_t)640, (uint32_t)480);

  try {
    hello.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
}
