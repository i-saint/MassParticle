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

extern "C" EXPORT_API void			mpGeneratePointMesh(mpWorld *context, int mi, mpMeshData *mds);
extern "C" EXPORT_API void			mpGenerateCubeMesh(mpWorld *context, int mi, mpMeshData *mds);
extern "C" EXPORT_API int			mpUpdateDataTexture(mpWorld *context, void *tex);
#ifdef mpWithCppScript
extern "C" EXPORT_API int			mpUpdateDataBuffer(mpWorld *context, UnityEngine::ComputeBuffer buf);
#endif // mpWithCppScript

extern "C" EXPORT_API mpWorld*		mpCreateContext();
extern "C" EXPORT_API void			mpDestroyContext(mpWorld *context);
extern "C" EXPORT_API void			mpBeginUpdate(mpWorld *context, float dt);
extern "C" EXPORT_API void			mpEndUpdate(mpWorld *context);
extern "C" EXPORT_API void			mpUpdate(mpWorld *context, float dt);
extern "C" EXPORT_API void			mpClearParticles(mpWorld *context);
extern "C" EXPORT_API void			mpClearCollidersAndForces(mpWorld *context);

extern "C" EXPORT_API ispc::KernelParams mpGetKernelParams(mpWorld *context);
extern "C" EXPORT_API void			mpSetKernelParams(mpWorld *context, ispc::KernelParams *params);

extern "C" EXPORT_API int			mpGetNumHitData(mpWorld *context);
extern "C" EXPORT_API mpHitData*	mpGetHitData(mpWorld *context);
extern "C" EXPORT_API int			mpGetNumParticles(mpWorld *context);
extern "C" EXPORT_API mpParticle*	mpGetParticles(mpWorld *context);
extern "C" EXPORT_API void			mpCopyParticles(mpWorld *context, mpParticle *dst);
extern "C" EXPORT_API void			mpWriteParticles(mpWorld *context, const mpParticle *from);
extern "C" EXPORT_API void			mpPutParticles(mpWorld *context, mpParticle *particles, int32_t num_particles);
extern "C" EXPORT_API void			mpScatterParticlesSphere(mpWorld *context, vec3 *center, float radius, int32_t num, vec3 *vel, float vel_diffuse);
extern "C" EXPORT_API void			mpScatterParticlesBox(mpWorld *context, vec3 *center, vec3 *size, int32_t num, vec3 *vel, float vel_diffuse);
extern "C" EXPORT_API void			mpScatterParticlesSphereTransform(mpWorld *context, mat4 *transform, int32_t num, vec3 *vel, float vel_diffuse);
extern "C" EXPORT_API void			mpScatterParticlesBoxTransform(mpWorld *context, mat4 *transform, int32_t num, vec3 *vel, float vel_diffuse);

extern "C" EXPORT_API void			mpAddSphereCollider(mpWorld *context, mpColliderProperties *props, vec3 *center, float radius);
extern "C" EXPORT_API void			mpAddCapsuleCollider(mpWorld *context, mpColliderProperties *props, vec3 *pos1, vec3 *pos2, float radius);
extern "C" EXPORT_API void			mpAddBoxCollider(mpWorld *context, mpColliderProperties *props, mat4 *transform, vec3 *size);
extern "C" EXPORT_API void			mpAddForce(mpWorld *context, mpForceProperties *p, mat4 *trans);




inline float mpGenRand()
{
	return float((rand() - (RAND_MAX / 2)) * 2) / (float)RAND_MAX;
}
// 0.0f-1.0f
inline float mpGenRand1() { return (float)rand() / RAND_MAX; }

// -1.0f-1.0f
inline float mpGenRand2() { return (mpGenRand1() - 0.5f)*2.0f; }



#endif // MassParticle_h
