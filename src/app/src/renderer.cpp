#include "renderer.h"

#define VULKAN_DESIRED_VERSION VK_MAKE_VERSION(1, 3, 201)
#define APP_VERSION VK_MAKE_VERSION(0, 0, 1)

#define ENABLE_INSTANCE_LAYERS // TODO for now hard-coded but it should cmake to define this
#ifdef ENABLE_INSTANCE_LAYERS
    #define INSTANCE_DEBUG_LAYERS {"VK_LAYER_KHRONOS_validation"}
#else
    #define INSTANCE_DEBUG_LAYERS {}
#endif

namespace app::renderer {
    bool Renderer::Initialize(const InitializationParams& initialization_params) {
        initialization_params_ = initialization_params;

        bool b_initialized = false;
        b_initialized = CreateVulkanInstance();
        b_initialized = PickSuitableDevice();

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

        struct DeviceMapping {
            VkPhysicalDevice physical_device;
            VkPhysicalDeviceProperties device_properties;
            uint64_t score;
        };
        
        std::vector<DeviceMapping> suitable_device_mapping;
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

                // flag that signal if we have graphics and compute queues
                std::bitset<2> queue_flags;

                // We need to find at least one queue family that supports both operations
                for (const auto queue_family_property : queue_family_properties) {
                    if(queue_family_property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                        queue_flags.set(0);

                    if(queue_family_property.queueFlags & VK_QUEUE_COMPUTE_BIT)
                        queue_flags.set(1);
                }

                // skip if both flags are not set
                if(!queue_flags.all())
                    break;
                
                VkPhysicalDeviceMemoryProperties memory_properties;
                vkGetPhysicalDeviceMemoryProperties(physical_device_handle, &memory_properties);

                DeviceMapping device_mapping {};
                device_mapping.physical_device = physical_device_handle;
                device_mapping.device_properties = device_properties;

                // There is a strong relation that the best card will normally have more memory, but might not always be the case
                device_mapping.score += memory_properties.memoryHeaps[0].size;
                device_mapping.score += device_properties.deviceType;
                
                suitable_device_mapping.push_back(device_mapping);
            }
        }

        // Sort devices by score
        std::sort(suitable_device_mapping.begin(), suitable_device_mapping.end(), [](const DeviceMapping& a, const DeviceMapping& b) { 
            return a.score > b.score;
        });

        // If there is no overrides by the user lets just pick the device with highest score
        // TODO Add console parameters to allow override the gpu
        if(!suitable_device_mapping.empty())
            physical_device_ = suitable_device_mapping[0].physical_device;
        
        return !suitable_device_mapping.empty();
    }

}
