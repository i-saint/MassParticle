#include "pch.h"
#include "mpInternal.h"
#include "GraphicsDevice.h"
#include "PluginAPI/IUnityGraphics.h"

#ifndef gdStaticLink

void gdUnityPluginLoad(IUnityInterfaces* unityInterfaces);
void gdUnityPluginUnload();

static IUnityInterfaces* g_unity_interface;

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    g_unity_interface = g_unity_interface;
    gdUnityPluginLoad(unityInterfaces);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginUnload()
{
    gdUnityPluginUnload();
}

static void UNITY_INTERFACE_API UnityRenderEvent(int eventID)
{
}

UnityRenderingEvent GetRenderEventFunc()
{
    return UnityRenderEvent;
}


IUnityInterfaces* GetUnityInterface()
{
    return g_unity_interface;
}

#ifdef _WIN32
#include <windows.h>
typedef IUnityInterfaces* (*GetUnityInterfaceT)();

void GfxForceInitialize()
{
    // PatchLibrary で突っ込まれたモジュールは UnityPluginLoad() が呼ばれないので、
    // 先にロードされているモジュールからインターフェースをもらって同等の処理を行う。
    HMODULE m = ::GetModuleHandleA("MassParticle.dll");
    if (m) {
        auto proc = (GetUnityInterfaceT)::GetProcAddress(m, "GetUnityInterface");
        if (proc) {
            auto *iface = proc();
            if (iface) {
                UnityPluginLoad(iface);
            }
        }
    }
}
#endif

#endif // gdStaticLink
