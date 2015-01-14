// Example low level rendering Unity plugin


#include "UnityPluginInterface.h"
#include <math.h>
#include <stdio.h>
#include <tbb/tbb.h>
#include <random>
#include "MassParticle.h"
#include "mpTypes.h"

extern mpRenderer *g_mpRenderer;
std::vector<mpWorld*> g_worlds;


std::mt19937 s_rand;

float mpGenRand()
{
    static std::uniform_real_distribution<float> s_dist(-1.0f, 1.0f);
    return s_dist(s_rand);
}

float mpGenRand1()
{
    static std::uniform_real_distribution<float> s_dist(0.0f, 1.0f);
    return s_dist(s_rand);
}


extern "C" EXPORT_API void mpGeneratePointMesh(int context, int mi, mpMeshData *mds)
{
    g_worlds[context]->generatePointMesh(mi, mds);
}

extern "C" EXPORT_API void mpGenerateCubeMesh(int context, int mi, mpMeshData *mds)
{
    g_worlds[context]->generateCubeMesh(mi, mds);
}

extern "C" EXPORT_API int mpUpdateDataTexture(int context, void *tex)
{
    return g_worlds[context]->updateDataTexture(tex);
}

#ifdef mpWithCppScript
extern "C" EXPORT_API int mpUpdateDataBuffer(int context, UnityEngine::ComputeBuffer buf)
{
    return g_worlds[context]->updateDataBuffer(buf);
}
struct mpUpdateDataBufferHelper
{
    mpUpdateDataBufferHelper()
    {
        cpsAddMethod("MPAPI::mpUpdateDataBuffer", &mpUpdateDataBuffer);
    }
} g_mpUpdateDataBufferHelper;

#endif // mpWithCppScript




extern "C" EXPORT_API int mpCreateContext()
{
#ifdef mpWithCppScript
    cpsClearCache();
#endif // mpWithCppScript

    mpWorld *p = new mpWorld();

    if (g_worlds.empty()) {
        g_worlds.push_back(nullptr);
    }
    for (int i = 1; i < (int)g_worlds.size(); ++i) {
        if (g_worlds[i] == nullptr) {
            g_worlds[i] = p;
            return i;
        }
    }
    g_worlds.push_back(p);
    return (int)g_worlds.size()-1;
}
extern "C" EXPORT_API void mpDestroyContext(int context)
{
    delete g_worlds[context];
    g_worlds[context] = nullptr;
}


extern "C" EXPORT_API void mpBeginUpdate(int context, float dt)
{
    g_worlds[context]->beginUpdate(dt);
}
extern "C" EXPORT_API void mpEndUpdate(int context)
{
    g_worlds[context]->endUpdate();
}
extern "C" EXPORT_API void mpUpdate(int context, float dt)
{
    g_worlds[context]->update(dt);
}
extern "C" EXPORT_API void mpCallHandlers(int context)
{
    g_worlds[context]->callHandlers();
}


extern "C" EXPORT_API void mpClearParticles(int context)
{
    g_worlds[context]->clearParticles();
}

extern "C" EXPORT_API void mpClearCollidersAndForces(int context)
{
    g_worlds[context]->clearCollidersAndForces();
}

extern "C" EXPORT_API ispc::KernelParams mpGetKernelParams(int context)
{
    return g_worlds[context]->getKernelParams();
}

extern "C" EXPORT_API void mpSetKernelParams(int context, ispc::KernelParams *params)
{
    g_worlds[context]->setKernelParams(*(mpKernelParams*)params);
}


extern "C" EXPORT_API int mpGetNumParticles(int context)
{
    return g_worlds[context]->getNumParticles();
}
extern "C" EXPORT_API mpParticleIM*	mpGetIntermediateData(int context, int nth)
{
    return nth < 0 ?
        &g_worlds[context]->getIntermediateData() :
        &g_worlds[context]->getIntermediateData(nth);
}

extern "C" EXPORT_API mpParticle* mpGetParticles(int context)
{
    return g_worlds[context]->getParticles();
}

extern "C" EXPORT_API void mpCopyParticles(int context, mpParticle *dst)
{
    memcpy(dst, g_worlds[context]->getParticles(), sizeof(mpParticle)*mpGetNumParticles(context));
}

extern "C" EXPORT_API void mpWriteParticles(int context, const mpParticle *from)
{
    memcpy(g_worlds[context]->getParticles(), from, sizeof(mpParticle)*mpGetNumParticles(context));
}


inline void mpApplySpawnParams(mpParticleCont &particles, const mpSpawnParams *params)
{
    if (params == nullptr) return;

    vec3 vel = params->velocity_base;
    float vel_diffuse = params->velocity_random_diffuse;
    float lifetime = params->lifetime;
    float lifetime_diffuse = params->lifetime_random_diffuse;
    int userdata = params->userdata;
    mpHitHandler handler = params->handler;

    for (auto &p : particles) {
        (vec3&)p.velocity = vec3(
            vel.x + mpGenRand()*vel_diffuse,
            vel.y + mpGenRand()*vel_diffuse,
            vel.z + mpGenRand()*vel_diffuse);
        p.lifetime = lifetime + mpGenRand()*lifetime_diffuse;
        p.userdata = userdata;
    }
    if (handler) {
        for (auto &p : particles) {
            handler(&p);
        }
    }
}

extern "C" EXPORT_API void mpScatterParticlesSphere(int context, vec3 *center, float radius, int32_t num, const mpSpawnParams *params)
{
    if (num <= 0) { return; }

    mpParticleCont particles(num);
    for (auto &p : particles) {
        float l = mpGenRand()*radius;
        vec3 dir = glm::normalize(vec3(mpGenRand(), mpGenRand(), mpGenRand()));
        vec3 pos = *center + dir*l;
        (vec3&)p.position = pos;
    }
    mpApplySpawnParams(particles, params);
    g_worlds[context]->addParticles(&particles[0], particles.size());
}

extern "C" EXPORT_API void mpScatterParticlesBox(int context, vec3 *center, vec3 *size, int32_t num, const mpSpawnParams *params)
{
    if (num <= 0) { return; }

    mpParticleCont particles(num);
    for (auto &p : particles) {
        vec3 pos = *center + vec3(mpGenRand()*size->x, mpGenRand()*size->y, mpGenRand()*size->z);
        (vec3&)p.position = pos;
    }
    mpApplySpawnParams(particles, params);
    g_worlds[context]->addParticles(&particles[0], particles.size());
}


extern "C" EXPORT_API void mpScatterParticlesSphereTransform(int context, mat4 *transform, int32_t num, const mpSpawnParams *params)
{
    if (num <= 0) { return; }

    mpParticleCont particles(num);
    simdmat4 mat(*transform);
    for (auto &p : particles) {
        vec3 dir = glm::normalize(vec3(mpGenRand(), mpGenRand(), mpGenRand()));
        float l = mpGenRand()*0.5f;
        simdvec4 pos = simdvec4(dir*l, 1.0f);
        pos = mat * pos;
        (vec3&)p.position = (vec3&)pos;
    }
    mpApplySpawnParams(particles, params);
    g_worlds[context]->addParticles(&particles[0], particles.size());
}

extern "C" EXPORT_API void mpScatterParticlesBoxTransform(int context, mat4 *transform, int32_t num, const mpSpawnParams *params)
{
    if (num <= 0) { return; }

    mpParticleCont particles(num);
    simdmat4 mat(*transform);
    for (auto &p : particles) {
        simdvec4 pos(mpGenRand()*0.5f, mpGenRand()*0.5f, mpGenRand()*0.5f, 1.0f);
        pos = mat * pos;
        (vec3&)p.position = (vec3&)pos;
    }
    mpApplySpawnParams(particles, params);
    g_worlds[context]->addParticles(&particles[0], particles.size());
}



inline void mpBuildBoxCollider(int context, mpBoxCollider &o, mat4 transform, vec3 size)
{
    float psize = g_worlds[context]->getKernelParams().particle_size;
    size.x = size.x * 0.5f;
    size.y = size.y * 0.5f;
    size.z = size.z * 0.5f;

    simdmat4 st = simdmat4(transform);
    simdvec4 vertices[] = {
        simdvec4(size.x, size.y, size.z, 0.0f),
        simdvec4(-size.x, size.y, size.z, 0.0f),
        simdvec4(-size.x, -size.y, size.z, 0.0f),
        simdvec4(size.x, -size.y, size.z, 0.0f),
        simdvec4(size.x, size.y, -size.z, 0.0f),
        simdvec4(-size.x, size.y, -size.z, 0.0f),
        simdvec4(-size.x, -size.y, -size.z, 0.0f),
        simdvec4(size.x, -size.y, -size.z, 0.0f),
    };
    for (int i = 0; i < mpCountof(vertices); ++i) {
        vertices[i] = st * vertices[i];
    }

    simdvec4 normals[6] = {
        glm::normalize(glm::cross(vertices[3] - vertices[0], vertices[4] - vertices[0])),
        glm::normalize(glm::cross(vertices[5] - vertices[1], vertices[2] - vertices[1])),
        glm::normalize(glm::cross(vertices[7] - vertices[3], vertices[2] - vertices[3])),
        glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[4] - vertices[0])),
        glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[3] - vertices[0])),
        glm::normalize(glm::cross(vertices[7] - vertices[4], vertices[5] - vertices[4])),
    };
    float distances[6] = {
        -(glm::dot(vertices[0], normals[0]) + psize),
        -(glm::dot(vertices[1], normals[1]) + psize),
        -(glm::dot(vertices[0], normals[2]) + psize),
        -(glm::dot(vertices[3], normals[3]) + psize),
        -(glm::dot(vertices[0], normals[4]) + psize),
        -(glm::dot(vertices[4], normals[5]) + psize),
    };

    (vec3&)o.shape.center = (vec3&)transform[3][0];
    for (int i = 0; i < 6; ++i) {
        (vec3&)o.shape.planes[i].normal = (vec3&)normals[i];
        o.shape.planes[i].distance = distances[i];
    }
    for (int i = 0; i < mpCountof(vertices); ++i) {
        vec3 p = (vec3&)vertices[i] + (vec3&)o.shape.center;
        if (i == 0) {
            (vec3&)o.bounds.bl = p;
            (vec3&)o.bounds.ur = p;
        }
        (vec3&)o.bounds.bl = glm::min((vec3&)o.bounds.bl, p - psize);
        (vec3&)o.bounds.ur = glm::max((vec3&)o.bounds.ur, p + psize);
    }
}

inline void mpBuildSphereCollider(int context, mpSphereCollider &o, vec3 center, float radius)
{
    float psize = g_worlds[context]->getKernelParams().particle_size;
    float er = radius + psize;
    (vec3&)o.shape.center = center;
    o.shape.radius = er;
    (vec3&)o.bounds.bl = center - er;
    (vec3&)o.bounds.ur = center + er;
}

inline void mpBuildCapsuleCollider(int context, mpCapsuleCollider &o, vec3 pos1, vec3 pos2, float radius)
{
    float psize = g_worlds[context]->getKernelParams().particle_size;
    float er = radius + psize;

    ispc::Capsule &shape = o.shape;
    (vec3&)shape.pos1 = pos1;
    (vec3&)shape.pos2 = pos2;
    shape.radius = er;
    float len_sq = glm::length_sq((vec3&)shape.pos2 - (vec3&)shape.pos1);
    shape.rcp_lensq = 1.0f / len_sq;

    (vec3&)o.bounds.bl = glm::min((vec3&)shape.pos1 - er, (vec3&)(shape.pos2) - er);
    (vec3&)o.bounds.ur = glm::max((vec3&)shape.pos1 + er, (vec3&)(shape.pos2) + er);
}


extern "C" EXPORT_API void mpAddBoxCollider(int context, mpColliderProperties *props, mat4 *transform, vec3 *size)
{
    mpBoxCollider col;
    col.props = *props;
    mpBuildBoxCollider(context, col, *transform, *size);
    g_worlds[context]->addBoxColliders(&col, 1);
}

extern "C" EXPORT_API void mpRemoveCollider(int context, mpColliderProperties *props)
{
    g_worlds[context]->removeCollider(*props);
}

extern "C" EXPORT_API void mpAddSphereCollider(int context, mpColliderProperties *props, vec3 *center, float radius)
{
    mpSphereCollider col;
    col.props = *props;
    mpBuildSphereCollider(context, col, *center, radius);
    g_worlds[context]->addSphereColliders(&col, 1);
}

extern "C" EXPORT_API void mpAddCapsuleCollider(int context, mpColliderProperties *props, vec3 *pos1, vec3 *pos2, float radius)
{
    mpCapsuleCollider col;
    col.props = *props;
    mpBuildCapsuleCollider(context, col, *pos1, *pos2, radius);
    g_worlds[context]->addCapsuleColliders(&col, 1);
}

extern "C" EXPORT_API void mpAddForce(int context, mpForceProperties *props, mat4 *_trans)
{
    mat4 &trans = *_trans;
    mpForce force;
    force.props = *props;
    force.props.rcp_range = 1.0f / (force.props.range_outer - force.props.range_inner);

    switch (force.props.shape_type) {
    case mpFS_Sphere:
        {
            mpSphereCollider col;
            vec3 pos = (vec3&)trans[3];
            float radius = (trans[0][0] + trans[1][1] + trans[2][2]) * 0.3333333333f * 0.5f;
            mpBuildSphereCollider(context, col, pos, radius);
            force.bounds = col.bounds;
            force.sphere.center = col.shape.center;
            force.sphere.radius = col.shape.radius;
        }
        break;

    case mpFS_Capsule:
        {
            mpCapsuleCollider col;
            vec3 pos = (vec3&)trans[3];
            float radius = (trans[0][0] + trans[2][2]) * 0.5f * 0.5f;
            mpBuildCapsuleCollider(context, col, pos, pos, radius);
            force.bounds = col.bounds;
            force.sphere.center = col.shape.center;
            force.sphere.radius = col.shape.radius;
        }
        break;

    case mpFS_Box:
        {
            mpBoxCollider col;
            mpBuildBoxCollider(context, col, trans, vec3(1.0f, 1.0f, 1.0f));
            force.bounds = col.bounds;
            force.box.center = col.shape.center;
            for (int i = 0; i < 6; ++i) {
                force.box.planes[i] = col.shape.planes[i];
            }
        }
        break;

    }
    g_worlds[context]->addForces(&force, 1);
}

extern "C" EXPORT_API void mpScanSphere(int context, mpHitHandler handler, vec3 *center, float radius)
{
    return g_worlds[context]->scanSphere(handler, *center, radius);
}
extern "C" EXPORT_API void mpScanAABB(int context, mpHitHandler handler, vec3 *center, vec3 *extent)
{
    return g_worlds[context]->scanAABB(handler, *center, *extent);
}
extern "C" EXPORT_API void mpScanSphereParallel(int context, mpHitHandler handler, vec3 *center, float radius)
{
    return g_worlds[context]->scanSphereParallel(handler, *center, radius);
}
extern "C" EXPORT_API void mpScanAABBParallel(int context, mpHitHandler handler, vec3 *center, vec3 *extent)
{
    return g_worlds[context]->scanAABBParallel(handler, *center, *extent);
}

extern "C" EXPORT_API void mpScanAll(int context, mpHitHandler handler)
{
    return g_worlds[context]->scanAll(handler);
}

extern "C" EXPORT_API void mpScanAllParallel(int context, mpHitHandler handler)
{
    return g_worlds[context]->scanAllParallel(handler);
}

extern "C" EXPORT_API void mpMoveAll(int context, vec3 *move_amount)
{
    g_worlds[context]->moveAll(*move_amount);
}
