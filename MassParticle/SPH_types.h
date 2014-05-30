#ifndef _SPH_types_h_
#define _SPH_types_h_

#include <vector>
#include "SPH_const.h"
#include "SPH_core_ispc.h"
#include "SoA.h"
#include "UnityPluginInterface.h"
#include <xnamath.h>
#include <mutex>

typedef short           int16;
typedef unsigned short  uint16;
typedef int             int32;
typedef unsigned int    uint32;
typedef float           float32;

using ist::simdvec4;
using ist::simdvec8;
using ist::soavec24;
using ist::soavec34;
using ist::soavec44;

typedef ispc::Particle_SOA8 sphParticleSOA8;
typedef ispc::GridData sphGridData;

#define set_xyz(v, _x, _y, _z)  v.x =_x; v.y =_y; v.z =_z;
#define set_nxyz(v, _x, _y, _z) v.nx=_x; v.ny=_y; v.nz=_z;
#define set_vxyz(v, _x, _y, _z) v.vx=_x; v.vy=_y; v.vz=_z;


struct peParticle
{
    simdvec4 position;
    simdvec4 velocity;
    union {
        struct {
            float32 density;
            uint32 hash;
            int32 hit;
            float32 lifetime;
        } params;
        simdvec4 paramsv;
    };
};
struct peParticleRaw
{
    char data[sizeof(peParticle)];
};

struct peWorld
{
    peParticle particles[SPH_MAX_PARTICLE_NUM];
    sphParticleSOA8 particles_soa[SPH_MAX_PARTICLE_NUM];
    sphGridData cell[SPH_GRID_DIV][SPH_GRID_DIV];
    uint32_t num_active_particles;
    float32 particle_lifetime;

    std::vector<ispc::SphereCollider>   collision_spheres;
    std::vector<ispc::PlaneCollider>    collision_planes;
    std::vector<ispc::BoxCollider>      collision_boxes;

    std::vector<ispc::PointForce>       force_point;
    std::vector<ispc::DirectionalForce> force_directional;
    std::vector<ispc::BoxForce>         force_box;

    std::mutex m_mutex;

    peWorld();
    void clearParticles();
    void clearCollidersAndForces();
    void addParticles(peParticle *p, uint32_t num_particles);
    void update(float32 dt);
};

// external
extern "C" EXPORT_API peWorld*      peCreateContext(uint32_t max_particles);
extern "C" EXPORT_API void          peDeleteContext(peWorld *ctx);
extern "C" EXPORT_API void          peResetState(peWorld *ctx);
extern "C" EXPORT_API void          peClearParticles();
extern "C" EXPORT_API void          peUpdate(float dt);

extern "C" EXPORT_API void          peSetViewProjectionMatrix(XMFLOAT4X4 view, XMFLOAT4X4 proj);

extern "C" EXPORT_API uint32_t      peGetNumParticles(peWorld *ctx);
extern "C" EXPORT_API peParticle*   peGetParticles(peWorld *ctx);
extern "C" EXPORT_API uint32_t      pePutParticles(peWorld *ctx, peParticle *particles, uint32_t num_particles);
extern "C" EXPORT_API void          peUpdateParticle(peWorld *ctx, uint32_t index, peParticleRaw particle);
extern "C" EXPORT_API uint32_t      peScatterParticlesSphererical(peWorld *ctx, XMFLOAT3 center, float radius, uint32 num);

extern "C" EXPORT_API uint32_t      peAddBoxCollider(peWorld *ctx, XMFLOAT4X4 transform, XMFLOAT3 size);
extern "C" EXPORT_API uint32_t      peAddSphereCollider(peWorld *ctx, XMFLOAT3 center, float radius);
extern "C" EXPORT_API uint32_t      peAddDirectionalForce(peWorld *ctx, XMFLOAT3 direction, float strength);



#endif // _SPH_types_h_
