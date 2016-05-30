#pragma once

class GraphicsDevice
{
public:
    enum class DeviceType
    {
        Unknown,
        D3D9,
        D3D11,
        D3D12,
        OpenGL,
        Vulkan,
        PS4,
    };

    enum class Error
    {
        OK,
        Unknown,
        NotImplemented,
        InvalidParameter,
        OutOfMemory,
        InaccesibleFromCPU,
    };

    enum class TextureFormat
    {
        Unknown = 0,

        ElementsMask  = 0xFF,
        Elements_R    = 0x01,
        Elements_RG   = 0x02,
        Elements_RGBA = 0x04,

        TypeMask = 0xFF << 8,
        Type_f16 = 0x1 << 8,
        Type_f32 = 0x2 << 8,
        Type_u8  = 0x3 << 8,
        Type_i16 = 0x4 << 8,
        Type_i32 = 0x5 << 8,

        Rf16    = Type_f16 | Elements_R,
        RGf16   = Type_f16 | Elements_RG,
        RGBAf16 = Type_f16 | Elements_RGBA,
        Rf32    = Type_f32 | Elements_R,
        RGf32   = Type_f32 | Elements_RG,
        RGBAf32 = Type_f32 | Elements_RGBA,
        Ru8     = Type_u8  | Elements_R,
        RGu8    = Type_u8  | Elements_RG,
        RGBAu8  = Type_u8  | Elements_RGBA,
        Ri16    = Type_i16 | Elements_R,
        RGi16   = Type_i16 | Elements_RG,
        RGBAi16 = Type_i16 | Elements_RGBA,
        Ri32    = Type_i32 | Elements_R,
        RGi32   = Type_i32 | Elements_RG,
        RGBAi32 = Type_i32 | Elements_RGBA,

        I420    = 0x10 << 8,
    };


public:
    virtual ~GraphicsDevice() {}
    virtual void* getDevicePtr() = 0;
    virtual DeviceType getDeviceType() = 0;

    virtual void sync() = 0;

    virtual Error readTexture(void *dst, size_t dstsize, void *src_tex, int width, int height, TextureFormat format) = 0;
    virtual Error writeTexture(void *dst_tex, int width, int height, TextureFormat format, const void *src, size_t srcsize) = 0;

    virtual Error readBuffer(void *dst, const void *src_buf, size_t read_size) = 0;
    virtual Error readIndexBuffer(void *dst, const void *src_buf, size_t read_size) { return readBuffer(dst, src_buf, read_size); }
    virtual Error readVertexBuffer(void *dst, const void *src_buf, size_t read_size) { return readBuffer(dst, src_buf, read_size); }
    virtual Error writeBuffer(void *dst_buf, const void *src, size_t write_size) = 0;
    virtual Error writeIndexBuffer(void *dst_buf, const void *src, size_t write_size) { return writeBuffer(dst_buf, src, write_size); }
    virtual Error writeVertexBuffer(void *dst_buf, const void *src, size_t write_size) { return writeBuffer(dst_buf, src, write_size); }
};


int             GetTexelSize(GraphicsDevice::TextureFormat format);

GraphicsDevice* CreateGraphicsDevice(GraphicsDevice::DeviceType type, void *device_ptr);
void            ReleaseGraphicsDevice();
GraphicsDevice* GetGraphicsDevice();
