#include "VulkanRenderer.hpp"
#include "VulkanImageUtilities.hpp"

void VulkanRenderer::CreateGLFWWindow(int width, int height, std::string name) {

    glfwInit();
    // NO OPENGL CONTEXT, WE USE VULKAN
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create the window.
    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void VulkanRenderer::CreateVulkanInstance(VulkanInstanceInfo instanceInfo)
{
    // Check for validation layer support.
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    // Define Info about the application for drivers.
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = instanceInfo.ApplicationName.c_str();
    appInfo.applicationVersion = instanceInfo.ApplicationVersion;
    appInfo.pEngineName = "Ryan's Simple Vulkan Renderer";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Tell Vulkan which extensions and validation layers to use.
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Get the required extensions.
    auto extensions = getRequiredExtensions();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // Enable validation layers.
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    // Create the Vulkan Instace. Pass in the info and instance variable by refrence. The instance var is populated by this method.
    // Pointer to struct with creation info, pointer to custom allocater, pointer to variable that stores the handle for the new object.
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw new std::runtime_error("Failed to create Vulkan Instance!");
    }
}

/// <summary>
/// Setup the Vulkan Debug Messenger if desired.
/// 
/// Access VkDebugUtilsMessengerEXT through the debugMessenger class property.
/// 
/// Throws a runtime error if it does not succeed.
/// </summary>
void VulkanRenderer::SetupDebugMessenger()
{
    if (!enableValidationLayers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

/// <summary>
/// Create the GLFW window surface.
/// 
/// Throws a runtime error if it does not succeed.
/// </summary>
void VulkanRenderer::CreateGLFWSurface()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface!");
    }
}

/// <summary>
/// Have the renderer select the graphics card to use.
/// 
/// By default, the renderer will select a card for you and
/// throws a runtime error if one cannot be found.
/// 
/// This method will populate the physicalDevice property.
/// </summary>
void VulkanRenderer::SelectPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        // TODO:: Make this function configuraeable. 
        if (isDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Unable to find a suitable GPU!");
    }
}

/// <summary>
/// Select Queue Families from the physical device to use and create a device to use.
/// 
/// This also creates the graphics and presentation queues that can be used.
/// 
/// Throws a runtime error if a logical device cannot be created with
/// the default requirements.
/// </summary>
void VulkanRenderer::CreateLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Define the device features. Theis was done using vkGetPhysicalDeviceFeatures
    VkPhysicalDeviceFeatures deviceFeatures{};
    // Ask for the the Anisotrpy feature for the texture image sampler.
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    // The main device creation
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    // Enable extensions for the logical device
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // Modern implementations will ignore these as device layers are deprecated.
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else 
    {
        createInfo.enabledLayerCount = 0;
    }

    // The params are the physicalDevice to interface with (GPU), the queue and usage info specified, allocation call back,
    // and a pointer to a variable to store the logical device handle in (the one defined in the private section of the class).
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical deivce!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

/// <summary>
/// Create the swap chain to use and the image views that go with it.
/// </summary>
/// <param name="swapChainInfo">Configuration of the swap chain.</param>
void VulkanRenderer::SetupSwapChain(const SwapChainDescriptor descriptor)
{
    mSwapChain = std::make_shared<VulkanSwapChain>(descriptor);
    mSwapChain->InitializeSwapChain(window, surface, physicalDevice, device);
    CreateRenderPass();
    mSwapChain->CreateDepthImage(physicalDevice, device);
    mSwapChain->CreateFrameBuffers(device, renderPass);
}

/// <summary>
/// Create the main render pass for the renderer.
/// </summary>
void VulkanRenderer::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = mSwapChain->ImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // Load and clear the framebuffer to black.
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // Render contents will be stored in memory and can be read later.
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // The layout is for images to be presented in the swap chain.
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat(physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void VulkanRenderer::CreateDescriptorSetLayout()
{
    // A descriptor for shaders.
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    // The binding that matches with the shader code.
    uboLayoutBinding.binding = 0;
    // The type is a uniform
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // A uniform object could be an arary. This defines the number of objects.
    uboLayoutBinding.descriptorCount = 1;
    // We need to specifiy what stage the descriptor is going to be used. In this case
    // it is the vertex stage. (Multiple can be used)
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    // For image sampling descriptors.
    uboLayoutBinding.pImmutableSamplers = nullptr;

    // Create the descripter for image sampling for textures.
    // This is done in the fragment shader stange instead of the vertex stage.
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // The array of descriptor bindings.
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    // Create the descriptor set layout.
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void VulkanRenderer::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& descriptor)
{
    if (mGraphicsPipeline != nullptr)
    {
        throw std::runtime_error("Graphics Pipeline already exists!");
    }
    mGraphicsPipeline = std::make_shared<VulkanGraphicsPipeline>( descriptor);
    mGraphicsPipeline->UpdatePipeline(device, renderPass, mSwapChain->Extent(), descriptorSetLayout);
}

/// <summary>
/// Create the semaphores and fences required to ensure frames
/// are rendered in the correct order.
/// </summary>
void VulkanRenderer::CreateSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(mSwapChain->Images().size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }
    }
}