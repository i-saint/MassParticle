#include "pch.h"
#include "gdInternal.h"
#include "gdUnityPluginImpl.h"

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

namespace gd {

static IUnityInterfaces* g_unity_interface;

static void UNITY_INTERFACE_API UnityOnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
    if (eventType == kUnityGfxDeviceEventInitialize) {
        auto unity_gfx = g_unity_interface->Get<IUnityGraphics>();
        auto api = unity_gfx->GetRenderer();

#ifdef gdSupportD3D9
        if (api == kUnityGfxRendererD3D9) {
            CreateGraphicsDevice(DeviceType::D3D9, g_unity_interface->Get<IUnityGraphicsD3D9>()->GetDevice());
        }
#endif
#ifdef gdSupportD3D11
        if (api == kUnityGfxRendererD3D11) {
            CreateGraphicsDevice(DeviceType::D3D11, g_unity_interface->Get<IUnityGraphicsD3D11>()->GetDevice());
        }
#endif
#ifdef gdSupportD3D12
        if (api == kUnityGfxRendererD3D12) {
            CreateGraphicsDevice(DeviceType::D3D12, g_unity_interface->Get<IUnityGraphicsD3D12>()->GetDevice());
        }
#endif
#ifdef gdSupportOpenGL
        if (api == kUnityGfxRendererOpenGL ||
            api == kUnityGfxRendererOpenGLCore ||
            api == kUnityGfxRendererOpenGLES20 ||
            api == kUnityGfxRendererOpenGLES30)
        {
            CreateGraphicsDevice(DeviceType::OpenGL, nullptr);
        }
#endif
#ifdef gdSupportVulkan
        if (false) // todo
        {
            CreateGraphicsDevice(DeviceType::Vulkan, nullptr);
        }
#endif
    }
    else if (eventType == kUnityGfxDeviceEventShutdown) {
        ReleaseGraphicsDevice();
    }
}


void UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    g_unity_interface = unityInterfaces;
    g_unity_interface->Get<IUnityGraphics>()->RegisterDeviceEventCallback(UnityOnGraphicsDeviceEvent);
    UnityOnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

void UnityPluginUnload()
{
    auto unity_gfx = g_unity_interface->Get<IUnityGraphics>();
    unity_gfx->UnregisterDeviceEventCallback(UnityOnGraphicsDeviceEvent);
}

} // namespace gd
