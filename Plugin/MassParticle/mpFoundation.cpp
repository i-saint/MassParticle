#include "pch.h"
#include "mpFoundation.h"

namespace {
    std::mt19937 g_rand;
}

float mpGenRand()
{
    static std::uniform_real_distribution<float> s_dist(-1.0f, 1.0f);
    return s_dist(g_rand);
}

float mpGenRand1()
{
    static std::uniform_real_distribution<float> s_dist(0.0f, 1.0f);
    return s_dist(g_rand);
}

void* mpAlignedAlloc(size_t size, size_t align)
{
#ifdef _MSC_VER
    return _aligned_malloc(size, align);
#elif defined(__APPLE__)
    return malloc(size);
#else  // _MSC_VER
    return memalign(align, size);
#endif // _MSC_VER
}

void mpAlignedFree(void *p)
{
#ifdef _MSC_VER
    _aligned_free(p);
#else  // _MSC_VER
    free(p);
#endif // _MSC_VER
}


void mpSoAData::resize(size_t n)
{
    pos_x.resize(n);
    pos_y.resize(n);
    pos_z.resize(n);
    vel_x.resize(n);
    vel_y.resize(n);
    vel_z.resize(n);
    acl_x.resize(n);
    acl_y.resize(n);
    acl_z.resize(n);
    speed.resize(n);
    density.resize(n);
    affection.resize(n);
    hit.resize(n);
}
