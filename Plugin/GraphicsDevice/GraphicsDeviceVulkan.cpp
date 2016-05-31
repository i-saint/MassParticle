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

    Error createTexture(void **dst_tex, int width, int height, TextureFormat format, const void *data, CPUAccessFlag flags) override;
    Error readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, TextureFormat format) override;
    Error writeTexture(void *o_tex, int width, int height, TextureFormat format, const void *buf, size_t bufsize) override;

    Error createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, CPUAccessFlag flags) override;
    Error readBuffer(void *dst, const void *src_buf, size_t read_size, BufferType type) override;
    Error writeBuffer(void *dst_buf, const void *src, size_t write_size, BufferType type) override;

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

void GraphicsDeviceVulkan::sync()
{

}


Error GraphicsDeviceVulkan::createTexture(void **dst_tex, int width, int height, TextureFormat format, const void *data, CPUAccessFlag flags)
{
    return Error::NotAvailable;
}

Error GraphicsDeviceVulkan::readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, TextureFormat format)
{
    return Error::NotAvailable;
}

Error GraphicsDeviceVulkan::writeTexture(void *o_tex, int width, int height, TextureFormat format, const void *buf, size_t bufsize)
{
    return Error::NotAvailable;
}


Error GraphicsDeviceVulkan::createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, CPUAccessFlag flags)
{
    return Error::NotAvailable;
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

