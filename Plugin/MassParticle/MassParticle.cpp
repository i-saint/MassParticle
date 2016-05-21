#include "pch.h"
#include "mpInternal.h"
#include "mpWorld.h"
#include "MassParticle.h"

namespace {
    std::vector<mpWorld*> g_worlds;
}

extern "C" {

#ifndef mpStaticLink
mpAPI void UnitySetGraphicsDevice(void* device, int deviceType, int eventType)
{
    mpUnitySetGraphicsDevice(device, deviceType, eventType);
}

mpAPI void UnityRenderEvent(int eventID)
{
}
#endif // mpStaticLink


mpAPI void mpUpdateDataTexture(int context, void *tex, int width, int height)
{
    if (context == 0) return;
    g_worlds[context]->updateDataTexture(tex, width, height);
}



mpAPI int mpCreateContext()
{
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
mpAPI void mpDestroyContext(int context)
{
    delete g_worlds[context];
    g_worlds[context] = nullptr;
}


mpAPI void mpUpdate(int context, float dt)
{
    g_worlds[context]->update(dt);
}
mpAPI void mpBeginUpdate(int context, float dt)
{
    g_worlds[context]->beginUpdate(dt);
}
mpAPI void mpEndUpdate(int context)
{
    g_worlds[context]->endUpdate();
}
mpAPI void mpCallHandlers(int context)
{
    g_worlds[context]->callHandlers();
}


mpAPI void mpClearParticles(int context)
{
    g_worlds[context]->clearParticles();
}

mpAPI void mpClearCollidersAndForces(int context)
{
    g_worlds[context]->clearCollidersAndForces();
}

mpAPI void mpGetKernelParams(int context, mpKernelParams *params)
{
    *params = g_worlds[context]->getKernelParams();
}

mpAPI void mpSetKernelParams(int context, const mpKernelParams *params)
{
    g_worlds[context]->setKernelParams(*params);
}


mpAPI int mpGetNumParticles(int context)
{
    if (context == 0) return 0;
    return g_worlds[context]->getNumParticles();
}

mpAPI void mpForceSetNumParticles(int context, int num)
{
    g_worlds[context]->forceSetNumParticles(num);
}
mpAPI mpParticleIM*	mpGetIntermediateData(int context, int nth)
{
    return nth < 0 ?
        &g_worlds[context]->getIntermediateData() :
        &g_worlds[context]->getIntermediateData(nth);
}

mpAPI mpParticle* mpGetParticles(int context)
{
    return g_worlds[context]->getParticles();
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

mpAPI void mpScatterParticlesSphere(int context, vec3 *center, float radius, int32_t num, const mpSpawnParams *params)
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

mpAPI void mpScatterParticlesBox(int context, vec3 *center, vec3 *size, int32_t num, const mpSpawnParams *params)
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


mpAPI void mpScatterParticlesSphereTransform(int context, mat4 *transform, int32_t num, const mpSpawnParams *params)
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

mpAPI void mpScatterParticlesBoxTransform(int context, mat4 *transform, int32_t num, const mpSpawnParams *params)
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



inline void mpBuildBoxCollider(int context, mpBoxCollider &o, const mat4 &transform, const vec3 &center, const vec3 &_size)
{
    float psize = g_worlds[context]->getKernelParams().particle_size;
    vec3 size = _size * 0.5f;

    simdmat4 st = simdmat4(transform);
    simdvec4 vertices[] = {
        simdvec4(size.x + center.x, size.y + center.y, size.z + center.z, 0.0f),
        simdvec4(-size.x + center.x, size.y + center.y, size.z + center.z, 0.0f),
        simdvec4(-size.x + center.x, -size.y + center.y, size.z + center.z, 0.0f),
        simdvec4(size.x + center.x, -size.y + center.y, size.z + center.z, 0.0f),
        simdvec4(size.x + center.x, size.y + center.y, -size.z + center.z, 0.0f),
        simdvec4(-size.x + center.x, size.y + center.y, -size.z + center.z, 0.0f),
        simdvec4(-size.x + center.x, -size.y + center.y, -size.z + center.z, 0.0f),
        simdvec4(size.x + center.x, -size.y + center.y, -size.z + center.z, 0.0f),
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


mpAPI void mpAddBoxCollider(int context, mpColliderProperties *props, mat4 *transform, vec3 *size, vec3 *center)
{
    mpBoxCollider col;
    col.props = *props;
    mpBuildBoxCollider(context, col, *transform, *size, *center);
    g_worlds[context]->addBoxColliders(&col, 1);
}

mpAPI void mpRemoveCollider(int context, mpColliderProperties *props)
{
    g_worlds[context]->removeCollider(*props);
}

mpAPI void mpAddSphereCollider(int context, mpColliderProperties *props, vec3 *center, float radius)
{
    mpSphereCollider col;
    col.props = *props;
    mpBuildSphereCollider(context, col, *center, radius);
    g_worlds[context]->addSphereColliders(&col, 1);
}

mpAPI void mpAddCapsuleCollider(int context, mpColliderProperties *props, vec3 *pos1, vec3 *pos2, float radius)
{
    mpCapsuleCollider col;
    col.props = *props;
    mpBuildCapsuleCollider(context, col, *pos1, *pos2, radius);
    g_worlds[context]->addCapsuleColliders(&col, 1);
}

mpAPI void mpAddForce(int context, mpForceProperties *props, mat4 *_trans)
{
    mat4 &trans = *_trans;
    mpForce force;
    force.props = *props;

    switch ((mpForceShape)force.props.shape_type) {
    case mpForceShape::Sphere:
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

    case mpForceShape::Capsule:
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

    case mpForceShape::Box:
        {
            mpBoxCollider col;
            mpBuildBoxCollider(context, col, trans, vec3(), vec3(1.0f, 1.0f, 1.0f));
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

mpAPI void mpScanSphere(int context, mpHitHandler handler, vec3 *center, float radius)
{
    return g_worlds[context]->scanSphere(handler, *center, radius);
}
mpAPI void mpScanAABB(int context, mpHitHandler handler, vec3 *center, vec3 *extent)
{
    return g_worlds[context]->scanAABB(handler, *center, *extent);
}
mpAPI void mpScanSphereParallel(int context, mpHitHandler handler, vec3 *center, float radius)
{
    return g_worlds[context]->scanSphereParallel(handler, *center, radius);
}
mpAPI void mpScanAABBParallel(int context, mpHitHandler handler, vec3 *center, vec3 *extent)
{
    return g_worlds[context]->scanAABBParallel(handler, *center, *extent);
}

mpAPI void mpScanAll(int context, mpHitHandler handler)
{
    return g_worlds[context]->scanAll(handler);
}

mpAPI void mpScanAllParallel(int context, mpHitHandler handler)
{
    return g_worlds[context]->scanAllParallel(handler);
}

mpAPI void mpMoveAll(int context, vec3 *move_amount)
{
    g_worlds[context]->moveAll(*move_amount);
}

} // extern "C"
