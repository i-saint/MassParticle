#include "pch.h"
#include "gdInternal.h"


int GetTexelSize(GraphicsDevice::TextureFormat format)
{
    using TextureFormat = GraphicsDevice::TextureFormat;
    switch (format)
    {
    case TextureFormat::RGBAu8:  return 4;
    case TextureFormat::RGu8:    return 2;
    case TextureFormat::Ru8:     return 1;

    case TextureFormat::RGBAf16:
    case TextureFormat::RGBAi16: return 8;
    case TextureFormat::RGf16:
    case TextureFormat::RGi16:   return 4;
    case TextureFormat::Rf16:
    case TextureFormat::Ri16:    return 2;

    case TextureFormat::RGBAf32:
    case TextureFormat::RGBAi32: return 16;
    case TextureFormat::RGf32:
    case TextureFormat::RGi32:   return 8;
    case TextureFormat::Rf32:
    case TextureFormat::Ri32:    return 4;
    }
    return 0;
}


GraphicsDevice* CreateGraphicsDeviceD3D9(void *device);
GraphicsDevice* CreateGraphicsDeviceD3D11(void *device);
GraphicsDevice* CreateGraphicsDeviceD3D12(void *device);
GraphicsDevice* CreateGraphicsDeviceOpenGL();
GraphicsDevice* CreateGraphicsDeviceVulkan();

static GraphicsDevice *g_gfx_device;


GraphicsDevice* CreateGraphicsDevice(GraphicsDevice::DeviceType type, void *device_ptr)
{
    ReleaseGraphicsDevice();

    switch (type) {
#ifdef gdSupportD3D9
    case GraphicsDevice::DeviceType::D3D9:
        g_gfx_device = CreateGraphicsDeviceD3D9(device_ptr);
        break;
#endif
#ifdef gdSupportD3D11
    case GraphicsDevice::DeviceType::D3D11:
        g_gfx_device = CreateGraphicsDeviceD3D11(device_ptr);
        break;
#endif
#ifdef gdSupportD3D12
    case GraphicsDevice::DeviceType::D3D12:
        g_gfx_device = CreateGraphicsDeviceD3D12(device_ptr);
        break;
#endif
#ifdef gdSupportOpenGL
    case GraphicsDevice::DeviceType::OpenGL:
        g_gfx_device = CreateGraphicsDeviceOpenGL();
#endif
#ifdef gdSupportVulkan
    case GraphicsDevice::DeviceType::Vulkan:
        g_gfx_device = CreateGraphicsDeviceVulkan();
#endif
        break;
    }
    return g_gfx_device;
}

void ReleaseGraphicsDevice()
{
    if (g_gfx_device) {
        delete g_gfx_device;
        g_gfx_device = nullptr;
    }
}

GraphicsDevice* GetGraphicsDevice()
{
    return g_gfx_device;
}




#ifndef gdStaticLink

#include "PluginAPI/IUnityGraphics.h"
#ifdef gdSupportD3D9
    #include <d3d9.h>
    #include "PluginAPI/IUnityGraphicsD3D9.h"
#endif
#ifdef gdSupportD3D11
    #include <d3d11.h>
    #include "PluginAPI/IUnityGraphicsD3D11.h"
#endif
#ifdef gdSupportD3D12
    #include <d3d12.h>
    #include "PluginAPI/IUnityGraphicsD3D12.h"
#endif


static IUnityInterfaces* g_unity_interface;

static void UNITY_INTERFACE_API UnityOnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
    if (eventType == kUnityGfxDeviceEventInitialize) {
        auto unity_gfx = g_unity_interface->Get<IUnityGraphics>();
        auto api = unity_gfx->GetRenderer();

#ifdef gdSupportD3D9
        if (api == kUnityGfxRendererD3D9) {
            CreateGraphicsDevice(GraphicsDevice::DeviceType::D3D9, g_unity_interface->Get<IUnityGraphicsD3D9>()->GetDevice());
        }
#endif
#ifdef gdSupportD3D11
        if (api == kUnityGfxRendererD3D11) {
            CreateGraphicsDevice(GraphicsDevice::DeviceType::D3D11, g_unity_interface->Get<IUnityGraphicsD3D11>()->GetDevice());
        }
#endif
#ifdef gdSupportD3D12
        if (api == kUnityGfxRendererD3D12) {
            CreateGraphicsDevice(GraphicsDevice::DeviceType::D3D12, g_unity_interface->Get<IUnityGraphicsD3D12>()->GetDevice());
        }
#endif
#ifdef gdSupportOpenGL
        if (api == kUnityGfxRendererOpenGL ||
            api == kUnityGfxRendererOpenGLCore ||
            api == kUnityGfxRendererOpenGLES20 ||
            api == kUnityGfxRendererOpenGLES30)
        {
            CreateGraphicsDevice(GraphicsDevice::DeviceType::OpenGL, nullptr);
        }
#endif
    }
    else if (eventType == kUnityGfxDeviceEventShutdown) {
        ReleaseGraphicsDevice();
    }
}


void gdUnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    g_unity_interface = unityInterfaces;
    g_unity_interface->Get<IUnityGraphics>()->RegisterDeviceEventCallback(UnityOnGraphicsDeviceEvent);
    UnityOnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

void gdUnityPluginUnload()
{
    auto unity_gfx = g_unity_interface->Get<IUnityGraphics>();
    unity_gfx->UnregisterDeviceEventCallback(UnityOnGraphicsDeviceEvent);
}

#endif // gdStaticLink
