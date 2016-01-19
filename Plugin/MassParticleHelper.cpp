#include <string>
#ifdef _MSC_VER
#include <windows.h>
#endif // _MSC_VER
#include "UnityPluginInterface.h"


extern "C" EXPORT_API void mphInitialize()
{
    static bool s_is_first = true;
    if (s_is_first) {
        s_is_first = false;

        std::string path;
        path.resize(1024 * 64);

#ifdef _MSC_VER
        DWORD ret = ::GetEnvironmentVariableA("PATH", &path[0], (DWORD)path.size());
        path.resize(ret);
        {
            char path_to_this_module[MAX_PATH+1];
            HMODULE mod = 0;
            ::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)&mphInitialize, &mod);
            DWORD size = ::GetModuleFileNameA(mod, path_to_this_module, sizeof(path_to_this_module));
            for (int i = size - 1; i >= 0; --i) {
                if (path_to_this_module[i]=='\\') {
                    path_to_this_module[i] = '\0';
                    break;
                }
            }
            path += ";";
            path += path_to_this_module;
        }
        ::SetEnvironmentVariableA("PATH", path.c_str());
#elif defined(__APPLE__)

#endif
    }
}
