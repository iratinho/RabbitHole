#include "render_context.h"
#include "Texture.h"

Texture::Texture(RenderContext* render_context, TextureParams params)
    : params_(params)
    , render_context_(render_context)
    , image_(nullptr) {}

Texture::Texture(RenderContext* render_context, TextureParams params, VkImage image)
    : params_(params)
    , render_context_(render_context)
    , image_(image) {}

Texture::Texture(Texture&& texture) {  // NOLINT(cppcorkeguidelines-pro-type-member-init)
    image_ = texture.image_;
    texture.image_ = nullptr;

    params_ = texture.params_;
    texture.params_ = {};

    render_context_ = texture.render_context_;
    texture.render_context_ = nullptr;
}

bool Texture::Initialize() {
    // Do not initialize if resource is already valid, call free resource first
    if(IsValidResource())
       return true; 

    if(render_context_ == nullptr)
        return false;
    
    const bool is_depth_renderTarget = params_.format == VK_FORMAT_D32_SFLOAT;

    VkExtent3D extent;
    extent.width = params_.width;
    extent.height = params_.height;
    extent.depth = 1;

    VkImageCreateInfo image_create_info{};
    image_create_info.extent = extent;
    image_create_info.flags = 0;
    image_create_info.format = params_.format;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.usage = is_depth_renderTarget ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.arrayLayers = 1;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.mipLevels = 1;
    image_create_info.pNext = nullptr;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    VkResult result = vkCreateImage(render_context_->GetLogicalDeviceHandle(), &image_create_info, nullptr, &image_);

    if (result == VK_SUCCESS) {
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(render_context_->GetLogicalDeviceHandle(), image_, &memory_requirements);

        const int memory_type_index = render_context_->FindMemoryTypeIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_requirements);

        VkMemoryAllocateInfo memory_allocate_info;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.pNext = nullptr;
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.memoryTypeIndex = memory_type_index;

        VkDeviceMemory device_memory;
        result = vkAllocateMemory(render_context_->GetLogicalDeviceHandle(), &memory_allocate_info, nullptr, &device_memory);

        if (result == VK_SUCCESS) {
            result = vkBindImageMemory(render_context_->GetLogicalDeviceHandle(), image_, device_memory, {});
        }
    }

    return result == VK_SUCCESS;
}

void Texture::FreeResource() {
    if(render_context_ && image_ != VK_NULL_HANDLE) {
        render_context_->DestroyImage(image_);
        image_ = nullptr;
    }
}
