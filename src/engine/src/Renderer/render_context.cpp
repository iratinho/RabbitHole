// vulkan
#include "Renderer/render_context.hpp"
#include "Renderer/Swapchain.hpp"
#include "Renderer/ShaderCompiler.hpp"
#include "window.hpp"
#include "Renderer/VulkanTranslator.hpp"

bool operator==(VkSurfaceFormatKHR lhs, VkSurfaceFormatKHR rhs)
{
    return lhs.format == rhs.format && lhs.colorSpace == rhs.colorSpace;
}

#if defined(__APPLE__)
#define VULKAN_DESIRED_VERSION VK_MAKE_VERSION(1, 3, 243)
#else
#define VULKAN_DESIRED_VERSION VK_MAKE_VERSION(1, 3, 239)
#endif

#define APP_VERSION VK_MAKE_VERSION(0, 0, 1)

#define INSTANCE_DEBUG_LAYERS "VK_LAYER_KHRONOS_validation"


bool RenderContext::Initialize(const InitializationParams& initialization_params)
{
    // Load vulkan function pointers
    vulkan_loader_.Initialize();
    
    initialization_params_ = initialization_params;
    
    VALIDATE_RETURN(CreateVulkanInstance());
    VALIDATE_RETURN(CreateWindowSurface());
    VALIDATE_RETURN(PickSuitableDevice());
    VALIDATE_RETURN(CreateLogicalDevice());

    m_swapchain = std::make_shared<Swapchain>(shared_from_this());
    VALIDATE_RETURN(m_swapchain->Initialize());
    
    // VALIDATE_RETURN(CreateSwapChain());
    VALIDATE_RETURN(CreatePersistentCommandPool());

    return true;
}

bool RenderContext::CreateShader(const char* shader_path, ShaderStage shaderStage, VkPipelineShaderStageCreateInfo& shader_stage_create_info) const {
    std::vector<unsigned int> shaderCode = std::move(ShaderCompiler::Get().Compile(shader_path, shaderStage));

    VkShaderModuleCreateInfo shader_module_create_info;
    shader_module_create_info.flags = 0;
    shader_module_create_info.codeSize = shaderCode.size() * sizeof(unsigned int);
    shader_module_create_info.pCode = reinterpret_cast<const unsigned int*>(shaderCode.data());
    shader_module_create_info.pNext = nullptr;
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    VkShaderModule shader_module;
    const VkResult result = VkFunc::vkCreateShaderModule(logical_device_, &shader_module_create_info, nullptr, &shader_module);

    if (result != VK_SUCCESS)
    {
        return false;
    }

    shader_stage_create_info.flags = 0;
    shader_stage_create_info.module = shader_module;
    shader_stage_create_info.stage = TranslateShaderStage(shaderStage);
    shader_stage_create_info.pName = "main";
    shader_stage_create_info.pNext = nullptr;
    shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_create_info.pSpecializationInfo = nullptr;

    return true;
}

bool RenderContext::AcquireNextImage(VkSwapchainKHR swapchain, uint32_t& swapchainImageIndex, VkSemaphore swapchainSemaphore)
{
    const VkResult result = VkFunc::vkAcquireNextImageKHR(GetLogicalDeviceHandle(), swapchain, 0, swapchainSemaphore, VK_NULL_HANDLE, &swapchainImageIndex);
    return result != VK_ERROR_OUT_OF_DATE_KHR;
    // if(result == VK_ERROR_OUT_OF_DATE_KHR || IsSwapchainDirty())
    // {
    //     swapchain_dirty_ = false;
    // }
}

VkExtent2D RenderContext::GetSwapchainExtent() const
{
    VkExtent2D image_extent;
    image_extent.width = initialization_params_.window_->GetFramebufferSize().width;
    image_extent.height = initialization_params_.window_->GetFramebufferSize().height;

    return image_extent;
}

int RenderContext::FindMemoryTypeIndex(int memory_property_flag_bits, VkMemoryRequirements memory_requirements)
{
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    VkFunc::vkGetPhysicalDeviceMemoryProperties(GetPhysicalDeviceHandle(), &physical_device_memory_properties);

    uint32_t memory_type_index = 0;
    const uint32_t memory_type_count = physical_device_memory_properties.memoryTypeCount;
    for (uint32_t i = 0; i < memory_type_count; ++i)
    {
        // Check if we have the bit set in the memoryTypeBits
        const bool has_required_memory_type = memory_requirements.memoryTypeBits & (1 << i);
        const bool has_required_property_flag = physical_device_memory_properties.memoryTypes[i].propertyFlags &
            memory_property_flag_bits;
        if (has_required_memory_type && has_required_property_flag)
        {
            memory_type_index = i;
            break;
        }
    }

    return memory_type_index;
}

bool RenderContext::CreateVulkanInstance()
{
    if (VkFunc::vkEnumerateInstanceVersion(&loader_version_) != VK_SUCCESS)
    {
        std::cerr << "Unable to get the vulkan loader version." << std::endl;
        return false;
    }

    auto desired = VULKAN_DESIRED_VERSION;

    
    std::cout << std::printf("Current vulkan loader version: %i.%i.%i", VK_VERSION_MAJOR(loader_version_),
                             VK_VERSION_MINOR(loader_version_), VK_VERSION_PATCH(loader_version_)) << std::endl;
    std::cout << std::printf("Desired vulkan loader version: %i.%i.%i", VK_VERSION_MAJOR(VULKAN_DESIRED_VERSION),
                             VK_VERSION_MINOR(VULKAN_DESIRED_VERSION),
                             VK_VERSION_PATCH(VULKAN_DESIRED_VERSION)) << std::endl;

    // The loader version installed in the running system is lower than our application target loader
    if (VULKAN_DESIRED_VERSION > loader_version_)
    {
        std::cerr <<
            "Unable to create vulkan instance, the current installed vulkan loader does not meet the requirements for this application.\nPlease update your vulkan installation."
            << std::endl;
        return false;
    }

    // Enumerates validation layers
    uint32_t layerCount = 0;
    if (VkFunc::vkEnumerateInstanceLayerProperties(&layerCount, nullptr) != VK_SUCCESS)
    {
        std::cerr << "Unable to get vulkan layer count while using the vkEnumerateInstanceLayerProperties procedure." <<
            std::endl;
        return false;
    }

    // Gets layers properties
    std::vector<VkLayerProperties> layersProperties;
    const VkResult instance_enumeration_result = VkFunc::vkEnumerateInstanceLayerProperties(
        &layerCount, layersProperties.data());
    if (instance_enumeration_result != VK_SUCCESS)
    {
        if (instance_enumeration_result == VK_INCOMPLETE)
        {
            std::cerr << "Unable to get instance layer properties, found more properties than the requested count." <<
                std::endl;
        }
        else
        {
            std::cerr << "Unable to get instance layer properties, error not specified." << std::endl;
        }

        return false;
    }

    // Filter requested instance layers with available instance layers
    std::vector<const char*> requested_instance_layers = {INSTANCE_DEBUG_LAYERS};
    for (auto& layers_property : layersProperties)
    {
        const bool has_layer = std::find(requested_instance_layers.begin(), requested_instance_layers.end(),
                                         layers_property.layerName) != requested_instance_layers.end();
        if (!has_layer)
        {
            requested_instance_layers.erase(
                std::remove(requested_instance_layers.begin(), requested_instance_layers.end(),
                            layers_property.layerName), requested_instance_layers.end());
        }
    }

    // Gets instance extension count
    uint32_t extension_count = 0;
    if (VkFunc::vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr))
    {
        std::cerr <<
            "Unable to get vulkan instance extensions count while using the vkEnumerateInstanceExtensionProperties procedure."
            << std::endl;
        return false;
    }

    // Gets default vulkan instance extensions
    std::vector<VkExtensionProperties> extension_properties(extension_count);
    const VkResult instance_extensions_result = VkFunc::vkEnumerateInstanceExtensionProperties(
        nullptr, &extension_count, extension_properties.data());
    if (instance_extensions_result != VK_SUCCESS)
    {
        if (instance_extensions_result == VK_INCOMPLETE)
        {
            std::cerr << "Unable to get instance extension properties, found more properties than the requested count."
                << std::endl;
        }
        else
        {
            std::cerr << "Unable to get instance extensions properties, error not specified." << std::endl;
        }

        return false;
    }

    // Generate a temporary lookup set with default vulkan extensions properties
    std::unordered_set<std::string> extension_properties_lookup;
    for (const auto& extension : extension_properties)
    {
        extension_properties_lookup.insert(extension.extensionName);
    }

    // Validate that requested extensions are available
    bool found_invalid_extensions = false;
    std::vector<const char*> requested_extensions(initialization_params_.instance_extensions,
                                                        initialization_params_.instance_extensions +
                                                        initialization_params_.extension_count);
    
#if defined(__APPLE__)
    requested_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
    
    for (auto& extension : requested_extensions)
    {
        // Check if requested extension are part of default vulkan extensions list 
        if (extension_properties_lookup.find(extension) == extension_properties_lookup.end())
        {
            found_invalid_extensions |= true;
            std::cerr << std::printf(
                "[Error:] Application requested %s extensions, but the current vulkan implementation does not supports it.",
                extension) << std::endl;
        }

        // TODO: right now we are only handling the case where the requested extensions are present in the default extensions, but if we want additional ones that are not we still need to query vulkan for support
        // Query it here
    }

    // TODO: Merge non present default extensions

    if (found_invalid_extensions)
    {
        return false;
    }

    VkApplicationInfo applicationInfo;
    applicationInfo.apiVersion = VULKAN_DESIRED_VERSION;
    applicationInfo.pEngineName = "RabbitHole";
    applicationInfo.engineVersion = APP_VERSION;
    applicationInfo.pApplicationName = "RabbitHole";
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;

    VkInstanceCreateInfo instanceCreateInfo {};
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requested_instance_layers.size());
    instanceCreateInfo.ppEnabledLayerNames = requested_instance_layers.data();
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requested_extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = requested_extensions.data();
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
#if defined(__APPLE__)
    instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    const VkResult instance_result = VkFunc::vkCreateInstance(&instanceCreateInfo, nullptr, &instance_);
    if (instance_result != VK_SUCCESS)
    {
        // Last change to validate extensions and layers
        if (instance_result == VK_ERROR_LAYER_NOT_PRESENT)
        {
            std::cerr << "Instance creation and initialization has failed because of layer incompatibility.\n";
        }

        if (instance_result == VK_ERROR_EXTENSION_NOT_PRESENT)
        {
            std::cerr << "Instance creation and initialization has failed because of extension incompatibility.\n";
        }

        if (instance_result == VK_ERROR_INCOMPATIBLE_DRIVER)
        {
            std::cerr << "Instance creation and initialization has failed because of driver incompatibility.\n";
        }

        if (instance_result == VK_ERROR_OUT_OF_HOST_MEMORY || instance_result == VK_ERROR_OUT_OF_DEVICE_MEMORY ||
            instance_result == VK_ERROR_INITIALIZATION_FAILED)
        {
            std::cerr << "Instance creation and initialization has failed.\n";
        }

        return false;
    }

    vulkan_loader_.LoadInstanceLevelFunctions(instance_);
    vulkan_loader_.LoadInstanceExtensionLevelFunctions(instance_, requested_extensions);

    return true;
}

bool RenderContext::PickSuitableDevice()
{
    uint32_t device_count = 0;
    VkFunc::vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);

    std::vector<VkPhysicalDevice> physical_device_handles(device_count);
    const VkResult physical_device_result = VkFunc::vkEnumeratePhysicalDevices(instance_, &device_count,
                                                                       physical_device_handles.data());
    if (physical_device_result != VK_SUCCESS)
    {
        if (physical_device_result == VK_INCOMPLETE)
        {
            std::wcerr <<
                "Physical device enumeration was not able to provide all available devices installed in the system. \n";
        }

        if (physical_device_result != VK_INCOMPLETE)
        {
            std::cerr << "Physical device enumeration failed. \n";
            return false;
        }
    }

    std::vector<PhysicalDeviceInfo> suitable_device_infos;

    for (const VkPhysicalDevice& physical_device_handle : physical_device_handles)
    {
        PhysicalDeviceInfo device_info{};

        VkPhysicalDeviceProperties device_properties;
        VkFunc::vkGetPhysicalDeviceProperties(physical_device_handle, &device_properties);
        
        // Only consider devices that have the apiVersion matching or above the application loader version
        // the spec states that the application must not use functionality that exceeds the version of Vulkan
        // if (device_properties.apiVersion >= loader_version_)
        {
            // We also need to filter devices that have graphics and compute command queues
            uint32_t queue_family_properties_count = 0;
            VkFunc::vkGetPhysicalDeviceQueueFamilyProperties(physical_device_handle, &queue_family_properties_count, nullptr);

            std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_properties_count);
            VkFunc::vkGetPhysicalDeviceQueueFamilyProperties(physical_device_handle, &queue_family_properties_count,
                                                     queue_family_properties.data());

            uint32_t device_extensions_properties_count = 0;
            VkFunc::vkEnumerateDeviceExtensionProperties(physical_device_handle, nullptr, &device_extensions_properties_count,
                                                 nullptr);

            std::vector<VkExtensionProperties> device_extensions_properties(device_extensions_properties_count);
            VkFunc::vkEnumerateDeviceExtensionProperties(physical_device_handle, nullptr, &device_extensions_properties_count,
                                                 device_extensions_properties.data());

            VkPhysicalDeviceFeatures device_features;
            VkFunc::vkGetPhysicalDeviceFeatures(physical_device_handle, &device_features);

            std::bitset<3> flags;

            // We need to find at least one queue family that supports both operations for VK_QUEUE_GRAPHICS_BIT and VK_QUEUE_COMPUTE_BIT and supports presentation
            int graphics_queue_family_index = -1;
            int compute_queue_family_index = -1;
            int presentation_queue_family_index = -1;
            int queue_family_index = 0;
            for (const auto queue_family_property : queue_family_properties)
            {
                if (queue_family_property.queueFlags & VK_QUEUE_GRAPHICS_BIT && graphics_queue_family_index < 0)
                {
                    graphics_queue_family_index = queue_family_index;
                }

                if (queue_family_property.queueFlags & VK_QUEUE_COMPUTE_BIT && compute_queue_family_index < 0)
                {
                    compute_queue_family_index = queue_family_index;
                }

                VkBool32 allows_presentation = false;
                VkFunc::vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_handle, queue_family_index, surface_,
                                                     &allows_presentation);
                if (allows_presentation)
                {
                    presentation_queue_family_index = queue_family_index;
                }

                queue_family_index++;

                if (graphics_queue_family_index >= 0 && compute_queue_family_index >= 0 &&
                    presentation_queue_family_index >= 0)
                {
                    // we found all queues, no need to keep searching
                    flags.set(0);
                    break;
                }
            }

            // We only care about devices that support Swapchain extension
            for (auto extension : device_extensions_properties)
            {
                if (strcmp(extension.extensionName, "VK_KHR_swapchain") == 0)
                {
                    device_info.extensions.emplace("VK_KHR_swapchain");
                    flags.set(1);
                }
                
                if (strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0)
                {
                    device_info.extensions.emplace("VK_KHR_portability_subset");
                    flags.set(2);
                }
                
                if(flags.test(1) && flags.test(2)) {
                    break;
                }
            }

            // To continue all flags must be set
            if (!flags.all())
                break;

            VkPhysicalDeviceMemoryProperties memory_properties;
            VkFunc::vkGetPhysicalDeviceMemoryProperties(physical_device_handle, &memory_properties);

            device_info.physical_device = physical_device_handle;
            device_info.device_properties = device_properties;
            device_info.queue_family_index = queue_family_index;
            // TODO remove later when we support creating queues from diff queue families
            device_info.graphics_queue_family_index = graphics_queue_family_index;
            device_info.compute_queue_family_index = compute_queue_family_index;
            device_info.presentation_queue_family_index = presentation_queue_family_index;

            // There is a strong relation that the best card will normally have more memory, but might not always be the case
            // device_info.score += memory_properties.memoryHeaps[0].size;
            device_info.score *= device_properties.deviceType;
            device_info.score += device_properties.limits.maxImageDimension2D;
            device_info.score += device_properties.limits.maxImageDimension3D;
            device_info.features = device_features;
            suitable_device_infos.push_back(device_info);
        }
    }

    // Sort devices by score
    std::sort(suitable_device_infos.begin(), suitable_device_infos.end(),
              [](const PhysicalDeviceInfo& a, const PhysicalDeviceInfo& b)
              {
                  return a.score > b.score;
              });

    // If there is no overrides by the user lets just pick the device with highest score
    // TODO Add console parameters to allow override the gpu
    // if(!suitable_device_mapping.empty())
    //     physical_device_ = suitable_device_mapping[0].physical_device;

    // We didnt found any device that meets our standards
    if (suitable_device_infos.empty())
        return false;

    device_info_ = suitable_device_infos[0];

    std::cout << std::printf("\nSelected physical device information: \nName: %s", device_info_.device_properties.deviceName) << std::endl;
    std::cout << std::printf("Driver: %i.%i.%i", VK_VERSION_MAJOR(device_info_.device_properties.driverVersion),
                     VK_VERSION_MINOR(device_info_.device_properties.driverVersion), VK_VERSION_PATCH(device_info_.device_properties.driverVersion)) << std::endl;
    std::cout << std::printf("Vulkan: %i.%i.%i", VK_VERSION_MAJOR(device_info_.device_properties.apiVersion),
                         VK_VERSION_MINOR(device_info_.device_properties.apiVersion), VK_VERSION_PATCH(device_info_.device_properties.apiVersion)) << std::endl;

    return true;
}

bool RenderContext::CreateLogicalDevice()
{
    // TODO for now we assume the all queue families are the same, this might not be the case and if not we need to create a queue for them

    constexpr float queue_priority = 1.0f;

    // graphics queue
    VkDeviceQueueCreateInfo graphics_queue_info;
    graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphics_queue_info.pNext = nullptr;
    graphics_queue_info.flags = 0;
    graphics_queue_info.queueFamilyIndex = device_info_.graphics_queue_family_index;
    graphics_queue_info.queueCount = 1;
    graphics_queue_info.pQueuePriorities = &queue_priority;

    std::vector<const char*> device_extensions;
    device_extensions.insert(device_extensions.end(), device_info_.extensions.begin(), device_info_.extensions.end());
    
    VkDeviceCreateInfo device_create_info;
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = nullptr;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &graphics_queue_info;
    device_create_info.enabledExtensionCount = static_cast<int>(device_extensions.size());
    device_create_info.ppEnabledExtensionNames = device_extensions.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = nullptr;
    device_create_info.pEnabledFeatures = &device_info_.features;

    const VkResult create_device_result = VkFunc::vkCreateDevice(device_info_.physical_device, &device_create_info, nullptr,
                                                         &logical_device_);

    vulkan_loader_.LoadDeviceLevelFunctions(logical_device_);
    vulkan_loader_.LoadDeviceExtensionsLevelFunctions(logical_device_, device_extensions);
    
    // Cache the graphics queue
    auto asd = VkFunc::vkGetDeviceQueue;
    VkFunc::vkGetDeviceQueue(logical_device_, device_info_.graphics_queue_family_index, 0, &device_info_.graphics_queue);

    // Cache the present queue
    VkFunc::vkGetDeviceQueue(logical_device_, device_info_.presentation_queue_family_index, 0, &device_info_.present_queue);


    return create_device_result == VK_SUCCESS;
}

bool RenderContext::CreateWindowSurface()
{
    if (initialization_params_.window_)
    {
        surface_ = static_cast<VkSurfaceKHR>(initialization_params_.window_->CreateSurface(instance_));
        if (surface_)
        {
            return true;
        }
    }

    return false;
}

bool RenderContext::CreateSwapChain(VkSwapchainKHR& swapchain, std::vector<VkImage>& swapchainImages)
{
    uint32_t surface_formats_count = 0;
    VkFunc::vkGetPhysicalDeviceSurfaceFormatsKHR(device_info_.physical_device, surface_, &surface_formats_count, nullptr);

    std::vector<VkSurfaceFormatKHR> surface_formats(surface_formats_count);
    VkFunc::vkGetPhysicalDeviceSurfaceFormatsKHR(device_info_.physical_device, surface_, &surface_formats_count,
                                         surface_formats.data());

    VkSurfaceCapabilitiesKHR surface_capabilities;
    VkFunc::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_info_.physical_device, surface_, &surface_capabilities);

    uint32_t present_mode_count = 0;
    VkFunc::vkGetPhysicalDeviceSurfacePresentModesKHR(device_info_.physical_device, surface_, &present_mode_count, nullptr);

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    VkFunc::vkGetPhysicalDeviceSurfacePresentModesKHR(device_info_.physical_device, surface_, &present_mode_count,
                                              present_modes.data());

    VkExtent2D image_extent;
    image_extent.width = initialization_params_.window_->GetFramebufferSize().width;
    image_extent.height = initialization_params_.window_->GetFramebufferSize().height;

    // clamp extent
    image_extent.width = std::clamp(image_extent.width, surface_capabilities.minImageExtent.width,
                                    surface_capabilities.maxImageExtent.width);
    image_extent.height = std::clamp(image_extent.height, surface_capabilities.minImageExtent.height,
                                     surface_capabilities.maxImageExtent.height);

    // validations
    VkSurfaceFormatKHR swapchain_format = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    bool supports_desired_format = std::find(surface_formats.begin(), surface_formats.end(), swapchain_format) !=
        surface_formats.end();
    bool supports_composite_alpha = surface_capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    bool supports_usage_flags = surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (!supports_desired_format || !supports_composite_alpha || !supports_usage_flags)
    {
        return false;
    }

    VkPresentModeKHR desired_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
    bool supports_mailbox_present_mode = std::find(present_modes.begin(), present_modes.end(),
                                                   VK_PRESENT_MODE_MAILBOX_KHR) != present_modes.end();
    if (!supports_mailbox_present_mode)
    {
        // if we do not support mailbox at least lets try with FIFO, spec says that his is always present but lets check anyway
        bool supports_FIFO_present_mode = std::find(present_modes.begin(), present_modes.end(),
                                                    VK_PRESENT_MODE_FIFO_KHR) != present_modes.end();
        if (!supports_FIFO_present_mode)
        {
            return false;
        }

        desired_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    }

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = surface_;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.imageExtent = image_extent;
    swapchain_create_info.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
    swapchain_create_info.pNext = VK_NULL_HANDLE;
    swapchain_create_info.presentMode = desired_present_mode;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchain_create_info.minImageCount = std::clamp(2, (int)surface_capabilities.minImageCount,
                                                     (int)surface_capabilities.maxImageCount);

    /*
     * There are 2 types of imageSharingMode
     *  -> VK_SHARING_MODE_CONCURRENT swapchain images can be used across multiple queue families without any ownership transfer, this is not the best for performance.
     *  -> VK_SHARING_MODE_EXCLUSIVE swapchain images is only owned by one queue family and to allow other queue family to use it a manual ownership transfer must happen,
     *  this is the best for performance.
     *
     *  For now, while we do not have ownership transfer logic we will use VK_SHARING_MODE_CONCURRENT when the queue families from graphics and presentation do not match,
     *  this way we do not need to care about ownership transfer. Keep in mind that in the future we want to avoid using the VK_SHARING_MODE_CONCURRENT in favor of VK_SHARING_MODE_EXCLUSIVE,
     *  since its more performant. But not worth the trouble at this stage..
     */
    if (device_info_.graphics_queue_family_index != device_info_.presentation_queue_family_index)
    {
        const uint32_t queue_family_indices[] = {
            device_info_.graphics_queue_family_index, device_info_.presentation_queue_family_index
        };
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
        swapchain_create_info.queueFamilyIndexCount = 2;
    }
    else
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // swapchain_create_info.pQueueFamilyIndices = nullptr;
        // swapchain_create_info.queueFamilyIndexCount = 0;
    }

    VkResult result = VkFunc::vkCreateSwapchainKHR(logical_device_, &swapchain_create_info, nullptr, &swapchain);

    if (result == VK_SUCCESS)
    {
        // Create imageViews for swapchain images
        uint32_t image_count;
        VkFunc::vkGetSwapchainImagesKHR(logical_device_, swapchain, &image_count, nullptr);

        swapchainImages.resize(image_count);
        VkFunc::vkGetSwapchainImagesKHR(logical_device_, swapchain, &image_count, swapchainImages.data());
    }

    return result == VK_SUCCESS;
}

bool RenderContext::CreatePersistentCommandPool()
{
    VkCommandPoolCreateInfo command_pool_create_info {};
    // command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.pNext = nullptr;
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = GetGraphicsQueueIndex();

    const VkResult result = VkFunc::vkCreateCommandPool(GetLogicalDeviceHandle(), &command_pool_create_info, nullptr, &persistent_command_pool_);

    return result == VK_SUCCESS;
}

void RenderContext::MarkSwapchainDirty()
{
    m_swapchain->MarkSwapchainDirty();    
}

bool RenderContext::CreateIndexedRenderingBuffer(std::vector<uint32_t> indices, std::vector<VertexData> vertex_data, VkCommandPool command_pool, IndexRenderingData& index_rendering_data) {
    // The index buffer merged with vertex data
    std::vector<char> data;
    data.resize(sizeof(uint32_t) * indices.size() + sizeof(VertexData) * vertex_data.size());

    // Copy vertex data
    std::memcpy(data.data(), vertex_data.data(), sizeof(VertexData) * vertex_data.size());

    // Copy indices data
    size_t indices_offset = sizeof(VertexData) * vertex_data.size();
    std::memcpy(data.data() + indices_offset, indices.data(), sizeof(uint32_t) * indices.size());

    VkResult result;
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    // Staging buffer
    {
        bool result1 = AllocateBuffer(sizeof(char) * data.size(), staging_buffer, staging_buffer_memory
            , VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            , VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        
        if (!result1) {
            return false;
        }
        
        // Associate our buffer with this memory
        VkFunc::vkBindBufferMemory(logical_device_, staging_buffer, staging_buffer_memory, 0);

        // Copy the vertex data 
        void* buffer_data;
        VkFunc::vkMapMemory(logical_device_, staging_buffer_memory, 0, sizeof(char) * data.size(), 0, &buffer_data);
        memcpy(buffer_data, data.data(), sizeof(char) * data.size());
        VkFunc::vkUnmapMemory(logical_device_, staging_buffer_memory);
    }

    // device local buffer
    {
        VkBuffer buffer;
        VkDeviceMemory buffer_memory;

        bool result = AllocateBuffer(sizeof(char) * data.size(), buffer, buffer_memory
            , VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
        if (!result) {
            return false;
        }

        // Associate our buffer with this memory
        VkFunc::vkBindBufferMemory(logical_device_, buffer, buffer_memory, 0);

        // Temporary command buffer to do a transfer operation for our gpu buffer
        VkCommandBufferAllocateInfo command_buffer_allocate_info{};
        command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandPool = persistent_command_pool_;
        command_buffer_allocate_info.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        VkFunc::vkAllocateCommandBuffers(logical_device_, &command_buffer_allocate_info, &commandBuffer);

        VkCommandBufferBeginInfo transfer_command_buffer_begin_info{};
        transfer_command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        transfer_command_buffer_begin_info.pNext = nullptr;
        transfer_command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VkFunc::vkBeginCommandBuffer(commandBuffer, &transfer_command_buffer_begin_info);

        VkBufferCopy copyRegion{};
        copyRegion.size = sizeof(char) * data.size();

        VkFunc::vkCmdCopyBuffer(commandBuffer, staging_buffer, buffer, 1, &copyRegion);

        VkFunc::vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &commandBuffer;

        VkFenceCreateInfo fence_create_info;
        fence_create_info.flags = 0;
        fence_create_info.pNext = nullptr;
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        VkFence fence;
        VkFunc::vkCreateFence(logical_device_, &fence_create_info, nullptr, &fence);
        VkFunc::vkQueueSubmit(GetGraphicsQueueHandle(), 1, &submit_info, fence);

        // Ensure that GPU finishes executing
        VkFunc::vkWaitForFences(logical_device_, 1, &fence, VK_TRUE, UINT64_MAX);
        VkFunc::vkDestroyFence(logical_device_, fence, nullptr);
        VkFunc::vkResetCommandPool(logical_device_, persistent_command_pool_, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

        index_rendering_data.buffer = buffer;
        index_rendering_data.indices_offset = sizeof(VertexData) * vertex_data.size();
        index_rendering_data.indices_count = static_cast<uint32_t>(indices.size());
        index_rendering_data.vertex_data_offset = 0;

        // vkFreeCommandBuffers(logical_device_, command_pool, 1, &commandBuffer);
    }

    // Clean up staging buffer
    VkFunc::vkDestroyBuffer(logical_device_, staging_buffer, nullptr);
    VkFunc::vkFreeMemory(logical_device_, staging_buffer_memory, nullptr);
    
    return true;
}

void RenderContext::DestroyImageView(VkImageView image_view) {
    VkFunc::vkDestroyImageView(GetLogicalDeviceHandle(), image_view, nullptr);
}

void RenderContext::DestroyImage(VkImage image) {
    VkFunc::vkDestroyImage(GetLogicalDeviceHandle(), image, nullptr);
}

void RenderContext::DestroyFrameBuffer(VkFramebuffer framebuffer) {
    VkFunc::vkDestroyFramebuffer(GetLogicalDeviceHandle(), framebuffer, nullptr);
}

void RenderContext::DestroyCommandPool(VkCommandPool command_pool) {
    VkFunc::vkDestroyCommandPool(GetLogicalDeviceHandle(), command_pool, nullptr);
}

VkFence RenderContext::AllocateFence(VkFenceCreateInfo fence_create_info) {
    VkFence fence = VK_NULL_HANDLE;
    VkResult result = VkFunc::vkCreateFence(GetLogicalDeviceHandle(), &fence_create_info, nullptr, &fence);
    if(result != VK_SUCCESS)
        return VK_NULL_HANDLE;

    return fence;
}

void RenderContext::ResetFence(VkFence fence) {
    VkFunc::vkResetFences(GetLogicalDeviceHandle(), 1, &fence);
}

void RenderContext::WaitFence(VkFence fence) {
    VkFunc::vkWaitForFences(GetLogicalDeviceHandle(), 1, &fence, VK_TRUE, UINT64_MAX);
}

VkDescriptorPool RenderContext::CreateDescriptorPool(unsigned int uniformsCount, unsigned int samplersCount) {
    VkDescriptorPoolSize uniformPoolSize = {};
    uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformPoolSize.descriptorCount = uniformsCount;
    
    VkDescriptorPoolSize samplersPoolSize = {};
    samplersPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplersPoolSize.descriptorCount = samplersCount;
    
    std::array<VkDescriptorPoolSize, 2> poolSizes = { uniformPoolSize, samplersPoolSize };
    
    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.maxSets = 1000; // High number, so that when we are full we create a new pool
    poolCreateInfo.pPoolSizes = poolSizes.data();
    poolCreateInfo.poolSizeCount = poolSizes.size();
    poolCreateInfo.flags = 0;
    poolCreateInfo.pNext = nullptr;
    
    VkDescriptorPool pool;
    VkResult result = VkFunc::vkCreateDescriptorPool(logical_device_, &poolCreateInfo, nullptr, &pool);
    if (result == VK_SUCCESS) {
        return pool;
    }

    return VK_NULL_HANDLE;
}

VkCommandPool RenderContext::CreateCommandPool()
{
    VkCommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.pNext = nullptr;
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = GetGraphicsQueueIndex();

    VkCommandPool command_pool;
    const VkResult result = VkFunc::vkCreateCommandPool(GetLogicalDeviceHandle(), &command_pool_create_info, nullptr, &command_pool);
    if (result == VK_SUCCESS) {
        return command_pool;
    }
    
    return VK_NULL_HANDLE;
}

void RenderContext::ResetCommandPool(VkCommandPool commandPool) {
    VkFunc::vkResetCommandPool(GetLogicalDeviceHandle(), commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
}

VkCommandBuffer RenderContext::CreateCommandBuffer(void* commandPool)
{
    VkCommandBuffer commandBuffer;
    
    VkCommandBufferAllocateInfo command_buffer_allocate_info;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandPool = static_cast<VkCommandPool>(commandPool);
    command_buffer_allocate_info.pNext = nullptr;
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandBufferCount = 1;

    const VkResult result = VkFunc::vkAllocateCommandBuffers(GetLogicalDeviceHandle(), &command_buffer_allocate_info, &commandBuffer);
    if (result == VK_SUCCESS) {
        return commandBuffer;
    }

    return VK_NULL_HANDLE;
}

bool RenderContext::BeginCommandBuffer(void* commandBuffer) {
    VkCommandBufferBeginInfo command_buffer_begin_info;
    command_buffer_begin_info.flags = 0;
    command_buffer_begin_info.pNext = nullptr;
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    return VK_SUCCESS == VkFunc::vkBeginCommandBuffer(static_cast<VkCommandBuffer>(commandBuffer), &command_buffer_begin_info);
}

bool RenderContext::EndCommandBuffer(void* commandBuffer) {
    return VK_SUCCESS == VkFunc::vkEndCommandBuffer(static_cast<VkCommandBuffer>(commandBuffer));
}

bool RenderContext::AllocateBuffer(size_t allocationSize, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags) {
    VkBufferCreateInfo bufferCreateInfo {};
    bufferCreateInfo.flags = 0;
    bufferCreateInfo.size = allocationSize;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.pNext = nullptr;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    VkResult result = VkFunc::vkCreateBuffer(logical_device_, &bufferCreateInfo, nullptr, &buffer);

    if (result != VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements memory_requirements;
    VkFunc::vkGetBufferMemoryRequirements(logical_device_, buffer, &memory_requirements);

    int memory_type_index = FindMemoryTypeIndex(
        memoryFlags, memory_requirements);

    VkMemoryAllocateInfo memoryAllocateInfo {};
    memoryAllocateInfo.memoryTypeIndex = memory_type_index;
    memoryAllocateInfo.allocationSize = memory_requirements.size;
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = nullptr;

    result = VkFunc::vkAllocateMemory(logical_device_, &memoryAllocateInfo, nullptr, &bufferMemory);

    if (result != VK_SUCCESS) {
        return false;
    }

    // Associate our buffer with this memory
    VkFunc::vkBindBufferMemory(logical_device_, buffer, bufferMemory, 0);

    return true;
}

void* RenderContext::LockBuffer(VkDeviceMemory bufferMemory, size_t allocationSize) const {
    void* buffer = nullptr;
    VkFunc::vkMapMemory(logical_device_, bufferMemory, 0, allocationSize, 0, &buffer);
    return buffer;
}

void RenderContext::UnlockBuffer(VkDeviceMemory bufferMemory) const {
    VkFunc::vkUnmapMemory(logical_device_, bufferMemory);
}

void RenderContext::CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer src, VkBuffer dest, size_t allocationSize) {
    VkBufferCopy copyRegion{};
    copyRegion.size = allocationSize;

    VkFunc::vkCmdCopyBuffer(commandBuffer, src, dest, 1, &copyRegion);

}
