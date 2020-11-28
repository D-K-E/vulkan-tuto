// main file
#include <GLFW/glfw3.h>
#include <cstdint>
#include <external.hpp>
//
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

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
  VkInstance instance;

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
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(
        &glfwExtensionCount);

    //
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    //
    createInfo.enabledLayerCount = 0;

    // 4. create the instance with the given information
    if (vkCreateInstance(&createInfo, nullptr, &instance) !=
        VK_SUCCESS) {
      throw std::runtime_error(
          "Failed to create Vulkan instance");
    }
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
    //
    // 1. destroy instance
    vkDestroyInstance(instance, nullptr);

    // 2. destroy window
    glfwDestroyWindow(window);

    // 3. glfw terminate
    glfwTerminate();
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
