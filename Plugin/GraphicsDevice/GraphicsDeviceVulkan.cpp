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
    Error createStagingBuffer(size_t size, StagingFlag flag, VkBuffer &buffer, VkDeviceMemory &memory);

private:
    VkDevice m_device = nullptr;
};


GraphicsDevice* CreateGraphicsDeviceVulkan(void *device)
{
    return new GraphicsDeviceVulkan(device);
}


GraphicsDeviceVulkan::GraphicsDeviceVulkan(void *device)
    : m_device((VkDevice)device)
{
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

Error GraphicsDeviceVulkan::createStagingBuffer(size_t size, StagingFlag flag, VkBuffer &buffer, VkDeviceMemory &memory)
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
    if(vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    VkMemoryRequirements mem_required;
    vkGetBufferMemoryRequirements(m_device, buffer, &mem_required);

    VkMemoryAllocateInfo mem_info = {};
    mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = mem_required.size;
    mem_info.memoryTypeIndex = GetMemoryType(mem_required.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vr = vkAllocateMemory(m_device, &mem_info, nullptr, &memory);
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = vkBindBufferMemory(m_device, buffer, memory, 0);
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    return Error::OK;
}



void GraphicsDeviceVulkan::sync()
{

}


Error GraphicsDeviceVulkan::createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags)
{
    return Error::NotAvailable;
}

void GraphicsDeviceVulkan::releaseTexture2D(void *tex)
{

}

Error GraphicsDeviceVulkan::readTexture2D(void *o_buf, size_t bufsize, void *tex, int width, int height, TextureFormat format)
{
    return Error::NotAvailable;
}

Error GraphicsDeviceVulkan::writeTexture2D(void *o_tex, int width, int height, TextureFormat format, const void *buf, size_t bufsize)
{
    return Error::NotAvailable;
}


Error GraphicsDeviceVulkan::createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags)
{
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
    mem_info.memoryTypeIndex = GetMemoryType(mem_required.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory memory;
    vr = vkAllocateMemory(m_device, &mem_info, nullptr, &memory);
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    vr = vkBindBufferMemory(m_device, ret, memory, 0);
    if (vr != VK_SUCCESS) { return TranslateReturnCode(vr); }

    *dst_buf = ret;

    return Error::OK;
}

void GraphicsDeviceVulkan::releaseBuffer(void *buf)
{

}

Error GraphicsDeviceVulkan::readBuffer(void *dst, const void *src_buf, size_t read_size, BufferType type)
{
    return Error::NotAvailable;
}

Error GraphicsDeviceVulkan::writeBuffer(void *dst_buf, const void *src, size_t write_size, BufferType type)
{
    return Error::NotAvailable;
}

} // namespace gd

