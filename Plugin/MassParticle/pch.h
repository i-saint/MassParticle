#include <cmath>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <random>
#include <mutex>

#define GLM_FORCE_RADIANS
#ifdef _WIN64
#   define GLM_FORCE_SSE4
#endif
#include <glm/glm.hpp>
#include <glm/gtx/simd_vec4.hpp>
#include <glm/gtx/simd_mat4.hpp>

#if defined(_WIN32) && !defined(mpWithTBB)
    #include <Windows.h>
    #include <ppl.h>
#else
    #include <tbb/tbb.h>
    #include <tbb/combinable.h>
#endif

#ifdef max
    #undef max
    #undef min
#endif // max
