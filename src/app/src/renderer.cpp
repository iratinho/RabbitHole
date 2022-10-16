#include "renderer.h"
#include "window.h"

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
    bool Renderer::Initialize(const InitializationParams& initialization_params) {
        initialization_params_ = initialization_params;

        bool b_initialized = false;
        b_initialized = CreateVulkanInstance();
        b_initialized = CreateWindowSurface();
        b_initialized = PickSuitableDevice();
        b_initialized = CreateLogicalDevice();
        b_initialized = CreateSwapChain();

        return b_initialized;
    }

    bool Renderer::CreateVulkanInstance() {
        if(vkEnumerateInstanceVersion(&loader_version_) != VK_SUCCESS) {
            std::cerr << "Unable to get the vulkan loader version." << std::endl;
            return false;
        }

        std::cout << std::printf("Current vulkan loader version: %i.%i.%i", VK_VERSION_MAJOR(loader_version_), VK_VERSION_MINOR(loader_version_), VK_VERSION_PATCH(loader_version_)) << std::endl;
        std::cout << std::printf("Desired vulkan loader version: %i.%i.%i", VK_VERSION_MAJOR(VULKAN_DESIRED_VERSION), VK_VERSION_MINOR(VULKAN_DESIRED_VERSION), VK_VERSION_PATCH(VULKAN_DESIRED_VERSION)) << std::endl;;

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
        for (auto& extension : extension_properties) {
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

    bool Renderer::PickSuitableDevice() {
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
                        device_info_.extensions.push_back(extension.extensionName);
                        flags.set(1);
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
                device_info.extensions.push_back("VK_KHR_swapchain");
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

    bool Renderer::CreateLogicalDevice() {
        // TODO for now we assume the all queue families are the same, this might not be the case and if not we need to create a queue for them
        constexpr float queue_priority = 1.0f;
        VkDeviceQueueCreateInfo device_queue_create_info;
        device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info.pNext = nullptr;
        device_queue_create_info.flags = 0;
        device_queue_create_info.queueFamilyIndex = device_info_.queue_family_index;
        device_queue_create_info.queueCount = 1;
        device_queue_create_info.pQueuePriorities = &queue_priority;
        
        VkDeviceCreateInfo device_create_info;
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pNext = nullptr;
        device_create_info.flags = 0;
        device_create_info.queueCreateInfoCount = 1;
        device_create_info.pQueueCreateInfos = &device_queue_create_info;
        device_create_info.enabledExtensionCount = static_cast<int>(device_info_.extensions.size());
        device_create_info.ppEnabledExtensionNames = device_info_.extensions.data();
        device_create_info.enabledLayerCount = 0;
        device_create_info.ppEnabledLayerNames = nullptr;
        device_create_info.pEnabledFeatures = &device_info_.features;

        const VkResult create_device_result = vkCreateDevice(device_info_.physical_device, &device_create_info, nullptr, &logical_device_);
        
        return create_device_result == VK_SUCCESS;
    }

    bool Renderer::CreateWindowSurface() {
        if(initialization_params_.window_) {
            surface_ = static_cast<VkSurfaceKHR>(initialization_params_.window_->CreateSurface(instance_));
            if(surface_) {
                return true;
            }
        }

        return false;
    }

    bool Renderer::CreateSwapChain() {
        VkExtent2D image_extent;
        image_extent.width = initialization_params_.window_->GetFramebufferSize().width; // Clamp with min and max value returned by vkGetPhysicalDeviceSurfaceCapabilitiesKHR
        image_extent.height = initialization_params_.window_->GetFramebufferSize().height; // Clamp with min and max value returned by vkGetPhysicalDeviceSurfaceCapabilitiesKHR
        
        VkSwapchainCreateInfoKHR swapchain_create_info;
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.flags = 0;
        swapchain_create_info.surface = surface_;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // validate for support with vkGetPhysicalDeviceSurfaceCapabilitiesKHR
        swapchain_create_info.imageExtent = image_extent;
        swapchain_create_info.imageFormat = VK_FORMAT_R8G8B8A8_SRGB; // validate or format support with vkGetPhysicalDeviceSurfaceFormatsKHR
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // validate that imageUsage is supported with vkGetPhysicalDeviceSurfaceCapabilitiesKHR
        swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
        swapchain_create_info.pNext = nullptr;
        swapchain_create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR; // validate for mailbox support and fallback to fifo if not present
        swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; // validate for support with vkGetPhysicalDeviceSurfaceCapabilitiesKHR
        swapchain_create_info.minImageCount = 2; // For now double buffering, if we exceed the 16.6ms to render a new image consider using a triple buffer. Also clamp the imageCount with maxImageCount in vkGetPhysicalDeviceSurfaceCapabilitiesKHR
        
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
            swapchain_create_info.pQueueFamilyIndices = nullptr;
            swapchain_create_info.queueFamilyIndexCount = 0;
        }
        
        const VkResult result = vkCreateSwapchainKHR(logical_device_, &swapchain_create_info, nullptr, &swapchain_);

        // Create imageViews for swapchain images
        uint32_t image_count;
        vkGetSwapchainImagesKHR(logical_device_, swapchain_, &image_count, nullptr);
        swapchain_image_views_.resize(image_count);

        for (uint32_t i = 0; i < image_count; ++i)
        {
            VkImage image;
            vkGetSwapchainImagesKHR(logical_device_, swapchain_, &image_count, &image);

            VkImageSubresourceRange resources_ranges;
            resources_ranges.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            resources_ranges.layerCount = 1;
            resources_ranges.levelCount = 1;
            resources_ranges.baseArrayLayer = 0;
            resources_ranges.baseMipLevel = 0;
            
            VkImageView image_view;
            VkImageViewCreateInfo image_view_create_info {};
            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_A; // Does swapchain images need to set alpha? can we make it VK_COMPONENT_SWIZZLE_ZERO ?
            image_view_create_info.flags = 0;
            image_view_create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
            image_view_create_info.image = image;
            image_view_create_info.pNext = nullptr;
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.subresourceRange = resources_ranges;
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            
            vkCreateImageView(logical_device_, &image_view_create_info, nullptr, &image_view);
            swapchain_image_views_[i] = image_view;
        }
        
        return result == VK_SUCCESS;
    }

}
