// main file
#include <cstdint>
#include <external.hpp>
#include <utils.hpp>
//
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

/**
  standard validation functions are defined in
  VK_LAYER_KHRONOS_validation by
  the lunar sdk
 */
std::vector<const char *> requested_validation_layers = {
    "VK_LAYER_KHRONOS_validation"};

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
  bool is_complete() { return graphics_family.has_value(); }
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

  /** physical device handler*/
  VkPhysicalDevice physicalDev = VK_NULL_HANDLE;

  /** logical device handler */
  VkDevice l_device;

  /** graphics queue */
  VkQueue graphics_queue;

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

    // 3. Pick physical device
    pickPhysicalDevice();

    // 4. Create logical device
    createLogicalDevice();
  }

  /**
    Create a Vulkan Instance

    Create a vulkan instance with application
    info. Application info contains regular
    information with respect to vulkan application.
    For example, name, version,
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
    // 1. destroy debugging utils
    if (enableValidationLayers) {
      DestroyDebugUtilsMessengerEXT(
          instance, debugMessenger, nullptr);
    }
    // 2. destroy instance
    vkDestroyInstance(instance, nullptr);

    // 3. destroy window
    glfwDestroyWindow(window);

    // 4. glfw terminate
    glfwTerminate();
  }

  /**
    Check if requested layers are available

    Validation layers come with sdk they are not supported
    by Vulkan by default. We check if the requested layers
    are found in the system. In order to do that we first
    check the instance's layer properties. Requested layers
    are in requested_validation_layers vector. We compare
    the name of the available layers in the instance and
    members of requested_validation_layers.
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
    Check if given device is complete

    We find the QueuFamilyIndices for the device and
    check if it is complete
   */
  bool is_device_ok(VkPhysicalDevice pdev) {
    //
    QueuFamilyIndices indices = find_family_indices(pdev);
    return indices.is_complete();
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
      if (is_device_ok(device)) {
        //
        physicalDev = device;
        break;
      }
    }
    if (physicalDev == VK_NULL_HANDLE) {
      //
      throw std::runtime_error("Your device does not "
                               "respond to any of "
                               "available queueFamilies");
    }
  }

  void createLogicalDevice() {
    //
    QueuFamilyIndices indices =
        find_family_indices(physicalDev);

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType =
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex =
        indices.graphics_family.value();
    queueCreateInfo.queueCount = 1;

    //
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    //
    VkPhysicalDeviceFeatures deviceFeature{};

    //
    VkDeviceCreateInfo createInfo{};
    createInfo.sType =
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;

    createInfo.pEnabledFeatures = &deviceFeature;
    createInfo.enabledExtensionCount = 0;

    //
    if (enableValidationLayers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(
          requested_validation_layers.size());
      createInfo.ppEnabledLayerNames =
          requested_validation_layers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }
    CHECK_VK(vkCreateDevice(physicalDev, &createInfo,
                            nullptr, &l_device),
             "failed to create a logical device given "
             "create info params");

    //
    vkGetDeviceQueue(l_device,
                     indices.graphics_family.value(), 0,
                     &graphics_queue);
  }
};

int main() {
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
