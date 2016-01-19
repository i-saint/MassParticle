
#include "pch.h"
#include "UnityPluginInterface.h"
#include "mpTypes.h"
#include "MassParticle.h"


extern mpRenderer *g_mpRenderer;


extern "C" void EXPORT_API UnitySetGraphicsDevice(void* device, int deviceType, int eventType)
{
    if (eventType == kGfxDeviceEventInitialize) {
    #if SUPPORT_D3D9
        if (deviceType == kGfxRendererD3D9)
        {
            g_mpRenderer = mpCreateRendererD3D9(device);
        }
    #endif // SUPPORT_D3D9
    #if SUPPORT_D3D11
        if (deviceType == kGfxRendererD3D11)
        {
            g_mpRenderer = mpCreateRendererD3D11(device);
        }
    #endif // SUPPORT_D3D11
    #if SUPPORT_OPENGL
        if (deviceType == kGfxRendererOpenGL)
        {
            g_mpRenderer = mpCreateRendererOpenGL(device);
        }
    #endif // SUPPORT_OPENGL
    }

    if (eventType == kGfxDeviceEventShutdown) {
        delete g_mpRenderer;
        g_mpRenderer = nullptr;
    }
}

extern "C" void EXPORT_API UnityRenderEvent(int eventID)
{
}
