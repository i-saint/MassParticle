#pragma once

struct IUnityInterfaces;

namespace gd {
    void UnityPluginLoad(IUnityInterfaces* unityInterfaces);
    void UnityPluginUnload();
} // namespace gd
