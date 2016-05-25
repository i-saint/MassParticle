#include "pch.h"
#include "gdInternal.h"

class GraphicsDeviceVulkan : public GraphicsDevice
{
public:
    GraphicsDeviceVulkan();
    ~GraphicsDeviceVulkan();
    void* getDevicePtr() override;
    GraphicsDeviceType getDeviceType() override;
    void sync() override;

    bool readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, PixelFormat format) override;
    bool writeTexture(void *o_tex, int width, int height, PixelFormat format, const void *buf, size_t bufsize) override;

    bool readBuffer(void *dst, const void *src_buf, size_t srcsize) override;
    bool writeBuffer(void *dst_buf, const void *src, size_t srcsize) override;

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

GraphicsDeviceType GraphicsDeviceVulkan::getDeviceType()
{
    return GraphicsDeviceType::Vulkan;
}

void GraphicsDeviceVulkan::sync()
{

}


bool GraphicsDeviceVulkan::readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, PixelFormat format)
{
    return false;
}

bool GraphicsDeviceVulkan::writeTexture(void *o_tex, int width, int height, PixelFormat format, const void *buf, size_t bufsize)
{
    return false;
}


bool GraphicsDeviceVulkan::readBuffer(void *dst, const void *src_buf, size_t srcsize)
{
    return false;
}

bool GraphicsDeviceVulkan::writeBuffer(void *dst_buf, const void *src, size_t srcsize)
{
    return false;
}
