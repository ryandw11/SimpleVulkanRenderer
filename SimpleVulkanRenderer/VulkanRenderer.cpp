#include "VulkanRenderer.hpp"
#include "VulkanImageUtilities.hpp"

namespace
{
    // A GLFW callback for when the window is resized.
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {
        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    // Check if extensions are suppored for a sepecific physical device.
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        // Remove required extension if it exists.
        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        // Extensions are supported if the list is empty.
        return requiredExtensions.empty();
    }

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // Find a queue family that supports QUEUE GRAPHICS BIT.

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            // Find one that supports presentation. (Can be different than the graphics queue)
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            // Check if the int is populated.
            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    // This checks to make sure that the Device can handle all of the
    // features that we use.
    bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        QueueFamilyIndices indicies = FindQueueFamilies(device, surface);

        bool extensionSupported = CheckDeviceExtensionSupport(device);

        // Check is swap chain is adequate.
        bool swapChainAdequate = false;
        if (extensionSupported) {
            VulkanSwapChain::SwapChainSupportDetails swapChainSupport = VulkanSwapChain::QuerySwapChainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        // Get PhysicalDevice features.
        VkPhysicalDeviceFeatures supportedFeatures{};
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indicies.isComplete() && extensionSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    // Get the list of required extensions.
    std::vector<const char*> getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    std::vector<float> QueuePriorities(VulkanQueueType type, std::vector<VulkanQueueDescriptor>& queueDescriptors)
    {
        std::vector<float> priorities;
        for (const auto& queueDescriptor : queueDescriptors)
        {
            if (queueDescriptor.Type == type)
            {
                priorities.push_back(queueDescriptor.Priority);
            }
        }

        return priorities;
    }
}

void VulkanRenderer::CreateGLFWWindow(int width, int height, std::string name)
{

    glfwInit();
    // NO OPENGL CONTEXT, WE USE VULKAN
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create the window.
    mWindow = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
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

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    // Create the Vulkan Instace. Pass in the info and instance variable by refrence. The instance var is populated by this method.
    // Pointer to struct with creation info, pointer to custom allocater, pointer to variable that stores the handle for the new object.
    if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
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
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

void VulkanRenderer::AutoInitialize(VulkanAutoInitSettings settings, std::function<void(Ptr(VulkanDescriptorLayout))> descriptorLayoutBuilderStage, std::function<GraphicsPipelineDescriptor()> pipelineDescriptionStage, std::function<void()> loadingStage, std::function<void(Ptr(VulkanDescriptorSetBuilder))> descriptorSetCreationStage)
{
    CreateGLFWWindow(settings.WindowWidth, settings.WindowHeight, "Test Renderer Application");
    CreateVulkanInstance(settings.InstanceInfo);
    if (settings.SetupDebug)
    {
        SetupDebugMessenger();
    }
    CreateGLFWSurface();
    SelectPhysicalDevice();
    CreateLogicalDevice(settings.CustomQueues);
    SetupSwapChain(settings.SwapChainDescriptor);

    mDescriptorHandler = std::make_shared<VulkanDescriptorLayout>(mDevice);
    descriptorLayoutBuilderStage(mDescriptorHandler);
    mDescriptorHandler->BuildLayout();

    CreateGraphicsPipeline(pipelineDescriptionStage());
    CreateDefaultCommandPool("DefaultCommandPool");
    CreateBufferUtilities();

    loadingStage();

    mDescriptorHandler->CreateDescriptorPool(mSwapChain->FrameBuffers().size());
    auto descriptorSetBuilder = mDescriptorHandler->DescriptorSetBuilder();
    descriptorSetCreationStage(descriptorSetBuilder);
    descriptorSetBuilder->UpdateDescriptorSets();

    CreateDefaultRenderCommandBuffers();
}

/// <summary>
/// Create the GLFW window surface.
/// 
/// Throws a runtime error if it does not succeed.
/// </summary>
void VulkanRenderer::CreateGLFWSurface()
{
    if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS)
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
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        // TODO:: Make this function configuraeable. 
        if (IsDeviceSuitable(device, mSurface))
        {
            mPhysicalDevice = device;
            break;
        }
    }

    if (mPhysicalDevice == VK_NULL_HANDLE)
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
void VulkanRenderer::CreateLogicalDevice(std::vector<VulkanQueueDescriptor> queues)
{
    // Find and create the default queue for graphics and presentation.
    QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice, mSurface);
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    float queuePriority = 1.0f;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    std::vector<uint32_t> queueUsage(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, queueFamilies.data());

    std::vector<std::vector<float>> queuePriorities(queueFamilyCount);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.resize(queueFamilyCount);

    for (int i = 0; i < queueFamilyCount; i++)
    {
        queueUsage[i] = queueFamilies[i].queueCount;
    }

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        queuePriorities[queueFamily].push_back(1);
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = queuePriorities[queueFamily].data();
        queueCreateInfos[queueFamily] = queueCreateInfo;
        queueUsage[queueFamily] -= 1;
    }

    for (auto& vulkanQueue : queues)
    {
        for (int i = 0; i < queueFamilyCount; i++)
        {
            auto queueFamily = queueFamilies[i];
            if (vulkanQueue.Type == COMPUTE_QUEUE && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                if (queueUsage[i] > 0)
                {
                    if (uniqueQueueFamilies.find(i) == uniqueQueueFamilies.end())
                    {
                        queuePriorities[i].push_back(1);
                        VkDeviceQueueCreateInfo queueCreateInfo{};
                        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                        queueCreateInfo.queueFamilyIndex = i;
                        queueCreateInfo.queueCount = 1;
                        queueCreateInfo.pQueuePriorities = queuePriorities[i].data();
                        queueCreateInfos[i] = queueCreateInfo;
                    }
                    else
                    {
                        queueCreateInfos[i].queueCount += 1;
                        queuePriorities[i].push_back(1);
                        queueCreateInfos[i].pQueuePriorities = queuePriorities[i].data();
                    }
                    vulkanQueue._QueueFamily = i;
                    vulkanQueue._QueueIndex = queueFamily.queueCount - queueUsage[i];
                    queueUsage[i] -= 1;
                    uniqueQueueFamilies.insert(i);
                    goto outerloop; // Acts as a continue; for the outer loop.
                }
            }
        }
        std::cout << "CRITICAL ERROR: Unable to find desire queue for creation!" << std::endl;
    outerloop:;
    }

    std::vector<VkDeviceQueueCreateInfo> finalQueueCreateInfos;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        finalQueueCreateInfos.push_back(queueCreateInfos[queueFamily]);
    }

    // Define the device features. This was done using vkGetPhysicalDeviceFeatures
    VkPhysicalDeviceFeatures deviceFeatures{};
    // Ask for the the Anisotrpy feature for the texture image sampler.
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    // The main device creation
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = finalQueueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(finalQueueCreateInfos.size());
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
    if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical deivce!");
    }

    // Get the default queues
    vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mDefaultGraphicsQueue);
    vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);

    // Populate the queue map
    for (auto queue : queues)
    {
        VkQueue vkQueue;
        vkGetDeviceQueue(mDevice, queue._QueueFamily, queue._QueueIndex, &vkQueue);
        VulkanQueue vulkanQueue;
        vulkanQueue.queue = vkQueue;
        vulkanQueue.QueueFamily = queue._QueueFamily;
        vulkanQueue.QueueIndex = queue._QueueIndex;
        mQueueMap[queue.Name] = vulkanQueue;
    }
}

/// <summary>
/// Create the swap chain to use and the image views that go with it.
/// </summary>
/// <param name="swapChainInfo">Configuration of the swap chain.</param>
void VulkanRenderer::SetupSwapChain(const SwapChainDescriptor descriptor)
{
    mSwapChain = std::make_shared<VulkanSwapChain>(mDevice, descriptor);
    mSwapChain->InitializeSwapChain(mWindow, mSurface, mPhysicalDevice);
    CreateRenderPass();
    mSwapChain->CreateDepthImage(mPhysicalDevice);
    mSwapChain->CreateFrameBuffers(mRenderPass);
    mSwapChain->CreateSyncObjects();
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
    depthAttachment.format = FindDepthFormat(mPhysicalDevice);
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

    if (vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void VulkanRenderer::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& descriptor)
{
    if (mGraphicsPipeline != nullptr)
    {
        throw std::runtime_error("Graphics Pipeline already exists!");
    }
    mGraphicsPipeline = std::make_shared<VulkanGraphicsPipeline>(descriptor);
    mGraphicsPipeline->UpdatePipeline(mDevice, mRenderPass, mDescriptorHandler->Layout());
}
