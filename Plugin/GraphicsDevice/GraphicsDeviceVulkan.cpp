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
    GraphicsDeviceVulkan();
    ~GraphicsDeviceVulkan() override;
    void release() override;

    void* getDevicePtr() override;
    DeviceType getDeviceType() override;
    void sync() override;

    Error readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, TextureFormat format) override;
    Error writeTexture(void *o_tex, int width, int height, TextureFormat format, const void *buf, size_t bufsize) override;

    Error readBuffer(void *dst, const void *src_buf, size_t read_size, BufferType type) override;
    Error writeBuffer(void *dst_buf, const void *src, size_t write_size, BufferType type) override;

private:
};


GraphicsDevice* CreateGraphicsDeviceVulkan()
{
    return new GraphicsDeviceVulkan();
}


GraphicsDeviceVulkan::GraphicsDeviceVulkan()
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


Error GraphicsDeviceVulkan::readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, TextureFormat format)
{
    return Error::NotAvailable;
}

Error GraphicsDeviceVulkan::writeTexture(void *o_tex, int width, int height, TextureFormat format, const void *buf, size_t bufsize)
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

