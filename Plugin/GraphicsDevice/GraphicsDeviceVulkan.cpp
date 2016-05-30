#include "pch.h"
#include "gdInternal.h"

class GraphicsDeviceVulkan : public GraphicsDevice
{
public:
    GraphicsDeviceVulkan();
    ~GraphicsDeviceVulkan();
    void* getDevicePtr() override;
    DeviceType getDeviceType() override;
    void sync() override;

    Error readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, TextureFormat format) override;
    Error writeTexture(void *o_tex, int width, int height, TextureFormat format, const void *buf, size_t bufsize) override;

    Error readBuffer(void *dst, const void *src_buf, size_t srcsize) override;
    Error writeBuffer(void *dst_buf, const void *src, size_t srcsize) override;

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

void* GraphicsDeviceVulkan::getDevicePtr()
{
    return nullptr;
}

GraphicsDevice::DeviceType GraphicsDeviceVulkan::getDeviceType()
{
    return DeviceType::Vulkan;
}

void GraphicsDeviceVulkan::sync()
{

}


GraphicsDevice::Error GraphicsDeviceVulkan::readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, TextureFormat format)
{
    return Error::NotImplemented;
}

GraphicsDevice::Error GraphicsDeviceVulkan::writeTexture(void *o_tex, int width, int height, TextureFormat format, const void *buf, size_t bufsize)
{
    return Error::NotImplemented;
}


GraphicsDevice::Error GraphicsDeviceVulkan::readBuffer(void *dst, const void *src_buf, size_t srcsize)
{
    return Error::NotImplemented;
}

GraphicsDevice::Error GraphicsDeviceVulkan::writeBuffer(void *dst_buf, const void *src, size_t srcsize)
{
    return Error::NotImplemented;
}
