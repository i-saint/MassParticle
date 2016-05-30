#include "pch.h"
#include "gdInternal.h"
#include <d3d12.h>

class GraphicsDeviceD3D12 : public GraphicsDevice
{
public:
    GraphicsDeviceD3D12(void *device);
    ~GraphicsDeviceD3D12();
    void* getDevicePtr() override;
    DeviceType getDeviceType() override;
    void sync() override;

    Error readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, TextureFormat format) override;
    Error writeTexture(void *o_tex, int width, int height, TextureFormat format, const void *buf, size_t bufsize) override;

    Error readBuffer(void *dst, const void *src_buf, size_t srcsize) override;
    Error writeBuffer(void *dst_buf, const void *src, size_t srcsize) override;

private:
    ID3D12Device *m_device;
};


GraphicsDevice* CreateGraphicsDeviceD3D12(void *device)
{
    return new GraphicsDeviceD3D12(device);
}


GraphicsDeviceD3D12::GraphicsDeviceD3D12(void *device)
    : m_device((ID3D12Device*)device)
{
}

GraphicsDeviceD3D12::~GraphicsDeviceD3D12()
{
}

void* GraphicsDeviceD3D12::getDevicePtr()
{
    return nullptr;
}

GraphicsDevice::DeviceType GraphicsDeviceD3D12::getDeviceType()
{
    return DeviceType::D3D12;
}

void GraphicsDeviceD3D12::sync()
{

}


GraphicsDevice::Error GraphicsDeviceD3D12::readTexture(void *dst, size_t dstsize, void *src_tex_, int width, int height, TextureFormat format)
{
    auto *src_tex = (ID3D12Resource*)src_tex_;
    return Error::NotImplemented;
}

GraphicsDevice::Error GraphicsDeviceD3D12::writeTexture(void *dst_tex_, int width, int height, TextureFormat format, const void *src, size_t srcsize)
{
    auto *dst_tex = (ID3D12Resource*)dst_tex_;
    return Error::NotImplemented;
}


GraphicsDevice::Error GraphicsDeviceD3D12::readBuffer(void *dst, const void *src_buf, size_t srcsize)
{
    return Error::NotImplemented;
}

GraphicsDevice::Error GraphicsDeviceD3D12::writeBuffer(void *dst_buf, const void *src, size_t srcsize)
{
    return Error::NotImplemented;
}
