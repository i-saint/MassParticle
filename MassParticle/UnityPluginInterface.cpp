
// --------------------------------------------------------------------------
// Include headers for the graphics APIs we support

#include "UnityPluginInterface.h"
#include "mpTypes.h"
#include "MassParticle.h"


extern mpWorld g_mpWorld;


extern "C" void EXPORT_API UnitySetGraphicsDevice(void* device, int deviceType, int eventType)
{
#if SUPPORT_D3D9
    if (deviceType == kGfxRendererD3D9)
    {
          // todo
    }
#endif // SUPPORT_D3D9
#if SUPPORT_D3D11
    if (eventType == kGfxDeviceEventInitialize) {
        if (deviceType == kGfxRendererD3D11)
        {
            g_mpWorld.m_renderer = mpCreateRendererD3D11(device, g_mpWorld);
        }
    }
#endif // SUPPORT_D3D11
#if SUPPORT_OPENGL
    if (deviceType == kGfxRendererOpenGL)
    {
        // todo
    }
#endif // SUPPORT_OPENGL

    if (eventType == kGfxDeviceEventShutdown) {
        delete g_mpWorld.m_renderer;
        g_mpWorld.m_renderer = nullptr;
    }
}

extern "C" void EXPORT_API UnityRenderEvent(int eventID)
{
    if (g_mpWorld.m_renderer) {
        g_mpWorld.m_renderer->render();
    }
}
