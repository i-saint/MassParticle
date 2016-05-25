#include "pch.h"
#include "mpInternal.h"
#include "GraphicsDevice.h"
#include "PluginAPI/IUnityGraphics.h"

#ifndef mpStaticLink

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


UNITY_INTERFACE_EXPORT IUnityInterfaces* GetUnityInterface()
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

BOOL WINAPI DllMain(HINSTANCE module_handle, DWORD reason_for_call, LPVOID reserved)
{
    if (reason_for_call == DLL_PROCESS_ATTACH)
    {
        GfxForceInitialize();
    }
    return TRUE;
}

#endif // _WIN32
#endif // mpStaticLink
