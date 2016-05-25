#pragma once

enum class PixelFormat
{
    Unknown = 0,

    ChannelMask = 0xF,
    TypeMask    = 0xF << 4,
    Type_f16    = 0x1 << 4,
    Type_f32    = 0x2 << 4,
    Type_u8     = 0x3 << 4,
    Type_i16    = 0x4 << 4,
    Type_i32    = 0x5 << 4,

    Rf16    = Type_f16 | 1,
    RGf16   = Type_f16 | 2,
    RGBf16  = Type_f16 | 3,
    RGBAf16 = Type_f16 | 4,
    Rf32    = Type_f32 | 1,
    RGf32   = Type_f32 | 2,
    RGBf32  = Type_f32 | 3,
    RGBAf32 = Type_f32 | 4,
    Ru8     = Type_u8 | 1,
    RGu8    = Type_u8 | 2,
    RGBu8   = Type_u8 | 3,
    RGBAu8  = Type_u8 | 4,
    Ri16    = Type_i16 | 1,
    RGi16   = Type_i16 | 2,
    RGBi16  = Type_i16 | 3,
    RGBAi16 = Type_i16 | 4,
    Ri32    = Type_i32 | 1,
    RGi32   = Type_i32 | 2,
    RGBi32  = Type_i32 | 3,
    RGBAi32 = Type_i32 | 4,
    I420    = 0x10 << 4,
};

enum class GraphicsDeviceType
{
    Unknown,
    D3D9,
    D3D11,
    D3D12,
    OpenGL,
    Vulkan,
    PS4,
};


class GraphicsDevice
{
public:
    virtual ~GraphicsDevice() {}
    virtual void* getDevicePtr() = 0;
    virtual GraphicsDeviceType getDeviceType() = 0;
    virtual void sync() = 0;
    virtual bool readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, PixelFormat format) = 0;
    virtual bool writeTexture(void *o_tex, int width, int height, PixelFormat format, const void *buf, size_t bufsize) = 0;
};


int             GetPixelSize(PixelFormat format);

GraphicsDevice* CreateGraphicsDevice(GraphicsDeviceType type, void *device_ptr);
void            ReleaseGraphicsDevice();
GraphicsDevice* GetGraphicsDevice();
