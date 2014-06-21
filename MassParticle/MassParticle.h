#ifndef MassParticle_h
#define MassParticle_h

#include <xnamath.h>
#include "UnityPluginInterface.h"
#include "mpCore_ispc.h"


//external

enum mpForceShape
{
	mpFS_AffectAll,
	mpFS_Sphere,
	mpFS_Box,
};

enum mpForceDirection
{
	mpFD_Directional,
	mpFD_Radial,
	mpFD_Vortex,      // todo:
	mpFD_Spline,      // 
	mpFD_VectorField, //
};

extern "C" EXPORT_API void          mpReloadShader();
extern "C" EXPORT_API void          mpGeneratePointMesh(int mi, mpMeshData *mds);
extern "C" EXPORT_API void          mpGenerateCubeMesh(int mi, mpMeshData *mds);

extern "C" EXPORT_API void          mpUpdate(float dt);
extern "C" EXPORT_API void          mpClearParticles();
extern "C" EXPORT_API void          mpClearCollidersAndForces();

extern "C" EXPORT_API void          mpSetViewProjectionMatrix(XMFLOAT4X4 view, XMFLOAT4X4 proj, XMFLOAT3 camerapos);
extern "C" EXPORT_API ispc::KernelParams mpGetKernelParams();
extern "C" EXPORT_API void          mpSetKernelParams(ispc::KernelParams *params);

extern "C" EXPORT_API uint32_t      mpGetNumParticles();
extern "C" EXPORT_API mpParticle*   mpGetParticles();
extern "C" EXPORT_API void          mpCopyParticles(mpParticle *dst);
extern "C" EXPORT_API void          mpWriteParticles(const mpParticle *from);
extern "C" EXPORT_API void          mpPutParticles(mpParticle *particles, int32_t num_particles);
extern "C" EXPORT_API void          mpScatterParticlesSphere(XMFLOAT3 center, float radius, int32_t num, XMFLOAT3 vel, float vel_diffuse);
extern "C" EXPORT_API void          mpScatterParticlesBox(XMFLOAT3 center, XMFLOAT3 size, int32_t num, XMFLOAT3 vel, float vel_diffuse);
extern "C" EXPORT_API void          mpScatterParticlesSphereTransform(XMFLOAT4X4 transform, int32_t num, XMFLOAT3 vel, float vel_diffuse);
extern "C" EXPORT_API void          mpScatterParticlesBoxTransform(XMFLOAT4X4 transform, int32_t num, XMFLOAT3 vel, float vel_diffuse);
extern "C" EXPORT_API void          mpAddBoxCollider(int32_t owner, XMFLOAT4X4 transform, XMFLOAT3 size);
extern "C" EXPORT_API void          mpAddSphereCollider(int32_t owner, XMFLOAT3 center, float radius);
extern "C" EXPORT_API void          mpAddForce(int force_shape, XMFLOAT4X4 trans, int force_direction, ispc::ForceParams p);




inline float mpGenRand()
{
	return float((rand() - (RAND_MAX / 2)) * 2) / (float)RAND_MAX;
}
// 0.0f-1.0f
inline float mpGenRand1() { return (float)rand() / RAND_MAX; }

// -1.0f-1.0f
inline float mpGenRand2() { return (mpGenRand1() - 0.5f)*2.0f; }



#endif // MassParticle_h
