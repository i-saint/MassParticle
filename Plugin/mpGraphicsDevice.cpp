
#include "pch.h"
#include "mpFoundation.h"
#include "MassParticle.h"


// Graphics device identifiers in Unity
enum GfxDeviceRenderer
{
    kGfxRendererOpenGL = 0,          // OpenGL
    kGfxRendererD3D9,                // Direct3D 9
    kGfxRendererD3D11,               // Direct3D 11
    kGfxRendererGCM,                 // Sony PlayStation 3 GCM
    kGfxRendererNull,                // "null" device (used in batch mode)
    kGfxRendererHollywood,           // Nintendo Wii
    kGfxRendererXenon,               // Xbox 360
    kGfxRendererOpenGLES,            // OpenGL ES 1.1
    kGfxRendererOpenGLES20Mobile,    // OpenGL ES 2.0 mobile variant
    kGfxRendererMolehill,            // Flash 11 Stage3D
    kGfxRendererOpenGLES20Desktop,   // OpenGL ES 2.0 desktop variant (i.e. NaCl)
    kGfxRendererCount
};

// Event types for UnitySetGraphicsDevice
enum GfxDeviceEventType {
    kGfxDeviceEventInitialize = 0,
    kGfxDeviceEventShutdown,
    kGfxDeviceEventBeforeReset,
    kGfxDeviceEventAfterReset,
};


namespace {
    mpRenderer *g_mpRenderer;
}

mpRenderer* mpGetRenderer() { return g_mpRenderer; }

mpRenderer* mpCreateRendererD3D9(void *dev);
mpRenderer* mpCreateRendererD3D11(void *dev);
mpRenderer* mpCreateRendererOpenGL(void *dev);

void mpUnitySetGraphicsDevice(void* device, int deviceType, int eventType)
{
    if (eventType == kGfxDeviceEventInitialize) {
#ifdef mpSupportD3D9
        if (deviceType == kGfxRendererD3D9)
        {
            g_mpRenderer = mpCreateRendererD3D9(device);
        }
#endif
#if mpSupportD3D11
        if (deviceType == kGfxRendererD3D11)
        {
            g_mpRenderer = mpCreateRendererD3D11(device);
        }
#endif
#if mpSupportOpenGL
        if (deviceType == kGfxRendererOpenGL)
        {
            g_mpRenderer = mpCreateRendererOpenGL(device);
        }
#endif
    }

    if (eventType == kGfxDeviceEventShutdown) {
        delete g_mpRenderer;
        g_mpRenderer = nullptr;
    }
}

mpCLinkage mpAPI void UnitySetGraphicsDevice(void* device, int deviceType, int eventType)
{
    mpUnitySetGraphicsDevice(device, deviceType, eventType);
}

mpCLinkage mpAPI void UnityRenderEvent(int eventID)
{
}
