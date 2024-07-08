#include "Renderer/render_context.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/VulkanTranslator.hpp"

Texture::Texture(RenderContext* render_context, TextureParams params)
    : _textureWidth(params._width)
    , _textureHeight(params._height)
    , _params(params)
    , _renderContext(render_context)
    , _imageLayout(ImageLayout::LAYOUT_UNDEFINED)
{
}

Texture::~Texture() {};

bool Texture::Initialize() {
    // Do not initialize if resource is already valid, call free resource first
    if(IsValidResource())
       return true; 

    if(_renderContext == nullptr)
        return false;

    // TODO check for valid params like sizes
    // const bool is_depth_renderTarget = params_.format == VK_FORMAT_D32_SFLOAT;

    VkExtent3D extent;
    extent.width = _params._width;
    extent.height = _params._height;
    extent.depth = 1;

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.extent = extent;
    imageCreateInfo.flags = 0;
    imageCreateInfo.format = TranslateFormat(_params.format);
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.usage = TranslateTextureUsageFlags(_params.flags);
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.initialLayout = TranslateImageLayout(_imageLayout);
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    VkResult result = VkFunc::vkCreateImage(_renderContext->GetLogicalDeviceHandle(), &imageCreateInfo, nullptr, &_image);

    if (result == VK_SUCCESS) {
        VkFunc::vkGetImageMemoryRequirements(_renderContext->GetLogicalDeviceHandle(), _image, &_memoryRequirements);

        const int memory_type_index = _renderContext->FindMemoryTypeIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _memoryRequirements);

        VkMemoryAllocateInfo memoryAllocationInfo;
        memoryAllocationInfo.allocationSize = _memoryRequirements.size;
        memoryAllocationInfo.pNext = nullptr;
        memoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocationInfo.memoryTypeIndex = memory_type_index;

        result = VkFunc::vkAllocateMemory(_renderContext->GetLogicalDeviceHandle(), &memoryAllocationInfo, nullptr, &_deviceMemory);

        if (result == VK_SUCCESS) {
            result = VkFunc::vkBindImageMemory(_renderContext->GetLogicalDeviceHandle(), _image, _deviceMemory, {});
        }
    }

    CreateView();
    
    return result == VK_SUCCESS;
}

void Texture::FreeResource() {
    // VkImages are controlled by the implementation when used in the swapchain, we should not call DestroyImage
    if(_params.flags & Tex_PRESENTATION)
        _image = VK_NULL_HANDLE;
    
    if(_renderContext && _image != VK_NULL_HANDLE) {
        _renderContext->DestroyImage(_image);
        _image = nullptr;
    }
    
    _renderContext->DestroyImageView(_imageView);
    _imageView = VK_NULL_HANDLE;
}

void Texture::WritePixelData(void* data, size_t size) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    
    // Copy pixel data to a vulkan CPU backed memory
    {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = VkFunc::vkCreateBuffer(_renderContext->GetLogicalDeviceHandle(), &bufferCreateInfo, nullptr,
                                                 &stagingBuffer);

        if(result != VK_SUCCESS)
            return;

        VkMemoryRequirements memory_requirements;
        VkFunc::vkGetBufferMemoryRequirements(_renderContext->GetLogicalDeviceHandle(), stagingBuffer, &memory_requirements);

        int memoryTypeIndex = _renderContext->FindMemoryTypeIndex(
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memory_requirements);

        VkMemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
        memoryAllocateInfo.allocationSize = memory_requirements.size;
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.pNext = nullptr;

        result = VkFunc::vkAllocateMemory(_renderContext->GetLogicalDeviceHandle(), &memoryAllocateInfo, nullptr, &stagingMemory);

        if(result != VK_SUCCESS)
            return;

        // Associate our buffer with this memory
        VkFunc::vkBindBufferMemory(_renderContext->GetLogicalDeviceHandle(), stagingBuffer, stagingMemory, 0);

        // Copy the pixel data to staging buffer 
        void* stagedPixelData;
        VkFunc::vkMapMemory(_renderContext->GetLogicalDeviceHandle(), stagingMemory, 0, bufferCreateInfo.size, 0, &stagedPixelData);
        memcpy(stagedPixelData,  data, bufferCreateInfo.size);
        VkFunc::vkUnmapMemory(_renderContext->GetLogicalDeviceHandle(), stagingMemory);
    }

    VkBuffer localBuffer;
    VkDeviceMemory LocalMemory;
    
    // Copy data from staging buffer to GPU backed memory
    {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = VkFunc::vkCreateBuffer(_renderContext->GetLogicalDeviceHandle(), &bufferCreateInfo, nullptr,
                                                 &localBuffer);

        if(result != VK_SUCCESS)
            return;

        VkMemoryRequirements memory_requirements;
        VkFunc::vkGetBufferMemoryRequirements(_renderContext->GetLogicalDeviceHandle(), localBuffer, &memory_requirements);

        const int memoryTypeIndex = _renderContext->FindMemoryTypeIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_requirements);

        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.memoryTypeIndex = memoryTypeIndex;
        memory_allocate_info.allocationSize = memory_requirements.size;
        memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.pNext = nullptr;

        result = VkFunc::vkAllocateMemory(_renderContext->GetLogicalDeviceHandle(), &memory_allocate_info, nullptr, &LocalMemory);

        if (result != VK_SUCCESS)
            return;

        // Associate our buffer with this memory
        VkFunc::vkBindBufferMemory(_renderContext->GetLogicalDeviceHandle(), localBuffer, LocalMemory, 0);

        std::vector<VkCommandBuffer> commandBuffers;
        
        // Commands to copy staging buffer to local buffer
        {
            VkCommandBuffer stagingToGPUBufferCommand;

            // Temporary command buffer to do a transfer operation for our gpu buffer
            VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferAllocateInfo.commandPool = _renderContext-> GetPersistentCommandPool();
            commandBufferAllocateInfo.commandBufferCount = 1;

            VkFunc::vkAllocateCommandBuffers(_renderContext->GetLogicalDeviceHandle(), &commandBufferAllocateInfo, &stagingToGPUBufferCommand);

            VkCommandBufferBeginInfo commandBufferBeginInfo{};
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            commandBufferBeginInfo.pNext = nullptr;
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            VkFunc::vkBeginCommandBuffer(stagingToGPUBufferCommand, &commandBufferBeginInfo);
        
            VkBufferCopy copyRegion{};
            copyRegion.size = bufferCreateInfo.size;
        
            VkFunc::vkCmdCopyBuffer(stagingToGPUBufferCommand, stagingBuffer, localBuffer, 1, &copyRegion);

            VkFunc::vkEndCommandBuffer(stagingToGPUBufferCommand);

            commandBuffers.push_back(stagingToGPUBufferCommand);
        }
        
        // Command to make layout transition and copy image data
        {
            VkCommandBuffer copyToImageCommandBuffer;

            // Temporary command buffer to do a transfer operation for our gpu buffer
            VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferAllocateInfo.commandPool = _renderContext-> GetPersistentCommandPool();
            commandBufferAllocateInfo.commandBufferCount = 1;

            VkFunc::vkAllocateCommandBuffers(_renderContext->GetLogicalDeviceHandle(), &commandBufferAllocateInfo, &copyToImageCommandBuffer);

            VkCommandBufferBeginInfo commandBufferBeginInfo{};
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            commandBufferBeginInfo.pNext = nullptr;
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            VkFunc::vkBeginCommandBuffer(copyToImageCommandBuffer, &commandBufferBeginInfo);

            // Barrier to transition image layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL so we can copy transfer the data
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = _image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            
            VkFunc::vkCmdPipelineBarrier(copyToImageCommandBuffer,
                                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

            // Copy from localBuffer to VkImage 
            VkBufferImageCopy region = {};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {GetWidth(), GetHeight(), 1};
            VkFunc::vkCmdCopyBufferToImage(copyToImageCommandBuffer, localBuffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


            // Transition this image to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL since it will be sampled from the shader
            VkImageMemoryBarrier imageMemoryBarrier {};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            imageMemoryBarrier.image = _image;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            imageMemoryBarrier.subresourceRange.levelCount = 1;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = 1;

            VkFunc::vkCmdPipelineBarrier(
                copyToImageCommandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier
            );
            
            VkFunc::vkEndCommandBuffer(copyToImageCommandBuffer);

            commandBuffers.push_back(copyToImageCommandBuffer);
        }
        
        VkFenceCreateInfo fence_create_info;
        fence_create_info.flags = 0;
        fence_create_info.pNext = nullptr;
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        VkFence fence;
        VkFunc::vkCreateFence(_renderContext->GetLogicalDeviceHandle(), &fence_create_info, nullptr, &fence);
        
        VkSubmitInfo submitInfo {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = commandBuffers.size();
        submitInfo.pCommandBuffers = commandBuffers.data();

        VkFunc::vkQueueSubmit(_renderContext->GetGraphicsQueueHandle(), 1, &submitInfo, fence);

        // Ensure that GPU finishes executing
        VkFunc::vkWaitForFences(_renderContext->GetLogicalDeviceHandle(), 1, &fence, VK_TRUE, UINT64_MAX);
        VkFunc::vkDestroyFence(_renderContext->GetLogicalDeviceHandle(), fence, nullptr);
        VkFunc::vkResetCommandPool(_renderContext->GetLogicalDeviceHandle(), _renderContext-> GetPersistentCommandPool(), VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    }

    // Clean buffers
    VkFunc::vkDestroyBuffer(_renderContext->GetLogicalDeviceHandle(), stagingBuffer, nullptr);
    VkFunc::vkDestroyBuffer(_renderContext->GetLogicalDeviceHandle(), localBuffer, nullptr);

    // Clean buffers memory
    VkFunc::vkFreeMemory(_renderContext->GetLogicalDeviceHandle(), stagingMemory, nullptr);
    VkFunc::vkFreeMemory(_renderContext->GetLogicalDeviceHandle(), LocalMemory, nullptr);
}

bool Texture::CreateView() {
    if (!GetResource()) {
        std::cerr << "[Error]: Trying to create render target resource with invalid texture resource." << std::endl;
        return false;
    }
    
    VkImageSubresourceRange resourcesRange;
    resourcesRange.aspectMask = TranslateFormat(_params.format)  == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    resourcesRange.layerCount = 1;
    resourcesRange.levelCount = 1;
    resourcesRange.baseArrayLayer = 0;
    resourcesRange.baseMipLevel = 0;
    
    VkImageViewCreateInfo imageViewCreateInfo {};
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.format = TranslateFormat(_params.format);
    imageViewCreateInfo.image =  static_cast<const VkImage>(GetResource());
    imageViewCreateInfo.pNext = nullptr;
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.subresourceRange = resourcesRange;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    
    const VkResult result = VkFunc::vkCreateImageView(_renderContext->GetLogicalDeviceHandle(), &imageViewCreateInfo, nullptr, &_imageView);

    if(result != VK_SUCCESS) {
        std::cerr << "[Error]: Failed to create image view for render target." << std::endl;
    }

    return result == VK_SUCCESS;
}

void* Texture::GetView() const {
    return _imageView;
}

unsigned Texture::GetWidth() const {
    return _textureWidth;
}

unsigned Texture::GetHeight() const {
    return _textureHeight;
}

void* Texture::GetResource() const {
    return (void*)_image;
}

bool Texture::IsValidResource() const {
    return _image != VK_NULL_HANDLE && _imageView != VK_NULL_HANDLE ;
}

void Texture::SetResource(void* resource) {
    _image = static_cast<VkImage>(resource);
}

Format Texture::GetFormat() const {
    return _params.format;
}

ImageLayout Texture::GetImageLayout() {
    return _imageLayout;
}

void Texture::SetImageLayout(ImageLayout layout) {
    _imageLayout = layout;
}

VkImageUsageFlags Texture::TranslateTextureUsageFlags(const TextureFlags& usageFlags)
{
    VkImageUsageFlags flags = 0;

    if (usageFlags & Tex_COLOR_ATTACHMENT) {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    if (usageFlags & Tex_DEPTH_ATTACHMENT) {
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    if (usageFlags & Tex_TRANSFER_DEST_OP) {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (usageFlags & Tex_SAMPLED_OP) {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    return flags;
}

