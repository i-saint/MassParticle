#include "pch.h"
#include "mpInternal.h"
#include "GraphicsDevice.h"
#include "gdUnityPluginImpl.h"
#include "PluginAPI/IUnityGraphics.h"

#ifndef mpStaticLink

static IUnityInterfaces* g_unity_interface;

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    g_unity_interface = g_unity_interface;
    gd::UnityPluginLoad(unityInterfaces);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginUnload()
{
    gd::UnityPluginUnload();
}

static void UNITY_INTERFACE_API UnityRenderEvent(int eventID)
{
}

UnityRenderingEvent GetRenderEventFunc()
{
    return UnityRenderEvent;
}


extern "C" UNITY_INTERFACE_EXPORT IUnityInterfaces* GetUnityInterface()
{
    return g_unity_interface;
}

#ifdef _WIN32
#include <windows.h>
typedef IUnityInterfaces* (*GetUnityInterfaceT)();

BOOL WINAPI DllMain(HINSTANCE module_handle, DWORD reason_for_call, LPVOID reserved)
{
    if (reason_for_call == DLL_PROCESS_ATTACH)
    {
        // PatchLibrary で突っ込まれたモジュールは UnityPluginLoad() が呼ばれないので、
        // 先にロードされているモジュールからインターフェースをもらって同等の処理を行う。
        auto mod = ::GetModuleHandleA("MassParticle.dll");
        if (mod) {
            auto proc = (GetUnityInterfaceT)::GetProcAddress(mod, "GetUnityInterface");
            if (proc) {
                auto *iface = proc();
                if (iface) {
                    UnityPluginLoad(iface);
                }
            }
        }
    }
    return TRUE;
}

#endif // _WIN32
#endif // mpStaticLink
