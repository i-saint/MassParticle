#ifndef MassParticle_h
#define MassParticle_h

#define mpWithCppScript

#ifdef mpWithCppScript
#include "cpsBinder.h"
#include "cpsUnityEngine.h"
#endif // mpWithCppScript

#include "UnityPluginInterface.h"
#include "mpCore_ispc.h"
#include "mpTypes.h"


//external

enum mpForceShape
{
	mpFS_AffectAll,
	mpFS_Sphere,
	mpFS_Capsule,
	mpFS_Box,
};

enum mpForceDirection
{
	mpFD_Directional,
	mpFD_Radial,
	mpFD_RadialCapsule,
	mpFD_Vortex,      // todo:
	mpFD_Spline,      // 
	mpFD_VectorField, //
};

extern "C" EXPORT_API void			mpGeneratePointMesh(int mi, mpMeshData *mds);
extern "C" EXPORT_API void			mpGenerateCubeMesh(int mi, mpMeshData *mds);
extern "C" EXPORT_API int			mpUpdateDataTexture(mpWorld *context, void *tex);
#ifdef mpWithCppScript
extern "C" EXPORT_API int			mpUpdateDataBuffer(mpWorld *context, UnityEngine::ComputeBuffer buf);
#endif // mpWithCppScript

extern "C" EXPORT_API mpWorld*		mpCreateContext();
extern "C" EXPORT_API void			mpDestroyContext(mpWorld *c);
extern "C" EXPORT_API void			mpMakeCurrent(mpWorld *c);
extern "C" EXPORT_API void			mpBeginUpdate(float dt);
extern "C" EXPORT_API void			mpEndUpdate();
extern "C" EXPORT_API void			mpUpdate(float dt);
extern "C" EXPORT_API void			mpClearParticles();
extern "C" EXPORT_API void			mpClearCollidersAndForces();

extern "C" EXPORT_API ispc::KernelParams mpGetKernelParams();
extern "C" EXPORT_API void			mpSetKernelParams(ispc::KernelParams *params);

extern "C" EXPORT_API int			mpGetNumHitData();
extern "C" EXPORT_API mpHitData*	mpGetHitData();
extern "C" EXPORT_API int			mpGetNumParticles();
extern "C" EXPORT_API mpParticle*	mpGetParticles();
extern "C" EXPORT_API void			mpCopyParticles(mpParticle *dst);
extern "C" EXPORT_API void			mpWriteParticles(const mpParticle *from);
extern "C" EXPORT_API void			mpPutParticles(mpParticle *particles, int32_t num_particles);
extern "C" EXPORT_API void			mpScatterParticlesSphere(vec3 *center, float radius, int32_t num, vec3 *vel, float vel_diffuse);
extern "C" EXPORT_API void			mpScatterParticlesBox(vec3 *center, vec3 *size, int32_t num, vec3 *vel, float vel_diffuse);
extern "C" EXPORT_API void			mpScatterParticlesSphereTransform(mat4 *transform, int32_t num, vec3 *vel, float vel_diffuse);
extern "C" EXPORT_API void			mpScatterParticlesBoxTransform(mat4 *transform, int32_t num, vec3 *vel, float vel_diffuse);

extern "C" EXPORT_API void			mpAddSphereCollider(mpColliderProperties *props, vec3 *center, float radius);
extern "C" EXPORT_API void			mpAddCapsuleCollider(mpColliderProperties *props, vec3 *pos1, vec3 *pos2, float radius);
extern "C" EXPORT_API void			mpAddBoxCollider(mpColliderProperties *props, mat4 *transform, vec3 *size);
extern "C" EXPORT_API void			mpAddForce(mpForceProperties *p, mat4 *trans);




inline float mpGenRand()
{
	return float((rand() - (RAND_MAX / 2)) * 2) / (float)RAND_MAX;
}
// 0.0f-1.0f
inline float mpGenRand1() { return (float)rand() / RAND_MAX; }

// -1.0f-1.0f
inline float mpGenRand2() { return (mpGenRand1() - 0.5f)*2.0f; }



#endif // MassParticle_h
