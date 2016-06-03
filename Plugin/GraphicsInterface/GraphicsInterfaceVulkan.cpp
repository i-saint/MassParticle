#include "pch.h"
#include "giInternal.h"

#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32
#include <vulkan/vulkan.h>
#pragma comment(lib, "vulkan-1.lib")

namespace gd {

class GraphicsInterfaceVulkan : public GraphicsInterface
{
public:
    GraphicsInterfaceVulkan(void *device);
    ~GraphicsInterfaceVulkan() override;
    void release() override;

    void* getDevicePtr() override;
    DeviceType getDeviceType() override;
    void sync() override;

    Result createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags) override;
    void releaseTexture2D(void *tex) override;
    Result readTexture2D(void *o_buf, size_t bufsize, void *tex, int width, int height, TextureFormat format) override;
    Result writeTexture2D(void *o_tex, int width, int height, TextureFormat format, const void *buf, size_t bufsize) override;

    Result createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags) override;
    void releaseBuffer(void *buf) override;
    Result readBuffer(void *dst, const void *src_buf, size_t read_size, BufferType type) override;
    Result writeBuffer(void *dst_buf, const void *src, size_t write_size, BufferType type) override;

private:
    uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties);

    enum class StagingFlag {
        Upload,
        Readback,
    };
    VkResult createStagingBuffer(size_t size, StagingFlag flag, VkBuffer &buffer, VkDeviceMemory &memory);

    // Body: [](VkBuffer staging_buffer, VkDeviceMemory staging_memory) -> void
    template<class Body> VkResult staging(VkBuffer target, StagingFlag flag, const Body& body);
    template<class Body> VkResult staging(VkImage target, StagingFlag flag, const Body& body);

    // Body: [](void *mapped_memory) -> void
    template<class Body> VkResult map(VkDeviceMemory device_memory, const Body& body);

    // Body: [](VkCommandBuffer *clist) -> void
    template<class Body> VkResult submitCommands(const Body& body);

private:
    VkPhysicalDevice m_physical_device = nullptr;
    VkDevice m_device = nullptr;
    VkQueue m_cqueue = nullptr;
    VkCommandPool m_cpool = nullptr;

    VkPhysicalDeviceMemoryProperties m_memory_properties;
};


GraphicsInterface* CreateGraphicsInterfaceVulkan(void *device)
{
    return new GraphicsInterfaceVulkan(device);
}

struct VulkanParams
{
    VkPhysicalDevice physical_device;
    VkDevice device;
};


GraphicsInterfaceVulkan::GraphicsInterfaceVulkan(void *device)
{
    auto& vkparams = *(VulkanParams*)device;
    m_physical_device = vkparams.physical_device;
    m_device = vkparams.device;

    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &m_memory_properties);

    uint32_t graphicsQueueIndex = 0;
    vkGetDeviceQueue(m_device, graphicsQueueIndex, 0, &m_cqueue);

    VkCommandPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = 0;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device, &info, nullptr, &m_cpool);
}

GraphicsInterfaceVulkan::~GraphicsInterfaceVulkan()
{
}

void GraphicsInterfaceVulkan::release()
{
    delete this;
}

void* GraphicsInterfaceVulkan::getDevicePtr()
{
    return nullptr;
}

DeviceType GraphicsInterfaceVulkan::getDeviceType()
{
    return DeviceType::Vulkan;
}

static Result TranslateReturnCode(VkResult vr)
{
    switch (vr) {
    case VK_SUCCESS: return Result::OK;
    case VK_ERROR_OUT_OF_HOST_MEMORY: return Result::OutOfMemory;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return Result::OutOfMemory;
    }
    return Result::Unknown;
}

uint32_t GraphicsInterfaceVulkan::getMemoryType(uint32_t type_bits, VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < 32; i++) {
        if ((type_bits & 1) == 1) {
            if ((m_memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        type_bits >>= 1;
    }

    return 0;
}

VkResult GraphicsInterfaceVulkan::createStagingBuffer(size_t size, StagingFlag flag, VkBuffer &buffer, VkDeviceMemory &memory)
{
    VkResult vr = VK_SUCCESS;

    VkBufferCreateInfo buf_info = {};
    VkMemoryRequirements mem_required = {};
    VkMemoryAllocateInfo mem_info = {};

    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.size = size;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (flag == StagingFlag::Upload) {
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (flag == StagingFlag::Readback) {
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    vr = vkCreateBuffer(m_device, &buf_info, nullptr, &buffer);
    if (vr != VK_SUCCESS) { goto on_error; }

    vkGetBufferMemoryRequirements(m_device, buffer, &mem_required);

    mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = mem_required.size;
    mem_info.memoryTypeIndex = getMemoryType(mem_required.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vr = vkAllocateMemory(m_device, &mem_info, nullptr, &memory);
    if (vr != VK_SUCCESS) { goto on_error; }

    vr = vkBindBufferMemory(m_device, buffer, memory, 0);
    if (vr != VK_SUCCESS) { goto on_error; }

    return VK_SUCCESS;

on_error:
    if (memory) {
        vkFreeMemory(m_device, memory, nullptr);
        memory = nullptr;
    }
    if (buffer) {
        vkDestroyBuffer(m_device, buffer, nullptr);
        buffer = nullptr;
    }
    return vr;
}

template<class Body>
VkResult GraphicsInterfaceVulkan::staging(VkBuffer target, StagingFlag flag, const Body& body)
{
    VkMemoryRequirements mem_required;
    vkGetBufferMemoryRequirements(m_device, target, &mem_required);

    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;
    auto vr = createStagingBuffer(mem_required.size, flag, staging_buffer, staging_memory);
    if (vr != VK_SUCCESS) { return vr; }

    body(staging_buffer, staging_memory);

    vkFreeMemory(m_device, staging_memory, nullptr);
    vkDestroyBuffer(m_device, staging_buffer, nullptr);
    return VK_SUCCESS;
}

template<class Body>
VkResult GraphicsInterfaceVulkan::staging(VkImage target, StagingFlag flag, const Body& body)
{
    VkMemoryRequirements mem_required;
    vkGetImageMemoryRequirements(m_device, target, &mem_required);

    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;
    auto vr = createStagingBuffer(mem_required.size, flag, staging_buffer, staging_memory);
    if (vr != VK_SUCCESS) { return vr; }

    body(staging_buffer, staging_memory);

    vkFreeMemory(m_device, staging_memory, nullptr);
    vkDestroyBuffer(m_device, staging_buffer, nullptr);
    return VK_SUCCESS;
}

template<class Body>
VkResult GraphicsInterfaceVulkan::map(VkDeviceMemory device_memory, const Body& body)
{
    void *mapped_memory = nullptr;
    auto vr = vkMapMemory(m_device, device_memory, 0, VK_WHOLE_SIZE, 0, &mapped_memory);
    if (vr != VK_SUCCESS) { return vr; }

    body(mapped_memory);

    vkUnmapMemory(m_device, device_memory);

    VkMappedMemoryRange mapped_range = { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE , nullptr, device_memory , 0, VK_WHOLE_SIZE };
    vr = vkFlushMappedMemoryRanges(m_device, 1, &mapped_range);
    if (vr != VK_SUCCESS) { return vr; }

    return VK_SUCCESS;
}

template<class Body>
VkResult GraphicsInterfaceVulkan::submitCommands(const Body& body)
{
    VkResult vr;
    VkCommandBuffer clist = nullptr;
    VkCommandBufferAllocateInfo alloc_info = {};
    VkCommandBufferBeginInfo begin_info = {};
    VkSubmitInfo submit_info = {};

    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = m_cpool;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;
    vr = vkAllocateCommandBuffers(m_device, &alloc_info, &clist);
    if (vr != VK_SUCCESS) { goto on_error; }

    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vr = vkBeginCommandBuffer(clist, &begin_info);
    if (vr != VK_SUCCESS) { goto on_error; }

    body(clist);

    vr = vkEndCommandBuffer(clist);
    if (vr != VK_SUCCESS) { goto on_error; }

    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &clist;
    vr = vkQueueSubmit(m_cqueue, 1, &submit_info, VK_NULL_HANDLE);
    if (vr != VK_SUCCESS) { goto on_error; }

    vr = vkQueueWaitIdle(m_cqueue);
    if (vr != VK_SUCCESS) { goto on_error; }

on_error:
    if (m_cpool) {
        vkFreeCommandBuffers(m_device, m_cpool, 1, &clist);
    }
    return vr;
}



void GraphicsInterfaceVulkan::sync()
{
    auto vr = vkQueueWaitIdle(m_cqueue);
}


static VkFormat TranslateFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::RGBAu8:  return VK_FORMAT_R8G8B8A8_UNORM;

    case TextureFormat::RGBAf16: return VK_FORMAT_R16G16B16A16_SFLOAT;
    case TextureFormat::RGf16:   return VK_FORMAT_R16G16_SFLOAT;
    case TextureFormat::Rf16:    return VK_FORMAT_R16_SFLOAT;

    case TextureFormat::RGBAf32: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case TextureFormat::RGf32:   return VK_FORMAT_R32G32_SFLOAT;
    case TextureFormat::Rf32:    return VK_FORMAT_R32_SFLOAT;

    case TextureFormat::RGBAi32: return VK_FORMAT_R32G32B32A32_SINT;
    case TextureFormat::RGi32:   return VK_FORMAT_R32G32_SINT;
    case TextureFormat::Ri32:    return VK_FORMAT_R32_SINT;
    }
    return VK_FORMAT_UNDEFINED;
}

Result GraphicsInterfaceVulkan::createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags)
{
    VkResult vr = VK_SUCCESS;
    VkImage ret = nullptr;
    VkDeviceMemory memory = nullptr;

    VkImageCreateInfo image_info = {};
    VkMemoryRequirements mem_required = {};
    VkMemoryAllocateInfo mem_info = {};

    image_info.sType          = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType      = VK_IMAGE_TYPE_2D;
    image_info.format         = TranslateFormat(format);
    image_info.mipLevels      = 1;
    image_info.arrayLayers    = 1;
    image_info.samples        = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling         = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage          = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode    = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout  = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_info.extent         = { (uint32_t)width, (uint32_t)height, 1 };
    image_info.usage          = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.flags          = 0;

    vr = vkCreateImage(m_device, &image_info, nullptr, &ret);
    if (vr != VK_SUCCESS) { goto on_error; }

    vkGetImageMemoryRequirements(m_device, ret, &mem_required);

    mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = mem_required.size;
    mem_info.memoryTypeIndex = getMemoryType(mem_required.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vr = vkAllocateMemory(m_device, &mem_info, nullptr, &memory);
    if (vr != VK_SUCCESS) { goto on_error; }

    vr = vkBindImageMemory(m_device, ret, memory, 0);
    if (vr != VK_SUCCESS) { goto on_error; }

    //// todo: upload if needed
    //VkImageSubresourceRange subresourceRange = {};
    //subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //subresourceRange.baseMipLevel = 0;
    //subresourceRange.levelCount = 1;
    //subresourceRange.layerCount = 1;

    *dst_tex = ret;
    return Result::OK;

on_error:
    if (memory) { vkFreeMemory(m_device, memory, nullptr); }
    if (ret) { vkDestroyImage(m_device, ret, nullptr); }
    return TranslateReturnCode(vr);
}

void GraphicsInterfaceVulkan::releaseTexture2D(void *tex_)
{
    if (!tex_) { return; }

    auto tex = (VkImage)tex_;
    vkDestroyImage(m_device, tex, nullptr);
}

Result GraphicsInterfaceVulkan::readTexture2D(void *dst, size_t read_size, void *src_tex_, int width, int height, TextureFormat format)
{
    if (read_size == 0) { return Result::OK; }
    if (!dst || !src_tex_) { return Result::InvalidParameter; }

    auto src_tex = (VkImage)src_tex_;
    auto res = Result::OK;

    VkResult vr;
    vr = staging(src_tex, StagingFlag::Readback, [&](VkBuffer staging_buffer, VkDeviceMemory staging_memory) {
        vr = submitCommands([&](VkCommandBuffer clist) {
            VkBufferImageCopy region = {};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageExtent.width = width;
            region.imageExtent.height = height;
            region.imageExtent.depth = 1;
            vkCmdCopyImageToBuffer(clist, src_tex, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging_buffer, 1, &region);
        });
        if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); return; }

        vr = map(staging_memory, [&](void *mapped_memory) {
            memcpy(dst, mapped_memory, read_size);
        });
        if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); return; }
    });
    if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); }

    return res;
}

Result GraphicsInterfaceVulkan::writeTexture2D(void *dst_tex_, int width, int height, TextureFormat format, const void *src, size_t write_size)
{
    if (write_size == 0) { return Result::OK; }
    if (!dst_tex_ || !src) { return Result::InvalidParameter; }

    auto dst_tex = (VkImage)dst_tex_;
    auto res = Result::OK;

    VkResult vr;
    vr = staging(dst_tex, StagingFlag::Upload, [&](VkBuffer staging_buffer, VkDeviceMemory staging_memory) {
        vr = map(staging_memory, [&](void *mapped_memory) {
            memcpy(mapped_memory, src, write_size);
        });
        if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); return; }

        vr = submitCommands([&](VkCommandBuffer clist) {
            VkBufferImageCopy region = {};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageExtent.width = width;
            region.imageExtent.height = height;
            region.imageExtent.depth = 1;
            vkCmdCopyBufferToImage(clist, staging_buffer, dst_tex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        });
        if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); return; }
    });
    if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); }

    return res;
}


Result GraphicsInterfaceVulkan::createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags)
{
    if (!dst_buf) { return Result::InvalidParameter; }

    VkResult vr = VK_SUCCESS;
    VkBuffer ret = nullptr;
    VkDeviceMemory memory = nullptr;

    VkBufferCreateInfo buf_info = {};
    VkMemoryRequirements mem_required = {};
    VkMemoryAllocateInfo mem_info = {};

    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.size = size;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    switch (type) {
    case BufferType::Index:
        buf_info.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case BufferType::Vertex:
        buf_info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case BufferType::Constant:
        buf_info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;
    case BufferType::Compute:
        buf_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        break;
    }

    vr = vkCreateBuffer(m_device, &buf_info, nullptr, &ret);
    if (vr != VK_SUCCESS) { goto on_error; }

    vkGetBufferMemoryRequirements(m_device, ret, &mem_required);

    mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = mem_required.size;
    mem_info.memoryTypeIndex = getMemoryType(mem_required.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vr = vkAllocateMemory(m_device, &mem_info, nullptr, &memory);
    if (vr != VK_SUCCESS) { goto on_error; }

    vr = vkBindBufferMemory(m_device, ret, memory, 0);
    if (vr != VK_SUCCESS) { goto on_error; }

    *dst_buf = ret;
    return Result::OK;

on_error:
    if (memory) { vkFreeMemory(m_device, memory, nullptr); }
    if (ret) { vkDestroyBuffer(m_device, ret, nullptr); }
    return TranslateReturnCode(vr);
}

void GraphicsInterfaceVulkan::releaseBuffer(void *buf_)
{
    if (!buf_) { return; }

    auto buf = (VkBuffer)buf_;
    vkDestroyBuffer(m_device, buf, nullptr);
}

Result GraphicsInterfaceVulkan::readBuffer(void *dst, const void *src_buf, size_t read_size, BufferType type)
{
    if (read_size == 0) { return Result::OK; }
    if (!dst || !src_buf) { return Result::InvalidParameter; }

    auto buf = (VkBuffer)src_buf;
    auto res = Result::OK;

    VkResult vr;
    vr = staging(buf, StagingFlag::Readback, [&](VkBuffer staging_buffer, VkDeviceMemory staging_memory) {
        vr = submitCommands([&](VkCommandBuffer clist) {
            VkBufferCopy region = { 0, 0, read_size };
            vkCmdCopyBuffer(clist, buf, staging_buffer, 1, &region);
        });
        if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); return; }

        vr = map(staging_memory, [&](void *mapped_memory) {
            memcpy(dst, mapped_memory, read_size);
        });
        if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); return; }
    });
    if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); }

    return res;
}

Result GraphicsInterfaceVulkan::writeBuffer(void *dst_buf, const void *src, size_t write_size, BufferType type)
{
    if (write_size == 0) { return Result::OK; }
    if (!dst_buf || !src) { return Result::InvalidParameter; }

    auto buf = (VkBuffer)dst_buf;
    auto res = Result::OK;

    VkResult vr;
    vr = staging(buf, StagingFlag::Upload, [&](VkBuffer staging_buffer, VkDeviceMemory staging_memory) {
        vr = map(staging_memory, [&](void *mapped_memory) {
            memcpy(mapped_memory, src, write_size);
        });
        if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); return; }

        vr = submitCommands([&](VkCommandBuffer clist) {
            VkBufferCopy region = {0, 0, write_size };
            vkCmdCopyBuffer(clist, staging_buffer, buf, 1, &region);
        });
        if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); return; }
    });
    if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); }

    return res;
}

} // namespace gd

