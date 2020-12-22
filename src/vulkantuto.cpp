// main file
#include <debug.hpp>
#include <external.hpp>
#include <hellotriangle.hpp>
#include <ldevice.hpp>
#include <pdevice.hpp>
#include <support.hpp>
#include <utils.hpp>
//
using namespace vtuto;

namespace vtuto {

/**
  @brief Hello Triangle application object.

  The application contains most of the steps for dealing
  with
  vulkan applications.
 */

/**
Run application.

Steps to run the application
*/
void HelloTriangle::run() {
  // 1. launch window
  initWindow();

  // 2. launch vulkan
  initVulkan();

  // 3. main loop
  renderLoop();

  // 4. clean up ressources
  cleanUp();
}

/**
  Initialize window.

  Gives window hints. Sets its size, its title, etc.
 */
void HelloTriangle::initWindow() {
  if (glfwInit() == 0) {
    throw std::runtime_error(
        "Unable to launch glfw window");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  //
  window =
      glfwCreateWindow(win_width, win_height,
                       win_title.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, nullptr);
  glfwSetFramebufferSizeCallback(
      window, framebuffer_resize_callback);
}
/**
Initialize vulkan.

Steps to initialize a vulkan api
1. Create a vulkan instance
*/
void HelloTriangle::initVulkan() {
  //
  // 1. Create a vulkan instance
  createInstance();

  // 2. Setup debug messenger
  setupDebugMessenger();

  // 3. Create physical device
  physical_dev = physical_device(&instance, window);

  // 4. Create logical device
  // createLogicalDevice();
  l_device =
      logical_device(enableValidationLayers, physical_dev);

  // 5. create swap chain
  createSwapChain();

  // 6. create swap chain image views
  createSwapChainImageViews();

  // 7. create render pass
  createRenderPass();

  // 8. create graphics pipeline
  createGraphicsPipeline();

  // 9. create framebuffers
  createFramebuffers();

  // 10. create command pool
  createCommandPool();

  // 11. create command buffer
  createCommandBuffer();

  // 12. create sync objects: semaphores, fences etc
  createSyncObjects();
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
void HelloTriangle::createInstance() {
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
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
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
void HelloTriangle::renderLoop() {
  //
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    draw();
  }
  vkDeviceWaitIdle(l_device.ldevice);
}
/**
  Clean up ressources.

  Destroy window, and other ressources.
 */
void HelloTriangle::cleanUp() {
  //
  cleanupSwapchain();
  for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(l_device.ldevice,
                       render_finished_semaphores[i],
                       nullptr);
    vkDestroySemaphore(l_device.ldevice,
                       image_available_semaphores[i],
                       nullptr);
    vkDestroyFence(l_device.ldevice, current_fences[i],
                   nullptr);
  }
  vkDestroyCommandPool(l_device.ldevice, command_pool,
                       nullptr);
  // 4. destroy logical device
  // vkDestroyDevice(l_device.ldevice, nullptr);
  l_device.destroy();
  // 5. destroy debugging utils
  if (enableValidationLayers) {
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger,
                                  nullptr);
  }
  // 6. destroy surface
  physical_dev.destroy();

  // 7. destroy instance always last in
  // a vulkan application.
  vkDestroyInstance(instance, nullptr);

  // 8. destroy window
  glfwDestroyWindow(window);

  // 9. glfw terminate
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
bool HelloTriangle::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

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
void HelloTriangle::populateDebugMessengerCreateInfo(
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

void HelloTriangle::createRenderPass() {
  //
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swapchain_image_format;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp =
      VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp =
      VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout =
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // reference object to attachment
  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout =
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // subpass description
  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint =
      VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  // render pass create info
  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType =
      VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  //
  CHECK_VK(vkCreateRenderPass(l_device.ldevice,
                              &renderPassInfo, nullptr,
                              &render_pass),
           "failed to create render pass");
}

/**
  Print validation layer output

  A static function that prints the output of the
  validation layer. The signature of the function fits to
  the @see populateDebugMessengerCreateInfo()
  content.
 */
/**
Set @see debugMessenger up by populating its related
info.
*/
void HelloTriangle::setupDebugMessenger() {
  if (!enableValidationLayers)
    return;
  //
  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);

  CHECK_VK(
      CreateDebugUtilsMessengerExt(
          instance, &createInfo, nullptr, &debugMessenger),
      "failed to create and setup debug messenger");
}
std::vector<const char *>
HelloTriangle::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(
      &glfwExtensionCount);

  // we reserve the size for the requested extensions
  // vector
  std::vector<const char *> extensions(
      glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  return extensions;
}
VkSurfaceFormatKHR HelloTriangle::chooseSwapSurfaceFormat(
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
VkPresentModeKHR HelloTriangle::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availables) {
  //
  for (const auto &apresent : availables) {
    if (apresent == VK_PRESENT_MODE_MAILBOX_KHR) {
      return apresent;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangle::chooseSwapExtent(
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
    actual_extent.width =
        std::max(capabilities.minImageExtent.width,
                 std::min(capabilities.minImageExtent.width,
                          actual_extent.width));
    actual_extent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.minImageExtent.height,
                 actual_extent.height));
    return actual_extent;
  }
}
void HelloTriangle::createSwapChain() {
  SwapChainSupportDetails swap_details =
      SwapChainSupportDetails::querySwapChainSupport(
          physical_dev.pdevice, physical_dev.surface);

  VkSurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swap_details.formats);

  VkPresentModeKHR presentMode =
      chooseSwapPresentMode(swap_details.present_modes);

  VkExtent2D extent =
      chooseSwapExtent(swap_details.capabilities);

  uint32_t img_count =
      swap_details.capabilities.minImageCount + 1;
  if (swap_details.capabilities.maxImageCount > 0 &&
      img_count > swap_details.capabilities.maxImageCount) {
    img_count = swap_details.capabilities.maxImageCount;
  }

  // Swapchain info details
  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType =
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  // surface
  createInfo.surface = physical_dev.surface;

  // image type, size etc
  createInfo.minImageCount = img_count;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueuFamilyIndices indices =
      QueuFamilyIndices::find_family_indices(
          physical_dev.pdevice, physical_dev.surface);
  uint32_t qfamily_indices[] = {
      indices.graphics_family.value(),
      indices.present_family.value()};

  if (indices.graphics_family != indices.present_family) {
    createInfo.imageSharingMode =
        VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = qfamily_indices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
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
  CHECK_VK(vkCreateSwapchainKHR(l_device.ldevice,
                                &createInfo, nullptr,
                                &swap_chain),
           "failed to create a swap chain");

  CHECK_VK(vkGetSwapchainImagesKHR(l_device.ldevice,
                                   swap_chain, &img_count,
                                   nullptr),
           "failed to reserve for swapchain images");
  swapchain_images.resize(img_count);
  CHECK_VK(vkGetSwapchainImagesKHR(l_device.ldevice,
                                   swap_chain, &img_count,
                                   swapchain_images.data()),
           "failed to set swapchain images");
  swapchain_image_format = surfaceFormat.format;
  swapchain_extent = extent;
}

/**
  Check if device support requested extensions.

  As usual we query the device for available extensions
  then check whether what we had requested is available
  by erasing from requested extensions.
 */
bool HelloTriangle::checkDeviceExtensionSupport(
    VkPhysicalDevice pdev) {
  //
  uint32_t ext_count = 0;
  vkEnumerateDeviceExtensionProperties(pdev, nullptr,
                                       &ext_count, nullptr);

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
void HelloTriangle::createSwapChainImageViews() {
  swapchain_image_views.resize(swapchain_images.size());
  for (std::size_t i = 0; i < swapchain_images.size();
       i++) {
    //
    VkImageViewCreateInfo createInfo{};
    createInfo.sType =
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = swapchain_images[i];
    /** view type can be 1d texture, 2d texture, 3d
     * textures and cubemaps*/
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = swapchain_image_format;

    /** vec.xx == vec2(vec.x, vec.x) */
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    createInfo.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    CHECK_VK(vkCreateImageView(l_device.ldevice,
                               &createInfo, nullptr,
                               &swapchain_image_views[i]),
             "failed to create image view");
  }
}
VkShaderModule HelloTriangle::createShaderModule(
    const std::vector<char> &shaderCode) {
  //
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType =
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = shaderCode.size();
  createInfo.pCode =
      reinterpret_cast<const uint32_t *>(shaderCode.data());

  VkShaderModule shaderModule;
  CHECK_VK(vkCreateShaderModule(l_device.ldevice,
                                &createInfo, nullptr,
                                &shaderModule),
           "failed to create shader module");
  return shaderModule;
}
void HelloTriangle::createGraphicsPipeline() {
  auto vxShaderCode = read_shader_file(
      "./shaders/vulkansimple/vulkansimple.vert.spv");
  auto fragShaderCode = read_shader_file(
      "./shaders/vulkansimple/vulkansimple.frag.spv");

  auto vertexModule = createShaderModule(vxShaderCode);
  auto fragModule = createShaderModule(fragShaderCode);
  //
  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertexModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragModule;
  fragShaderStageInfo.pName = "main";

  //
  VkPipelineShaderStageCreateInfo stages[] = {
      vertShaderStageInfo, fragShaderStageInfo};

  // vertex input pipeline creation
  VkPipelineVertexInputStateCreateInfo vxInputInfo{};
  vxInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vxInputInfo.vertexBindingDescriptionCount = 0;
  vxInputInfo.vertexAttributeDescriptionCount = 0;

  // input assembly pipeline creation
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology =
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // viewport configuration
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapchain_extent.width;
  viewport.height = (float)swapchain_extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // scissoring area configuration
  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swapchain_extent;

  // viewport state change configuration
  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  // rasterization state configuration
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  // multisample state configuration
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples =
      VK_SAMPLE_COUNT_1_BIT;

  // color blend attachement state configuration
  VkPipelineColorBlendAttachmentState
      colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  //
  // color blend state configuration
  VkPipelineColorBlendStateCreateInfo colorBlend{};
  colorBlend.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlend.logicOpEnable = VK_FALSE;
  colorBlend.logicOp = VK_LOGIC_OP_COPY;
  colorBlend.attachmentCount = 1;
  colorBlend.pAttachments = &colorBlendAttachment;
  colorBlend.blendConstants[0] = 0.0f;
  colorBlend.blendConstants[1] = 0.0f;
  colorBlend.blendConstants[2] = 0.0f;
  colorBlend.blendConstants[3] = 0.0f;

  // pipeline layout create info configuration
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pushConstantRangeCount = 0;

  CHECK_VK(vkCreatePipelineLayout(
               l_device.ldevice, &pipelineLayoutInfo,
               nullptr, &pipeline_layout),
           "failed to create pipeline layout");

  // create pipeline object
  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType =
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = stages;
  pipelineInfo.pVertexInputState = &vxInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlend;
  pipelineInfo.layout = pipeline_layout;
  pipelineInfo.renderPass = render_pass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  CHECK_VK(vkCreateGraphicsPipelines(
               l_device.ldevice, VK_NULL_HANDLE, 1,
               &pipelineInfo, nullptr, &graphics_pipeline),
           "failed to create graphics pipeline");

  vkDestroyShaderModule(l_device.ldevice, fragModule,
                        nullptr);
  vkDestroyShaderModule(l_device.ldevice, vertexModule,
                        nullptr);
}
void HelloTriangle::createFramebuffers() {
  swapchain_framebuffers.resize(
      swapchain_image_views.size());
  for (std::size_t i = 0; i < swapchain_image_views.size();
       i++) {
    // vk image view per frame
    VkImageView image_attachments[] = {
        swapchain_image_views[i]};
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType =
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = render_pass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = image_attachments;
    framebufferInfo.width = swapchain_extent.width;
    framebufferInfo.height = swapchain_extent.height;
    framebufferInfo.layers = 1;
    //
    CHECK_VK(
        vkCreateFramebuffer(l_device.ldevice,
                            &framebufferInfo, nullptr,
                            &swapchain_framebuffers[i]),
        "failed to create framebuffer for image view: " +
            std::to_string(i));
  }
}
void HelloTriangle::createCommandPool() {
  QueuFamilyIndices qfi =
      QueuFamilyIndices::find_family_indices(
          physical_dev.pdevice, physical_dev.surface);

  VkCommandPoolCreateInfo commandPoolInfo{};
  commandPoolInfo.sType =
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolInfo.queueFamilyIndex =
      qfi.graphics_family.value();
  CHECK_VK(vkCreateCommandPool(l_device.ldevice,
                               &commandPoolInfo, nullptr,
                               &command_pool),
           "failed to create command pool");
}
void HelloTriangle::createCommandBuffer() {
  command_buffers.resize(swapchain_framebuffers.size());

  //
  VkCommandBufferAllocateInfo comAllocInfo{};
  comAllocInfo.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  comAllocInfo.commandPool = command_pool;
  comAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  comAllocInfo.commandBufferCount =
      static_cast<uint32_t>(command_buffers.size());
  //
  CHECK_VK(
      vkAllocateCommandBuffers(l_device.ldevice,
                               &comAllocInfo,
                               command_buffers.data()),
      "failed allocate for registering command buffers");

  //
  for (std::size_t i = 0; i < command_buffers.size(); i++) {
    //
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    CHECK_VK(vkBeginCommandBuffer(command_buffers[i],
                                  &beginInfo),
             "failed to begin recording commands");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType =
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass;
    renderPassInfo.framebuffer = swapchain_framebuffers[i];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchain_extent;

    //
    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    //
    vkCmdBeginRenderPass(command_buffers[i],
                         &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(command_buffers[i],
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphics_pipeline);
    vkCmdDraw(command_buffers[i], 3, 1, 0, 0);
    vkCmdEndRenderPass(command_buffers[i]);
    CHECK_VK(vkEndCommandBuffer(command_buffers[i]),
             "failed to register command buffer");
  }
}
void HelloTriangle::createSyncObjects() {
  image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
  render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
  current_fences.resize(MAX_FRAMES_IN_FLIGHT);
  images_in_flight.resize(swapchain_images.size(),
                          VK_NULL_HANDLE);

  // create semaphore info
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType =
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  // create fences
  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    CHECK_VK(vkCreateSemaphore(
                 l_device.ldevice, &semaphoreInfo, nullptr,
                 &image_available_semaphores[i]),
             "Failed to create image available semaphore");
    CHECK_VK(vkCreateSemaphore(
                 l_device.ldevice, &semaphoreInfo, nullptr,
                 &render_finished_semaphores[i]),
             "Failed to create render finished semaphore");

    CHECK_VK(vkCreateFence(l_device.ldevice, &fenceInfo,
                           nullptr, &current_fences[i]),
             "Failed to in flight fence");
  }
}
void HelloTriangle::cleanupSwapchain() {
  //
  vkFreeCommandBuffers(
      l_device.ldevice, command_pool,
      static_cast<uint32_t>(command_buffers.size()),
      command_buffers.data());

  for (auto framebuffer : swapchain_framebuffers) {
    //
    vkDestroyFramebuffer(l_device.ldevice, framebuffer,
                         nullptr);
  }
  vkDestroyPipeline(l_device.ldevice, graphics_pipeline,
                    nullptr);
  // 1. destroy pipeline layout
  vkDestroyPipelineLayout(l_device.ldevice, pipeline_layout,
                          nullptr);
  // 2. destroy rendering pass
  vkDestroyRenderPass(l_device.ldevice, render_pass,
                      nullptr);
  // 2. destroy swap chain image views
  for (auto imview : swapchain_image_views) {
    vkDestroyImageView(l_device.ldevice, imview, nullptr);
  }
  // 3. destroy swap chain
  vkDestroySwapchainKHR(l_device.ldevice, swap_chain,
                        nullptr);
}
void HelloTriangle::recreateSwapchain() {
  //
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(l_device.ldevice);
  cleanupSwapchain();
  createSwapChain();
  createSwapChainImageViews();
  createRenderPass();
  createGraphicsPipeline();
  createFramebuffers();
  createCommandBuffer();
}
void HelloTriangle::draw() {
  vkWaitForFences(l_device.ldevice, 1,
                  &current_fences[current_frame], VK_TRUE,
                  UINT64_MAX);

  uint32_t image_index;
  VkResult res = vkAcquireNextImageKHR(
      l_device.ldevice, swap_chain, UINT64_MAX,
      image_available_semaphores[current_frame],
      VK_NULL_HANDLE, &image_index);

  if (res == VK_ERROR_OUT_OF_DATE_KHR) {
    //
    recreateSwapchain();
    return;
  } else if (res != VK_SUCCESS &&
             res != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error(
        "swap chain image request failed");
  }

  if (images_in_flight[image_index] != VK_NULL_HANDLE) {
    vkWaitForFences(l_device.ldevice, 1,
                    &images_in_flight[image_index], VK_TRUE,
                    UINT64_MAX);
  }
  images_in_flight[image_index] =
      current_fences[current_frame];

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {
      image_available_semaphores[current_frame]};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers =
      &command_buffers[image_index];

  VkSemaphore signalSemaphores[] = {
      render_finished_semaphores[current_frame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vkResetFences(l_device.ldevice, 1,
                &current_fences[current_frame]);
  //
  CHECK_VK(vkQueueSubmit(l_device.graphics_queue, 1,
                         &submitInfo,
                         current_fences[current_frame]),
           "failed to submit draw command buffer");
  //
  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swap_chains[] = {swap_chain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swap_chains;
  presentInfo.pImageIndices = &image_index;

  res = vkQueuePresentKHR(l_device.present_queue,
                          &presentInfo);

  if (res == VK_ERROR_OUT_OF_DATE_KHR ||
      res == VK_SUBOPTIMAL_KHR || framebuffer_resized) {
    framebuffer_resized = false;
    recreateSwapchain();
  } else if (res != VK_SUCCESS) {
    //
    throw std::runtime_error(
        "failed to present swap chain image");
  }

  //
  current_frame =
      (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}
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
