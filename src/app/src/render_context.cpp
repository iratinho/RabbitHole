#include "render_context.h"
#include "window.h"

bool operator==(VkSurfaceFormatKHR lhs, VkSurfaceFormatKHR rhs){
    return lhs.format==rhs.format && lhs.colorSpace==rhs.colorSpace;
}

#define VULKAN_DESIRED_VERSION VK_MAKE_VERSION(1, 3, 201)
#define APP_VERSION VK_MAKE_VERSION(0, 0, 1)

#define ENABLE_INSTANCE_LAYERS // TODO for now hard-coded but it should cmake to define this
#ifdef ENABLE_INSTANCE_LAYERS
    #define INSTANCE_DEBUG_LAYERS {"VK_LAYER_KHRONOS_validation"}
#else
    #define INSTANCE_DEBUG_LAYERS {}
#endif

// TODO do vulkan call checking more universal, so we dont need to handle all cases all the time...

namespace app::renderer {
    bool RenderContext::Initialize(const InitializationParams& initialization_params) {
        initialization_params_ = initialization_params;

        bool b_initialized = false;
        b_initialized = CreateVulkanInstance();
        b_initialized = CreateWindowSurface();
        b_initialized = PickSuitableDevice();
        b_initialized = CreateLogicalDevice();
        b_initialized = CreateSwapChain();
        b_initialized = CreatePersistentCommandPool();

        return b_initialized;
    }

    bool RenderContext::CreateShader(const char* shader_path, VkShaderStageFlagBits shader_stage, VkPipelineShaderStageCreateInfo& shader_stage_create_info) {
        std::ifstream shader_file;
        shader_file.open(shader_path, std::ios::binary);

        if(!shader_file.is_open()) {
            return false;
        }
        
        std::stringstream shader_buffer;
        shader_buffer << shader_file.rdbuf();

        const std::string shader_code = shader_buffer.str();
        
        VkShaderModuleCreateInfo shader_module_create_info;
        shader_module_create_info.flags = 0;
        shader_module_create_info.codeSize = shader_code.size() * sizeof(char);
        shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(shader_code.data());
        shader_module_create_info.pNext = nullptr;
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        
        VkShaderModule shader_module;
        const VkResult result = vkCreateShaderModule(logical_device_, &shader_module_create_info, nullptr, &shader_module);

        if(result != VK_SUCCESS) {
            return false;
        }

       shader_stage_create_info.flags = 0;
       shader_stage_create_info.module = shader_module;
       shader_stage_create_info.stage = shader_stage;
       shader_stage_create_info.pName = "main";
       shader_stage_create_info.pNext = nullptr;
       shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
       shader_stage_create_info.pSpecializationInfo = nullptr;

        return true;
    }

    bool RenderContext::RecreateSwapchain() {
        // Cleanup image views
        for (const auto swapchain_image : swapchain_images_) {
            vkDestroyImageView(logical_device_, swapchain_image.color_image_view, nullptr);

            if(swapchain_image.depth_image_view != VK_NULL_HANDLE) {
                vkDestroyImageView(logical_device_, swapchain_image.depth_image_view, nullptr);
            }
                
            if(swapchain_image.depth_image != VK_NULL_HANDLE) {
                vkDestroyImage(logical_device_, swapchain_image.depth_image, nullptr);
            }

            // NASTY and temporary
            for (auto& _swapchain_image : swapchain_images_) {
                _swapchain_image.depth_image_view = nullptr;
                _swapchain_image.depth_image = nullptr;
            }
        }
        
        swapchain_images_.clear();

        vkDestroySwapchainKHR(logical_device_, swapchain_, nullptr);
        CreateSwapChain();
        
        return true;
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
        vkGetPhysicalDeviceMemoryProperties(GetPhysicalDeviceHandle(), &physical_device_memory_properties);
        
        uint32_t memory_type_index = 0;
        const uint32_t memory_type_count = physical_device_memory_properties.memoryTypeCount;
        for (uint32_t i = 0; i < memory_type_count; ++i) {
            // Check if we have the bit set in the memoryTypeBits
            const bool has_required_memory_type = memory_requirements.memoryTypeBits & (1 << i);
            const bool has_required_property_flag = physical_device_memory_properties.memoryTypes[i].propertyFlags & memory_property_flag_bits;
            if(has_required_memory_type && has_required_property_flag) {
                memory_type_index = i;
                break;
            }
        }

        return memory_type_index;
    }
    
    bool RenderContext::CreateVulkanInstance() {
        if(vkEnumerateInstanceVersion(&loader_version_) != VK_SUCCESS) {
            std::cerr << "Unable to get the vulkan loader version." << std::endl;
            return false;
        }

        std::cout << std::printf("Current vulkan loader version: %i.%i.%i", VK_VERSION_MAJOR(loader_version_), VK_VERSION_MINOR(loader_version_), VK_VERSION_PATCH(loader_version_)) << std::endl;
        std::cout << std::printf("Desired vulkan loader version: %i.%i.%i", VK_VERSION_MAJOR(VULKAN_DESIRED_VERSION), VK_VERSION_MINOR(VULKAN_DESIRED_VERSION), VK_VERSION_PATCH(VULKAN_DESIRED_VERSION)) << std::endl;

        // The loader version installed in the running system is lower than our application target loader
        if(VULKAN_DESIRED_VERSION > loader_version_) {
            std::cerr << "Unable to create vulkan instance, the current installed vulkan loader does not meet the requirements for this application.\nPlease update your vulkan installation." << std::endl;
            return false;
        }

        // Enumerates validation layers
        uint32_t layerCount = 0;
        if(vkEnumerateInstanceLayerProperties(&layerCount, nullptr) != VK_SUCCESS) {
            std::cerr << "Unable to get vulkan layer count while using the vkEnumerateInstanceLayerProperties procedure." << std::endl;
            return false;
        }

        // Gets layers properties
        std::vector<VkLayerProperties> layersProperties;
        const VkResult instance_enumeration_result = vkEnumerateInstanceLayerProperties(&layerCount, layersProperties.data());
        if(instance_enumeration_result != VK_SUCCESS) {
            if(instance_enumeration_result == VK_INCOMPLETE) {
                std::cerr << "Unable to get instance layer properties, found more properties than the requested count." << std::endl;
            } else {
                std::cerr << "Unable to get instance layer properties, error not specified." << std::endl;
            }

            return false;
        }

        // Filter requested instance layers with available instance layers
        std::vector<const char*> requested_instance_layers = INSTANCE_DEBUG_LAYERS;
        for (auto& layers_property : layersProperties) {
            const bool has_layer = std::find(requested_instance_layers.begin(), requested_instance_layers.end(), layers_property.layerName) != requested_instance_layers.end();
            if(!has_layer) {
                requested_instance_layers.erase(std::remove(requested_instance_layers.begin(), requested_instance_layers.end(), layers_property.layerName), requested_instance_layers.end());
            }
        }

        // Gets instance extension count
        uint32_t extension_count = 0;
        if(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr)) {
            std::cerr << "Unable to get vulkan instance extensions count while using the vkEnumerateInstanceExtensionProperties procedure." << std::endl;
            return false;
        }

        // Gets default vulkan instance extensions
        std::vector<VkExtensionProperties> extension_properties(extension_count);
        const VkResult instance_extensions_result = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extension_properties.data());
        if(instance_extensions_result != VK_SUCCESS) {
            if(instance_extensions_result == VK_INCOMPLETE) {
                std::cerr << "Unable to get instance extension properties, found more properties than the requested count." << std::endl;
            } else {
                std::cerr << "Unable to get instance extensions properties, error not specified." << std::endl;
            }

            return false;
        }

        // Build a temporary lookup set with default vulkan extensions properties
        std::unordered_set<std::string> extension_properties_lookup;
        for (const auto& extension : extension_properties) {
            extension_properties_lookup.insert(extension.extensionName);
        }
        
        // Validate that requested extensions are available
        bool found_invalid_extensions = false;
        const std::vector<const char*> requested_extensions(initialization_params_.instance_extensions, initialization_params_.instance_extensions + initialization_params_.extension_count);
        for (auto& extension : requested_extensions) {
            // Check if requested extension are part of default vulkan extensions list 
            if(extension_properties_lookup.find(extension) == extension_properties_lookup.end()) {
                found_invalid_extensions |= true;
                std::cerr << std::printf("[Error:] Application requested %s extensions, but the current vulkan implementation does not supports it.", extension) << std::endl;
            }

            // TODO: right now we are only handling the case where the requested extensions are present in the default extensions, but if we want additional ones that are not we still need to query vulkan for support
            // Query it here
        }

        // TODO: Merge non present default extensions
        
        if(found_invalid_extensions) {
            return false;
        }
        
        VkApplicationInfo applicationInfo;
        applicationInfo.apiVersion = VULKAN_DESIRED_VERSION;
        applicationInfo.pEngineName = "RabbitHole";
        applicationInfo.engineVersion = APP_VERSION;
        applicationInfo.pApplicationName = "RabbitHole";
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pNext = nullptr;
        
        VkInstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requested_instance_layers.size());
        instanceCreateInfo.ppEnabledLayerNames = requested_instance_layers.data();
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requested_extensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = requested_extensions.data();
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext = nullptr;
        instanceCreateInfo.flags = 0;
        
        const VkResult instance_result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance_);
        if(instance_result != VK_SUCCESS) {
            // Last change to validate extensions and layers
            if(instance_result == VK_ERROR_LAYER_NOT_PRESENT) {
                std::cerr << "Instance creation and initialization has failed because of layer incompatibility.\n";
            }

            if(instance_result == VK_ERROR_EXTENSION_NOT_PRESENT) {
                std::cerr << "Instance creation and initialization has failed because of extension incompatibility.\n";
            }

            if(instance_result == VK_ERROR_INCOMPATIBLE_DRIVER) {
                std::cerr << "Instance creation and initialization has failed because of driver incompatibility.\n";
            }

            if(instance_result == VK_ERROR_OUT_OF_HOST_MEMORY || instance_result == VK_ERROR_OUT_OF_DEVICE_MEMORY || instance_result == VK_ERROR_INITIALIZATION_FAILED) {
                std::cerr << "Instance creation and initialization has failed.\n";
            }

            return false;
        }
        
        return true;
    }

    bool RenderContext::PickSuitableDevice() {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);

        std::vector<VkPhysicalDevice> physical_device_handles(device_count);
        const VkResult physical_device_result = vkEnumeratePhysicalDevices(instance_, &device_count, physical_device_handles.data());
        if(physical_device_result != VK_SUCCESS) {
            if(physical_device_result == VK_INCOMPLETE) {
                std::wcerr << "Physical device enumeration was not able to provide all available devices installed in the system. \n";
            }

            if(physical_device_result != VK_INCOMPLETE) {
                std::cerr << "Physical device enumeration failed. \n";
                return false;
            }
        }

        PhysicalDeviceInfo device_info {};
        std::vector<PhysicalDeviceInfo> suitable_device_infos;

        for (const VkPhysicalDevice& physical_device_handle : physical_device_handles) {
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(physical_device_handle, &device_properties);

            // Only consider devices that have the apiVersion matching or above the application loader version
            // the spec states that the application must not use functionality that exceeds the version of Vulkan
            if(device_properties.apiVersion >= loader_version_) {
                // We also need to filter devices that have graphics and compute command queues
                uint32_t queue_family_properties_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(physical_device_handle, &queue_family_properties_count, nullptr);

                std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_properties_count);
                vkGetPhysicalDeviceQueueFamilyProperties(physical_device_handle, &queue_family_properties_count, queue_family_properties.data());

                uint32_t device_extensions_properties_count = 0;
                vkEnumerateDeviceExtensionProperties(physical_device_handle, nullptr, &device_extensions_properties_count, nullptr);

                std::vector<VkExtensionProperties> device_extensions_properties(device_extensions_properties_count);
                vkEnumerateDeviceExtensionProperties(physical_device_handle, nullptr, &device_extensions_properties_count, device_extensions_properties.data());

                VkPhysicalDeviceFeatures device_features;
                vkGetPhysicalDeviceFeatures(physical_device_handle, &device_features);
                
                std::bitset<3> flags;

                // We need to find at least one queue family that supports both operations for VK_QUEUE_GRAPHICS_BIT and VK_QUEUE_COMPUTE_BIT and supports presentation
                int graphics_queue_family_index = -1;
                int compute_queue_family_index = -1;
                int presentation_queue_family_index = -1;
                int queue_family_index = 0;
                for (const auto queue_family_property : queue_family_properties) {
                    if(queue_family_property.queueFlags & VK_QUEUE_GRAPHICS_BIT && graphics_queue_family_index < 0) {
                        graphics_queue_family_index = queue_family_index;
                    }

                    if(queue_family_property.queueFlags & VK_QUEUE_COMPUTE_BIT && compute_queue_family_index < 0) {
                        compute_queue_family_index = queue_family_index;
                    }

                    VkBool32 allows_presentation = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_handle, queue_family_index, surface_, &allows_presentation);
                    if(allows_presentation) {
                        presentation_queue_family_index = queue_family_index;
                    }

                    queue_family_index++;

                    if(graphics_queue_family_index >= 0 && compute_queue_family_index >= 0 && presentation_queue_family_index >= 0) {
                        // we found all queues, no need to keep searching
                        flags.set(0);
                        break;
                    }
                }

                // We only care about devices that support Swapchain extension
                for (auto extension : device_extensions_properties) {
                    if(strcmp(extension.extensionName, "VK_KHR_swapchain") == 0) {
                        device_info.extensions.push_back("VK_KHR_swapchain");
                        flags.set(1);
                        break;
                    }
                }

                // We only care about devices that support geometry shader features
                if(device_features.geometryShader)
                    flags.set(2);

                // To continue all flags must be set
                if(!flags.all())
                    break;

                VkPhysicalDeviceMemoryProperties memory_properties;
                vkGetPhysicalDeviceMemoryProperties(physical_device_handle, &memory_properties);

                device_info.physical_device = physical_device_handle;
                device_info.device_properties = device_properties;
                device_info.queue_family_index = queue_family_index; // TODO remove later when we support creating queues from diff queue families
                device_info.graphics_queue_family_index = graphics_queue_family_index;
                device_info.compute_queue_family_index = compute_queue_family_index;
                device_info.presentation_queue_family_index = presentation_queue_family_index;

                // There is a strong relation that the best card will normally have more memory, but might not always be the case
                device_info.score += memory_properties.memoryHeaps[0].size;
                device_info.score += device_properties.deviceType;
                device_info.score += device_properties.limits.maxImageDimension2D;
                device_info.score += device_properties.limits.maxImageDimension3D;
                device_info.features = device_features;
                
                suitable_device_infos.push_back(device_info);
            }
        }

        // Sort devices by score
        std::sort(suitable_device_infos.begin(), suitable_device_infos.end(), [](const PhysicalDeviceInfo& a, const PhysicalDeviceInfo& b) { 
            return a.score > b.score;
        });

        // If there is no overrides by the user lets just pick the device with highest score
        // TODO Add console parameters to allow override the gpu
        // if(!suitable_device_mapping.empty())
        //     physical_device_ = suitable_device_mapping[0].physical_device;

        // We didnt found any device that meets our standards
        if(suitable_device_infos.empty())
            return false;

        device_info_ = suitable_device_infos[0];
        
        return true;
    }

    bool RenderContext::CreateLogicalDevice() {
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
        
        VkDeviceCreateInfo device_create_info;
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pNext = nullptr;
        device_create_info.flags = 0;
        device_create_info.queueCreateInfoCount = 1;
        device_create_info.pQueueCreateInfos = &graphics_queue_info;
        device_create_info.enabledExtensionCount = static_cast<int>(device_info_.extensions.size());
        device_create_info.ppEnabledExtensionNames = device_info_.extensions.data();
        device_create_info.enabledLayerCount = 0;
        device_create_info.ppEnabledLayerNames = nullptr;
        device_create_info.pEnabledFeatures = &device_info_.features;

        const VkResult create_device_result = vkCreateDevice(device_info_.physical_device, &device_create_info, nullptr, &logical_device_);

        // Cache the graphics queue
        vkGetDeviceQueue(logical_device_, device_info_.graphics_queue_family_index, 0, &device_info_.graphics_queue);

        // Cache the present queue
        vkGetDeviceQueue(logical_device_, device_info_.presentation_queue_family_index, 0, &device_info_.present_queue);

        
        return create_device_result == VK_SUCCESS;
    }

    bool RenderContext::CreateWindowSurface() {
        if(initialization_params_.window_) {
            surface_ = static_cast<VkSurfaceKHR>(initialization_params_.window_->CreateSurface(instance_));
            if(surface_) {
                return true;
            }
        }

        return false;
    }

    bool RenderContext::CreateSwapChain() {
        uint32_t surface_formats_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device_info_.physical_device, surface_, &surface_formats_count, nullptr);

        std::vector<VkSurfaceFormatKHR> surface_formats(surface_formats_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device_info_.physical_device, surface_, &surface_formats_count, surface_formats.data());

        VkSurfaceCapabilitiesKHR surface_capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_info_.physical_device, surface_, &surface_capabilities);

        uint32_t present_mode_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device_info_.physical_device, surface_, &present_mode_count, nullptr);

        std::vector<VkPresentModeKHR> present_modes(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device_info_.physical_device, surface_, &present_mode_count, present_modes.data());

        VkExtent2D image_extent;
        image_extent.width = initialization_params_.window_->GetFramebufferSize().width;
        image_extent.height = initialization_params_.window_->GetFramebufferSize().height;

        // clamp extent
        image_extent.width = std::clamp(image_extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
        image_extent.height = std::clamp(image_extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

        // validations
        VkSurfaceFormatKHR swapchain_format = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        bool supports_desired_format = std::find(surface_formats.begin(), surface_formats.end(), swapchain_format) != surface_formats.end();
        bool supports_composite_alpha = surface_capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        bool supports_usage_flags =  surface_capabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if(!supports_desired_format || !supports_composite_alpha || !supports_usage_flags) {
            return false;
        }

        VkPresentModeKHR desired_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
        bool supports_mailbox_present_mode = std::find(present_modes.begin(), present_modes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != present_modes.end();
        if(!supports_mailbox_present_mode) {
            // if we do not support mailbox at least lets try with FIFO, spec says that his is always present but lets check anyway
            bool supports_FIFO_present_mode = std::find(present_modes.begin(), present_modes.end(), VK_PRESENT_MODE_FIFO_KHR) != present_modes.end();
            if(!supports_FIFO_present_mode) {
                return false;
            }

            desired_present_mode = VK_PRESENT_MODE_FIFO_KHR;
        }

        VkSwapchainCreateInfoKHR swapchain_create_info {};
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
        swapchain_create_info.minImageCount = std::clamp(2, (int)surface_capabilities.minImageCount, (int)surface_capabilities.maxImageCount);

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
        if(device_info_.graphics_queue_family_index != device_info_.presentation_queue_family_index) {
            const uint32_t queue_family_indices[] = { device_info_.graphics_queue_family_index, device_info_.presentation_queue_family_index };
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
            swapchain_create_info.queueFamilyIndexCount = 2;
        }
        else {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            // swapchain_create_info.pQueueFamilyIndices = nullptr;
            // swapchain_create_info.queueFamilyIndexCount = 0;
        }
        
        VkResult result = vkCreateSwapchainKHR(logical_device_, &swapchain_create_info, nullptr, &swapchain_);

        if(result == VK_SUCCESS) {
            // Create imageViews for swapchain images
            uint32_t image_count;
            vkGetSwapchainImagesKHR(logical_device_, swapchain_, &image_count, nullptr);
        
            std::vector<VkImage> images(image_count);
            vkGetSwapchainImagesKHR(logical_device_, swapchain_, &image_count, images.data());

            swapchain_images_.resize(images.size());

            VkExtent2D swapchain_image_extent_2d = GetSwapchainExtent();

            // Depth images, we can always use the same depth image for all swapchain images
            VkImageView depth_image_view;
            VkImage depth_image;
            {
                ImageCreateInfo image_create_info {};
                image_create_info.format = VK_FORMAT_D32_SFLOAT;
                image_create_info.width = swapchain_image_extent_2d.width;
                image_create_info.height = swapchain_image_extent_2d.height;
                image_create_info.mip_count = 1;
                image_create_info.usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                image_create_info.is_depth = true;

                ImageResources image_resources {};
                if(!CreateImageResource(image_create_info, image_resources)) {
                    return false;
                }

                depth_image = image_resources.image;
                depth_image_view = image_resources.image_view;
            }
            
            for (uint32_t i = 0; i < image_count; ++i)
            {
                SwapchainImage swapchain_image {};
                
                // Color images
                {
                    VkImageSubresourceRange resources_ranges;
                    resources_ranges.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    resources_ranges.layerCount = 1;
                    resources_ranges.levelCount = 1;
                    resources_ranges.baseArrayLayer = 0;
                    resources_ranges.baseMipLevel = 0;
                    
                    VkImageViewCreateInfo color_image_view_create_info {};
                    color_image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
                    color_image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
                    color_image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
                    color_image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_A; // Does swapchain images need to set alpha? can we make it VK_COMPONENT_SWIZZLE_ZERO ?
                    color_image_view_create_info.flags = 0;
                    color_image_view_create_info.format = VK_FORMAT_B8G8R8A8_SRGB;
                    color_image_view_create_info.image = images[i];
                    color_image_view_create_info.pNext = nullptr;
                    color_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    color_image_view_create_info.subresourceRange = resources_ranges;
                    color_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    
                    result = vkCreateImageView(logical_device_, &color_image_view_create_info, nullptr, &swapchain_image.color_image_view);    
                }

                swapchain_images_[i] = swapchain_image;
                swapchain_images_[i].depth_image_view = depth_image_view;
                swapchain_images_[i].depth_image = depth_image;
                
                if(result != VK_SUCCESS) {
                    // TODO log something
                    continue;
                }
            }
            
            return result == VK_SUCCESS;
        }
        
        return false;        
    }

    bool RenderContext::CreatePersistentCommandPool() {
        VkCommandPoolCreateInfo command_pool_create_info;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_create_info.pNext = nullptr;
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.queueFamilyIndex = GetGraphicsQueueIndex();
        
        const VkResult result = vkCreateCommandPool(GetLogicalDeviceHandle(), &command_pool_create_info, nullptr, &persistent_command_pool_);
        
        return result == VK_SUCCESS;   
    }

    bool RenderContext::CreateIndexedRenderingBuffer(std::vector<uint16_t> indices, std::vector<VertexData> vertex_data, VkCommandPool command_pool, IndexRenderingData& index_rendering_data)
    {
        // The index buffer merged with vertex data
        std::vector<char> data;
        data.resize(sizeof(uint16_t) * indices.size() + sizeof(VertexData) *  vertex_data.size());

        // Copy vertex data
        std::memcpy(data.data(), vertex_data.data(), sizeof(VertexData) * vertex_data.size());

        // Copy indices data
        size_t indices_offset = sizeof(VertexData) * vertex_data.size();
        std::memcpy(data.data() + indices_offset, indices.data(), sizeof(uint16_t) * indices.size());
        
        VkResult result;
        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;

        // Staging buffer
        {
            VkBufferCreateInfo buffer_create_info {};
            buffer_create_info.flags = 0;
            buffer_create_info.size = data.size();
            buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            buffer_create_info.pNext = nullptr;
            buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

            result = vkCreateBuffer(logical_device_, &buffer_create_info, nullptr, &staging_buffer);

            if(result != VK_SUCCESS) {
                return false;
            }
            
            VkMemoryRequirements memory_requirements;
            vkGetBufferMemoryRequirements(logical_device_, staging_buffer, &memory_requirements);

            int memory_type_index = FindMemoryTypeIndex(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memory_requirements);

            VkMemoryAllocateInfo memory_allocate_info {};
            memory_allocate_info.memoryTypeIndex = memory_type_index;
            memory_allocate_info.allocationSize = memory_requirements.size;
            memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memory_allocate_info.pNext = nullptr;

            result = vkAllocateMemory(logical_device_, &memory_allocate_info, nullptr, &staging_buffer_memory);

            if(result != VK_SUCCESS) {
                return false;
            }
            
            // Associate our buffer with this memory
            vkBindBufferMemory(logical_device_, staging_buffer, staging_buffer_memory, 0);

            // Copy the vertex data 
            void* buffer_data;
            vkMapMemory(logical_device_, staging_buffer_memory, 0, buffer_create_info.size, 0, &buffer_data);
            memcpy(buffer_data, data.data(), buffer_create_info.size);
            vkUnmapMemory(logical_device_, staging_buffer_memory);
        }

        // device local buffer
        {
            VkBuffer buffer;
            VkDeviceMemory buffer_memory;

            VkBufferCreateInfo buffer_create_info {};
            buffer_create_info.flags = 0;
            buffer_create_info.size = sizeof(char) * data.size();
            buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            buffer_create_info.pNext = nullptr;
            buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        
            result = vkCreateBuffer(logical_device_, &buffer_create_info, nullptr, &buffer);

            if(result != VK_SUCCESS) {
                return false;
            }
            
            VkMemoryRequirements memory_requirements;
            vkGetBufferMemoryRequirements(logical_device_, buffer, &memory_requirements);

            int memory_type_index = FindMemoryTypeIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_requirements);
            
            VkMemoryAllocateInfo memory_allocate_info {};
            memory_allocate_info.memoryTypeIndex = memory_type_index;
            memory_allocate_info.allocationSize = memory_requirements.size;
            memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memory_allocate_info.pNext = nullptr;

            result = vkAllocateMemory(logical_device_, &memory_allocate_info, nullptr, &buffer_memory);

            if(result != VK_SUCCESS) {
                return false;
            }

            // Associate our buffer with this memory
            vkBindBufferMemory(logical_device_, buffer, buffer_memory, 0);

            // Temporary command buffer to do a transfer operation for our gpu buffer
            VkCommandBufferAllocateInfo command_buffer_allocate_info {};
            command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            command_buffer_allocate_info.commandPool = persistent_command_pool_;
            command_buffer_allocate_info.commandBufferCount = 1;
        
            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(logical_device_, &command_buffer_allocate_info, &commandBuffer);
            
            VkCommandBufferBeginInfo transfer_command_buffer_begin_info {};
            transfer_command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            transfer_command_buffer_begin_info.pNext = nullptr;
            transfer_command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
            vkBeginCommandBuffer(commandBuffer, &transfer_command_buffer_begin_info);
        
            VkBufferCopy copyRegion{};
            copyRegion.size = buffer_create_info.size;
        
            vkCmdCopyBuffer(commandBuffer, staging_buffer, buffer, 1, &copyRegion);
        
            vkEndCommandBuffer(commandBuffer);
        
            VkSubmitInfo submit_info {};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &commandBuffer;

            VkFenceCreateInfo fence_create_info;
            fence_create_info.flags = 0;
            fence_create_info.pNext = nullptr;
            fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            
            VkFence fence;
            vkCreateFence(logical_device_, &fence_create_info, nullptr, &fence);

            vkQueueSubmit(GetGraphicsQueueHandle(), 1, &submit_info, fence);

            vkWaitForFences(logical_device_, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
            vkDestroyFence(logical_device_, fence, nullptr);
            // vkQueueWaitIdle(render_context_->GetPresentQueueHandle()); // TODO can we use a fence?

            vkResetCommandPool(logical_device_, persistent_command_pool_, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
            
            index_rendering_data.buffer = buffer;
            index_rendering_data.indices_offset = sizeof(VertexData) * vertex_data.size();
            index_rendering_data.indices_count = static_cast<uint32_t>(indices.size());
            index_rendering_data.vertex_data_offset = 0;
        
            // vkFreeCommandBuffers(logical_device_, command_pool, 1, &commandBuffer);
        }

        // Clean up staging buffer
        vkFreeMemory(logical_device_, staging_buffer_memory, nullptr);
        vkDestroyBuffer(logical_device_, staging_buffer, nullptr);

        return result == VK_SUCCESS;
    }

    bool RenderContext::CreateImageResource(ImageCreateInfo image_info, ImageResources& out_image_resources) {
        VkResult result;
        VkImageView image_view;
        VkImage image;

        VkImageSubresourceRange resources_ranges;
        resources_ranges.aspectMask = image_info.is_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        resources_ranges.layerCount = 1;
        resources_ranges.levelCount = image_info.mip_count;
        resources_ranges.baseArrayLayer = 0;
        resources_ranges.baseMipLevel = 0;
                    
        VkExtent3D extent;
        extent.width = image_info.width;
        extent.height = image_info.height;
        extent.depth = 1;
                
        VkImageCreateInfo image_create_info {};
        image_create_info.extent = extent;
        image_create_info.flags = 0;
        image_create_info.format = image_info.format;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.usage = image_info.usage_flags;
        image_create_info.arrayLayers = 1;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.mipLevels = image_info.mip_count;
        image_create_info.pNext = nullptr;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

        vkCreateImage(logical_device_, &image_create_info, nullptr, &image);

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(logical_device_, image, &memory_requirements);

        int memory_type_index = FindMemoryTypeIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_requirements);

        VkMemoryAllocateInfo memory_allocate_info;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.pNext = nullptr;
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.memoryTypeIndex = memory_type_index;

        VkDeviceMemory device_memory;
        vkAllocateMemory(logical_device_, &memory_allocate_info, nullptr, &device_memory);
        vkBindImageMemory(logical_device_, image, device_memory, {});
                    
        VkImageViewCreateInfo image_view_create_info {};
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
        image_view_create_info.flags = 0;
        image_view_create_info.format = image_create_info.format;
        image_view_create_info.image = image;
        image_view_create_info.pNext = nullptr;
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.subresourceRange = resources_ranges;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

        result = vkCreateImageView(logical_device_, &image_view_create_info, nullptr, &image_view);

        out_image_resources.image = image;
        out_image_resources.image_view = image_view;

        return result == VK_SUCCESS;
    }
}
