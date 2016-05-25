#include "pch.h"
#include "gdInternal.h"
#include <d3d12.h>

class GraphicsDeviceD3D12 : public GraphicsDevice
{
public:
    GraphicsDeviceD3D12(void *device);
    ~GraphicsDeviceD3D12();
    void* getDevicePtr() override;
    GraphicsDeviceType getDeviceType() override;
    void sync() override;

    bool readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, PixelFormat format) override;
    bool writeTexture(void *o_tex, int width, int height, PixelFormat format, const void *buf, size_t bufsize) override;

    bool readBuffer(void *dst, const void *src_buf, size_t srcsize) override;
    bool writeBuffer(void *dst_buf, const void *src, size_t srcsize) override;

private:
};


GraphicsDevice* CreateGraphicsDeviceD3D12(void *device)
{
    return new GraphicsDeviceD3D12(device);
}


GraphicsDeviceD3D12::GraphicsDeviceD3D12(void *device)
{
}

GraphicsDeviceD3D12::~GraphicsDeviceD3D12()
{
}

void* GraphicsDeviceD3D12::getDevicePtr()
{
    return nullptr;
}

GraphicsDeviceType GraphicsDeviceD3D12::getDeviceType()
{
    return GraphicsDeviceType::D3D12;
}

void GraphicsDeviceD3D12::sync()
{

}


bool GraphicsDeviceD3D12::readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, PixelFormat format)
{
    return false;
}

bool GraphicsDeviceD3D12::writeTexture(void *o_tex, int width, int height, PixelFormat format, const void *buf, size_t bufsize)
{
    return false;
}


bool GraphicsDeviceD3D12::readBuffer(void *dst, const void *src_buf, size_t srcsize)
{
    return false;
}

bool GraphicsDeviceD3D12::writeBuffer(void *dst_buf, const void *src, size_t srcsize)
{
    return false;
}
