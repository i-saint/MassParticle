// Example low level rendering Unity plugin


#include "UnityPluginInterface.h"
#include <math.h>
#include <stdio.h>
#include <tbb/tbb.h>
#include "mpTypes.h"
#include "MassParticle.h"

extern mpWorld g_mpWorld;


bool mpInitialize(ID3D11Device *dev)
{
    tbb::task_scheduler_init tbb_init;

    g_mpWorld.m_renderer = mpCreateRendererD3D11(dev, g_mpWorld);
    return g_mpWorld.m_renderer !=nullptr;
}

void mpFinalize()
{
    delete g_mpWorld.m_renderer;
    g_mpWorld.m_renderer = nullptr;
}

void mpClearParticles()
{
    g_mpWorld.clearParticles();
}

extern "C" EXPORT_API void mpReloadShader()
{
    if (g_mpWorld.m_renderer) {
        g_mpWorld.m_renderer->reloadShader();
    }
}


extern "C" EXPORT_API ispc::KernelParams mpGetKernelParams()
{
    return g_mpWorld.m_params;
}

extern "C" EXPORT_API void mpSetKernelParams(ispc::KernelParams *params)
{
    g_mpWorld.m_params = *(mpKernelParams*)params;
}

extern "C" EXPORT_API void mpSetViewProjectionMatrix(XMFLOAT4X4 view_, XMFLOAT4X4 proj_, XMFLOAT3 camerapos)
{
	XMMATRIX view = (FLOAT*)&view_;
	XMMATRIX proj = (FLOAT*)&proj_;
	{
		float *v = (float*)&proj;
		v[4*0 + 2] = v[4*0 + 2] * 0.5f + v[4*0 + 3] * 0.5f;
		v[4*1 + 2] = v[4*1 + 2] * 0.5f + v[4*1 + 3] * 0.5f;
		v[4*2 + 2] = v[4*2 + 2] * 0.5f + v[4*2 + 3] * 0.5f;
		v[4*3 + 2] = v[4*3 + 2] * 0.5f + v[4*3 + 3] * 0.5f;
	}

	XMMATRIX viewproj = XMMatrixMultiply(view, proj);
	
	g_mpWorld.setViewProjection((XMFLOAT4X4&)viewproj, camerapos);
}

extern "C" EXPORT_API uint32_t mpGetNumParticles()
{
    return g_mpWorld.m_num_active_particles;
}
extern "C" EXPORT_API mpParticle* mpGetParticles()
{
    return &g_mpWorld.particles[0];
}

extern "C" EXPORT_API void mpCopyParticles(mpParticle *dst)
{
    memcpy(dst, &g_mpWorld.particles[0], sizeof(mpParticle)*mpGetNumParticles());
}

extern "C" EXPORT_API void mpWriteParticles(const mpParticle *from)
{
    memcpy(&g_mpWorld.particles[0], from, sizeof(mpParticle)*mpGetNumParticles());
}


inline XMVECTOR ComputeVelosity(XMFLOAT3 base, float vel_diffuse)
{
    XMVECTOR r = { base.x + mpGenRand()*vel_diffuse, base.y + mpGenRand()*vel_diffuse, base.z + mpGenRand()*vel_diffuse, 0.0f };
    return r;
}

extern "C" EXPORT_API int32_t mpScatterParticlesSphere(XMFLOAT3 center, float radius, int32_t num, XMFLOAT3 vel_base, float vel_diffuse)
{
    if (num <= 0) { return 0; }

    mpParticleVector particles(num);
    for (size_t i = 0; i < particles.size(); ++i) {
        float l = mpGenRand()*radius;
        XMVECTOR dir = { mpGenRand(), mpGenRand(), mpGenRand(), 0.0f };
        dir = XMVector3Normalize(dir);
        XMVECTOR pos = ist::simdvec4_set(center.x, center.y, center.z, 1.0f);
        pos = XMVectorAdd(pos, XMVectorMultiply(dir, ist::simdvec4_set(l, l, l, 0.0f)));
        XMVECTOR vel = ComputeVelosity(vel_base, vel_diffuse);

        particles[i].position = pos;
        particles[i].velocity = vel;
    }
    g_mpWorld.addParticles(&particles[0], particles.size());
    return num;
}

extern "C" EXPORT_API int32_t mpScatterParticlesBox(XMFLOAT3 center, XMFLOAT3 size, int32_t num, XMFLOAT3 vel_base, float vel_diffuse)
{
    if (num <= 0) { return 0; }

    mpParticleVector particles(num);
    for (size_t i = 0; i < particles.size(); ++i) {
        XMVECTOR pos = { center.x + mpGenRand()*size.x, center.y + mpGenRand()*size.y, center.z + mpGenRand()*size.z, 1.0f };
        XMVECTOR vel = ComputeVelosity(vel_base, vel_diffuse);

        particles[i].position = pos;
        particles[i].velocity = vel;
    }
    g_mpWorld.addParticles(&particles[0], particles.size());
    return num;
}


extern "C" EXPORT_API int32_t mpScatterParticlesSphereTransform(XMFLOAT4X4 transform, int32_t num, XMFLOAT3 vel_base, float vel_diffuse)
{
    if (num <= 0) { return 0; }

    mpParticleVector particles(num);
    XMMATRIX mat = XMMATRIX((float*)&transform);
    for (size_t i = 0; i < particles.size(); ++i) {
        XMVECTOR dir = { mpGenRand(), mpGenRand(), mpGenRand(), 0.0f };
        dir = XMVector3Normalize(dir);
        dir.m128_f32[3] = 1.0f;
        float l = mpGenRand()*0.5f;
        XMVECTOR pos = XMVectorMultiply(dir, ist::simdvec4_set(l, l, l, 1.0f));
        pos = XMVector4Transform(pos, mat);
        XMVECTOR vel = ComputeVelosity(vel_base, vel_diffuse);

        particles[i].position = pos;
        particles[i].velocity = vel;
    }
    g_mpWorld.addParticles(&particles[0], particles.size());
    return num;
}

extern "C" EXPORT_API int32_t mpScatterParticlesBoxTransform(XMFLOAT4X4 transform, int32_t num, XMFLOAT3 vel_base, float vel_diffuse)
{
    if (num <= 0) { return 0; }

    mpParticleVector particles(num);
    XMMATRIX mat = XMMATRIX((float*)&transform);
    for (size_t i = 0; i < particles.size(); ++i) {
        XMVECTOR pos = { mpGenRand()*0.5f, mpGenRand()*0.5f, mpGenRand()*0.5f, 1.0f };
        pos = XMVector4Transform(pos, mat);
        XMVECTOR vel = ComputeVelosity(vel_base, vel_diffuse);

        particles[i].position = pos;
        particles[i].velocity = vel;
    }
    g_mpWorld.addParticles(&particles[0], particles.size());
    return num;
}



inline void mpBuildBoxCollider(ispc::BoxCollider &o, uint32_t owner, XMFLOAT4X4 transform, XMFLOAT3 size)
{
    float psize = g_mpWorld.m_params.ParticleSize;
    size.x = size.x * 0.5f;
    size.y = size.y * 0.5f;
    size.z = size.z * 0.5f;

    XMMATRIX st = XMMATRIX((float*)&transform);
    XMVECTOR vertices[] = {
        { size.x, size.y, size.z, 0.0f },
        { -size.x, size.y, size.z, 0.0f },
        { -size.x, -size.y, size.z, 0.0f },
        { size.x, -size.y, size.z, 0.0f },
        { size.x, size.y, -size.z, 0.0f },
        { -size.x, size.y, -size.z, 0.0f },
        { -size.x, -size.y, -size.z, 0.0f },
        { size.x, -size.y, -size.z, 0.0f },
    };
    for (int i = 0; i < _countof(vertices); ++i) {
        vertices[i] = XMVector4Transform(vertices[i], st);
    }

    XMVECTOR normals[6] = {
        XMVector3Normalize(XMVector3Cross(XMVectorSubtract(vertices[3], vertices[0]), XMVectorSubtract(vertices[4], vertices[0]))),
        XMVector3Normalize(XMVector3Cross(XMVectorSubtract(vertices[5], vertices[1]), XMVectorSubtract(vertices[2], vertices[1]))),
        XMVector3Normalize(XMVector3Cross(XMVectorSubtract(vertices[7], vertices[3]), XMVectorSubtract(vertices[2], vertices[3]))),
        XMVector3Normalize(XMVector3Cross(XMVectorSubtract(vertices[1], vertices[0]), XMVectorSubtract(vertices[4], vertices[0]))),
        XMVector3Normalize(XMVector3Cross(XMVectorSubtract(vertices[1], vertices[0]), XMVectorSubtract(vertices[3], vertices[0]))),
        XMVector3Normalize(XMVector3Cross(XMVectorSubtract(vertices[7], vertices[4]), XMVectorSubtract(vertices[5], vertices[4]))),
    };
    float32 distances[6] = {
        -(XMVector3Dot(vertices[0], normals[0]).m128_f32[0] + psize),
        -(XMVector3Dot(vertices[1], normals[1]).m128_f32[0] + psize),
        -(XMVector3Dot(vertices[0], normals[2]).m128_f32[0] + psize),
        -(XMVector3Dot(vertices[3], normals[3]).m128_f32[0] + psize),
        -(XMVector3Dot(vertices[0], normals[4]).m128_f32[0] + psize),
        -(XMVector3Dot(vertices[4], normals[5]).m128_f32[0] + psize),
    };


    o.id = owner;
    o.x = transform.m[3][0];
    o.y = transform.m[3][1];
    o.z = transform.m[3][2];
    for (int i = 0; i < 6; ++i) {
        o.planes[i].nx = normals[i].m128_f32[0];
        o.planes[i].ny = normals[i].m128_f32[1];
        o.planes[i].nz = normals[i].m128_f32[2];
        o.planes[i].distance = distances[i];
    }
    for (int i = 0; i < _countof(vertices); ++i) {
        float x = o.x + vertices[i].m128_f32[0];
        float y = o.y + vertices[i].m128_f32[1];
        float z = o.z + vertices[i].m128_f32[2];
        if (i == 0) {
            o.bb.bl_x = o.bb.ur_x = x;
            o.bb.bl_y = o.bb.ur_y = y;
            o.bb.bl_z = o.bb.ur_z = z;
        }
        o.bb.bl_x = std::min<float>(o.bb.bl_x, x-psize);
        o.bb.bl_y = std::min<float>(o.bb.bl_y, y-psize);
        o.bb.bl_z = std::min<float>(o.bb.bl_z, z-psize);
        o.bb.ur_x = std::max<float>(o.bb.ur_x, x+psize);
        o.bb.ur_y = std::max<float>(o.bb.ur_y, y+psize);
        o.bb.ur_z = std::max<float>(o.bb.ur_z, z+psize);
    }
}

inline void mpBuildSphereCollider(ispc::SphereCollider &o, uint32_t owner, XMFLOAT3 center, float radius)
{
    float psize = g_mpWorld.m_params.ParticleSize;
    float er = radius + psize;
    o.id = owner;
    o.x = center.x;
    o.y = center.y;
    o.z = center.z;
    o.radius = radius;
    o.bb.bl_x = center.x - er;
    o.bb.bl_y = center.y - er;
    o.bb.bl_z = center.z - er;
    o.bb.ur_x = center.x + er;
    o.bb.ur_y = center.y + er;
    o.bb.ur_z = center.z + er;
}


extern "C" EXPORT_API uint32_t mpAddBoxCollider(int32_t owner, XMFLOAT4X4 transform, XMFLOAT3 size)
{
    ispc::BoxCollider col;
    mpBuildBoxCollider(col, owner, transform, size);
    g_mpWorld.collision_boxes.push_back(col);
    return 0;
}

extern "C" EXPORT_API uint32_t mpAddSphereCollider(int32_t owner, XMFLOAT3 center, float radius)
{
    ispc::SphereCollider col;
    mpBuildSphereCollider(col, owner, center, radius);
    g_mpWorld.collision_spheres.push_back(col);
    return 0;
}

extern "C" EXPORT_API uint32_t mpAddForce(int force_shape, XMFLOAT4X4 trans, int force_direction, ispc::ForceParams p)
{
    ispc::Force force;
    force.shape_type = force_shape;
    force.dir_type = force_direction;
    force.params = p;

    switch (force.shape_type) {
    case mpFS_Box:
    {
        ispc::BoxCollider col;
        mpBuildBoxCollider(col, 0, trans, XMFLOAT3(1.0f, 1.0f, 1.0f));
        force.bb = col.bb;
        force.shape_box.x = col.x;
        force.shape_box.y = col.y;
        force.shape_box.z = col.z;
        for (int i = 0; i < 6; ++i) {
            force.shape_box.planes[i] = col.planes[i];
        }
    }
        break;

    case mpFS_Sphere:
    {
        ispc::SphereCollider col;
        XMFLOAT3 pos = XMFLOAT3(trans.m[3][0], trans.m[3][1], trans.m[3][2]);
        float radius = (trans.m[0][0] + trans.m[1][1] + trans.m[2][2]) * 0.3333333333f * 0.5f;
        mpBuildSphereCollider(col, 0, pos, radius);
        force.bb = col.bb;
        force.shape_sphere.x = col.x;
        force.shape_sphere.y = col.y;
        force.shape_sphere.z = col.z;
        force.shape_sphere.radius = col.radius;
    }
        break;
    }
    g_mpWorld.forces.push_back(force);
    return g_mpWorld.forces.size() - 1;
}



extern "C" EXPORT_API void mpUpdate(float dt)
{
    static mpPerformanceCounter s_timer;
    static float s_prev = 0.0f;
    mpPerformanceCounter timer;

    {
        std::unique_lock<std::mutex> lock(g_mpWorld.m_mutex);
        g_mpWorld.update(1.0f);
        g_mpWorld.clearCollidersAndForces();
    }

    if (s_timer.getElapsedMillisecond() - s_prev > 1000.0f) {
        char buf[128];
        _snprintf(buf, _countof(buf), "  SPH update: %d particles %.3fms\n", g_mpWorld.m_num_active_particles, timer.getElapsedMillisecond());
        OutputDebugStringA(buf);
        s_prev = s_timer.getElapsedMillisecond();
    }
}
