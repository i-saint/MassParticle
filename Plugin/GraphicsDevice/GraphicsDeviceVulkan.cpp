#include "pch.h"
#include "gdInternal.h"

#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32
#include <vulkan/vulkan.h>
#pragma comment(lib, "vulkan-1.lib")

namespace gd {

class GraphicsDeviceVulkan : public GraphicsDevice
{
public:
    GraphicsDeviceVulkan(void *device);
    ~GraphicsDeviceVulkan() override;
    void release() override;

    void* getDevicePtr() override;
    DeviceType getDeviceType() override;
    void sync() override;

    Error createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags) override;
    void releaseTexture2D(void *tex) override;
    Error readTexture2D(void *o_buf, size_t bufsize, void *tex, int width, int height, TextureFormat format) override;
    Error writeTexture2D(void *o_tex, int width, int height, TextureFormat format, const void *buf, size_t bufsize) override;

    Error createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags) override;
    void releaseBuffer(void *buf) override;
    Error readBuffer(void *dst, const void *src_buf, size_t read_size, BufferType type) override;
    Error writeBuffer(void *dst_buf, const void *src, size_t write_size, BufferType type) override;

private:
    enum class StagingFlag {
        Upload,
        Readback,
    };
    VkResult createStagingBuffer(size_t size, StagingFlag flag, VkBuffer &buffer, VkDeviceMemory &memory);

    // Body: [](VkBuffer staging_buffer, VkDeviceMemory staging_memory) -> void
    template<class Body> VkResult staging(VkBuffer target, StagingFlag flag, const Body& body);

    // Body: [](void *mapped_memory) -> void
    template<class Body> VkResult map(VkDeviceMemory device_memory, const Body& body);

    // Body: [](VkCommandBuffer *clist) -> void
    template<class Body> VkResult executeCommands(const Body& body);

private:
    VkDevice m_device = nullptr;
    VkQueue m_cqueue = nullptr;
    VkCommandPool m_cpool = nullptr;
};


GraphicsDevice* CreateGraphicsDeviceVulkan(void *device)
{
    return new GraphicsDeviceVulkan(device);
}


GraphicsDeviceVulkan::GraphicsDeviceVulkan(void *device)
    : m_device((VkDevice)device)
{
    uint32_t graphicsQueueIndex = 0;
    vkGetDeviceQueue(m_device, graphicsQueueIndex, 0, &m_cqueue);

    VkCommandPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = 0;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCreateCommandPool(m_device, &info, nullptr, &m_cpool);
}

GraphicsDeviceVulkan::~GraphicsDeviceVulkan()
{
}

void GraphicsDeviceVulkan::release()
{
    delete this;
}

void* GraphicsDeviceVulkan::getDevicePtr()
{
    return nullptr;
}

DeviceType GraphicsDeviceVulkan::getDeviceType()
{
    return DeviceType::Vulkan;
}

static Error TranslateReturnCode(VkResult vr)
{
    switch (vr) {
    case VK_SUCCESS: return Error::OK;
    case VK_ERROR_OUT_OF_HOST_MEMORY: return Error::OutOfMemory;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return Error::OutOfMemory;
    }
    return Error::Unknown;
}

static uint32_t GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < 32; i++) {
        if ((typeBits & 1) == 1) {
            return i;
        }
        typeBits >>= 1;
    }

    return 0;
}

VkResult GraphicsDeviceVulkan::createStagingBuffer(size_t size, StagingFlag flag, VkBuffer &buffer, VkDeviceMemory &memory)
{
    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (flag == StagingFlag::Upload) {
        info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (flag == StagingFlag::Readback) {
        info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    auto vr = vkCreateBuffer(m_device, &info, nullptr, &buffer);
    if(vr != VK_SUCCESS) { return vr; }

    VkMemoryRequirements mem_required;
    vkGetBufferMemoryRequirements(m_device, buffer, &mem_required);

    VkMemoryAllocateInfo mem_info = {};
    mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = mem_required.size;
    mem_info.memoryTypeIndex = GetMemoryType(mem_required.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vr = vkAllocateMemory(m_device, &mem_info, nullptr, &memory);
    if (vr != VK_SUCCESS) { return vr; }

    vr = vkBindBufferMemory(m_device, buffer, memory, 0);
    if (vr != VK_SUCCESS) { return vr; }

    return VK_SUCCESS;
}

template<class Body>
VkResult GraphicsDeviceVulkan::staging(VkBuffer target, StagingFlag flag, const Body& body)
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
VkResult GraphicsDeviceVulkan::map(VkDeviceMemory device_memory, const Body& body)
{
    void *mapped_memory = nullptr;
    auto vr = vkMapMemory(m_device, device_memory, 0, VK_WHOLE_SIZE, 0, &mapped_memory);
    if (vr != VK_SUCCESS) { return vr; }

    body(mapped_memory);

    vkUnmapMemory(m_device, device_memory);
    return VK_SUCCESS;
}

template<class Body>
VkResult GraphicsDeviceVulkan::executeCommands(const Body& body)
{
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = m_cpool;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer clist;
    auto vr = vkAllocateCommandBuffers(m_device, &alloc_info, &clist);
    if (vr != VK_SUCCESS) { return vr; }

    body(clist);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &clist;

    vr = vkQueueSubmit(m_cqueue, 1, &submit_info, VK_NULL_HANDLE);
    if (vr != VK_SUCCESS) { return vr; }

    vr = vkQueueWaitIdle(m_cqueue);
    if (vr != VK_SUCCESS) { return vr; }

    vkFreeCommandBuffers(m_device, m_cpool, 1, &clist);
    return VK_SUCCESS;
}



void GraphicsDeviceVulkan::sync()
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

Error GraphicsDeviceVulkan::createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags)
{
    VkImageCreateInfo info = {};
    info.sType          = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType      = VK_IMAGE_TYPE_2D;
    info.format         = TranslateFormat(format);
    info.mipLevels      = 1;
    info.arrayLayers    = 1;
    info.samples        = VK_SAMPLE_COUNT_1_BIT;
    info.tiling         = VK_IMAGE_TILING_OPTIMAL;
    info.usage          = VK_IMAGE_USAGE_SAMPLED_BIT;
    info.sharingMode    = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout  = VK_IMAGE_LAYOUT_PREINITIALIZED;
    info.extent         = { (uint32_t)width, (uint32_t)height, 1 };
    info.usage          = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    info.flags          = 0;

    VkImage ret = nullptr;
    auto vr = vkCreateImage(m_device, &info, nullptr, &ret);
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    VkMemoryRequirements mem_required;
    vkGetImageMemoryRequirements(m_device, ret, &mem_required);

    VkMemoryAllocateInfo mem_info = {};
    mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = mem_required.size;
    mem_info.memoryTypeIndex = GetMemoryType(mem_required.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory memory;
    vr = vkAllocateMemory(m_device, &mem_info, nullptr, &memory);
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = vkBindImageMemory(m_device, ret, memory, 0);
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    //// todo: upload if needed
    //VkImageSubresourceRange subresourceRange = {};
    //subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //subresourceRange.baseMipLevel = 0;
    //subresourceRange.levelCount = 1;
    //subresourceRange.layerCount = 1;

    *dst_tex = ret;

    return Error::OK;
}

void GraphicsDeviceVulkan::releaseTexture2D(void *tex_)
{
    if (!tex_) { return; }

    auto tex = (VkImage)tex_;
    vkDestroyImage(m_device, tex, nullptr);
}

Error GraphicsDeviceVulkan::readTexture2D(void *dst, size_t read_size, void *src_tex_, int width, int height, TextureFormat format)
{
    if (read_size == 0) { return Error::OK; }
    if (!dst || !src_tex_) { return Error::InvalidParameter; }

    auto *src_tex = (VkImage*)src_tex_;

    return Error::NotAvailable;
}

Error GraphicsDeviceVulkan::writeTexture2D(void *dst_tex_, int width, int height, TextureFormat format, const void *src, size_t write_size)
{
    if (write_size == 0) { return Error::OK; }
    if (!dst_tex_ || !src) { return Error::InvalidParameter; }

    auto *dst_tex = (VkImage*)dst_tex_;

    return Error::NotAvailable;
}


Error GraphicsDeviceVulkan::createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags)
{
    if (!dst_buf) { return Error::InvalidParameter; }

    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    switch (type) {
    case BufferType::Index:
        info.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case BufferType::Vertex:
        info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    case BufferType::Constant:
        info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        break;
    case BufferType::Compute:
        info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        break;
    }

    VkBuffer ret;
    auto vr = vkCreateBuffer(m_device, &info, nullptr, &ret);
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    VkMemoryRequirements mem_required;
    vkGetBufferMemoryRequirements(m_device, ret, &mem_required);

    VkMemoryAllocateInfo mem_info = {};
    mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = mem_required.size;
    mem_info.memoryTypeIndex = GetMemoryType(mem_required.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkDeviceMemory memory;
    vr = vkAllocateMemory(m_device, &mem_info, nullptr, &memory);
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = vkBindBufferMemory(m_device, ret, memory, 0);
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    *dst_buf = ret;

    return Error::OK;
}

void GraphicsDeviceVulkan::releaseBuffer(void *buf_)
{
    if (!buf_) { return; }

    auto buf = (VkBuffer)buf_;
    vkDestroyBuffer(m_device, buf, nullptr);
}

Error GraphicsDeviceVulkan::readBuffer(void *dst, const void *src_buf, size_t read_size, BufferType type)
{
    if (read_size == 0) { return Error::OK; }
    if (!dst || !src_buf) { return Error::InvalidParameter; }

    auto buf = (VkBuffer)src_buf;
    auto res = Error::OK;

    VkResult vr;
    vr = staging(buf, StagingFlag::Readback, [&](VkBuffer staging_buffer, VkDeviceMemory staging_memory) {
        vr = executeCommands([&](VkCommandBuffer clist) {
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

Error GraphicsDeviceVulkan::writeBuffer(void *dst_buf, const void *src, size_t write_size, BufferType type)
{
    if (write_size == 0) { return Error::OK; }
    if (!dst_buf || !src) { return Error::InvalidParameter; }

    auto buf = (VkBuffer)dst_buf;
    auto res = Error::OK;

    VkResult vr;
    vr = staging(buf, StagingFlag::Upload, [&](VkBuffer staging_buffer, VkDeviceMemory staging_memory) {
        vr = map(staging_memory, [&](void *mapped_memory) {
            memcpy(mapped_memory, src, write_size);
        });
        if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); return; }

        vr = executeCommands([&](VkCommandBuffer clist) {
            VkBufferCopy region = {0, 0, write_size };
            vkCmdCopyBuffer(clist, staging_buffer, buf, 1, &region);
        });
        if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); return; }
    });
    if (vr != VK_SUCCESS) { res = TranslateReturnCode(vr); }

    return res;
}

} // namespace gd

