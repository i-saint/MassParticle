#pragma once
#include "mpConcurrency.h"

class mpWorld
{
public:

    mpWorld();
    ~mpWorld();
    void beginUpdate(float dt);
    void endUpdate();
    void update(float dt);
    void callHandlers();

    void addParticles(mpParticle *p, size_t num);
    void addPlaneColliders(mpPlaneCollider *col, size_t num);
    void addSphereColliders(mpSphereCollider *col, size_t num);
    void addCapsuleColliders(mpCapsuleCollider *col, size_t num);
    void addBoxColliders(mpBoxCollider *col, size_t num);
    void removeCollider(mpColliderProperties &props);
    void addForces(mpForce *force, size_t num);

    void scanSphere(mpHitHandler handler, const vec3 &pos, float radius);
    void scanAABB(mpHitHandler handler, const vec3 &center, const vec3 &extent);
    void scanSphereParallel(mpHitHandler handler, const vec3 &pos, float radius);
    void scanAABBParallel(mpHitHandler handler, const vec3 &center, const vec3 &extent);
    void scanAll(mpHitHandler handler);
    void scanAllParallel(mpHitHandler handler);

    void moveAll(const vec3 &move);

    void clearParticles();
    void clearCollidersAndForces();

    const mpKernelParams&   getKernelParams() const;
    void                    setKernelParams(const mpKernelParams &v);
    mpTempParams&           getTempParams();
    const mpCellCont&       getCells();

    void        forceSetNumParticles(int v);
    int         getNumParticles() const;
    mpParticle* getParticles();
    int         getNumParticlesGPU() const;
    mpParticle* getParticlesGPU();

    mpParticleIM& getIntermediateData(int i);
    mpParticleIM& getIntermediateData();

    std::mutex& getMutex();

    int updateDataTexture(void *tex, int width, int height);

private:
    typedef ist::combinable<mpPForceCont> mpPForceConbinable;

    mpParticleCont          m_particles;
    mpParticleIMCont        m_imd;
    mpSoAData               m_soa;
    mpCellCont              m_cells;
    u32                     m_id_seed;
    int                     m_num_particles;

    mpColliderPropertiesCont m_collider_properties;
    mpPlaneColliderCont     m_plane_colliders;
    mpSphereColliderCont    m_sphere_colliders;
    mpCapsuleColliderCont   m_capsule_colliders;
    mpBoxColliderCont       m_box_colliders;
    mpForceCont             m_forces;
    bool                    m_has_hithandler;
    bool                    m_has_forcehandler;

    ist::task_group         m_taskgroup;
    std::mutex              m_mutex;
    mpKernelParams          m_kparams;
    mpTempParams            m_tparams;

    mpPForceCont            m_pforce;
    mpPForceConbinable      m_pcombinable;

    int                     m_num_particles_gpu;
    int                     m_num_particles_gpu_prev;
    mpParticleCont          m_particles_gpu;

    int                     m_current;
};
