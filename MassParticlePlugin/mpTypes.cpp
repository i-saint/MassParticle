//#include "stdafx.h"
#include <algorithm>
#include <random>
#include "mpTypes.h"
#include "MassParticle.h"


std::mt19937 g_rand;

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


mpKernelParams::mpKernelParams()
{
    (vec3&)world_center = vec3(0.0f, 0.0f, 0.0f);
    (vec3&)world_extent = vec3(10.24f, 10.24f, 10.24f);
    (vec3&)coord_scaler = vec3(1.0f, 1.0f, 1.0f);

    enable_interaction = 1;
    enable_colliders = 1;
    enable_forces = 1;

    solver_type = mpSolverType_Impulse;
    timestep = 0.01f;
    damping = 0.6f;
    advection = 0.5f;

    pressure_stiffness = 500.0f;
    max_particles = 100000;
    particle_size = 0.08f;

    SPHRestDensity = 1000.0f;
    SPHParticleMass = 0.002f;
    SPHViscosity = 0.1f;
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
