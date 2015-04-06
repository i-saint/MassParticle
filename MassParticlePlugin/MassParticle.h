#ifndef MassParticle_h
#define MassParticle_h

//#define mpWithCppScript

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

extern "C" EXPORT_API void			mpUpdateDataTexture(int context, void *tex);
#ifdef mpWithCppScript
extern "C" EXPORT_API void			mpUpdateDataBuffer(int context, UnityEngine::ComputeBuffer buf);
#endif // mpWithCppScript

extern "C" EXPORT_API int			mpCreateContext();
extern "C" EXPORT_API void			mpDestroyContext(int context);
extern "C" EXPORT_API void			mpBeginUpdate(int context, float dt);
extern "C" EXPORT_API void			mpEndUpdate(int context);
extern "C" EXPORT_API void			mpUpdate(int context, float dt);
extern "C" EXPORT_API void			mpCallHandlers(int context);

extern "C" EXPORT_API void			mpClearParticles(int context);
extern "C" EXPORT_API void			mpClearCollidersAndForces(int context);

extern "C" EXPORT_API ispc::KernelParams mpGetKernelParams(int context);
extern "C" EXPORT_API void			mpSetKernelParams(int context, ispc::KernelParams *params);

extern "C" EXPORT_API int			mpGetNumParticles(int context);
extern "C" EXPORT_API mpParticleIM*	mpGetIntermediateData(int context, int nth=-1);
extern "C" EXPORT_API mpParticle*	mpGetParticles(int context);
extern "C" EXPORT_API void			mpCopyParticles(int context, mpParticle *dst);
extern "C" EXPORT_API void			mpWriteParticles(int context, const mpParticle *from);
extern "C" EXPORT_API void			mpPutParticles(int context, mpParticle *particles, i32 num_particles);
extern "C" EXPORT_API void			mpScatterParticlesSphere(int context, vec3 *center, float radius, i32 num, const mpSpawnParams *params);
extern "C" EXPORT_API void			mpScatterParticlesBox(int context, vec3 *center, vec3 *size, i32 num, const mpSpawnParams *params);
extern "C" EXPORT_API void			mpScatterParticlesSphereTransform(int context, mat4 *transform, i32 num, const mpSpawnParams *params);
extern "C" EXPORT_API void			mpScatterParticlesBoxTransform(int context, mat4 *transform, i32 num, const mpSpawnParams *params);

extern "C" EXPORT_API void			mpAddSphereCollider(int context, mpColliderProperties *props, vec3 *center, float radius);
extern "C" EXPORT_API void			mpAddCapsuleCollider(int context, mpColliderProperties *props, vec3 *pos1, vec3 *pos2, float radius);
extern "C" EXPORT_API void			mpAddBoxCollider(int context, mpColliderProperties *props, mat4 *transform, vec3 *center, vec3 *size);
extern "C" EXPORT_API void			mpRemoveCollider(int context, mpColliderProperties *props);
extern "C" EXPORT_API void			mpAddForce(int context, mpForceProperties *p, mat4 *trans);

extern "C" EXPORT_API void			mpScanSphere(int context, mpHitHandler handler, vec3 *center, float radius);
extern "C" EXPORT_API void			mpScanAABB(int context, mpHitHandler handler, vec3 *center, vec3 *extent);
extern "C" EXPORT_API void			mpScanSphereParallel(int context, mpHitHandler handler, vec3 *center, float radius);
extern "C" EXPORT_API void			mpScanAABBParallel(int context, mpHitHandler handler, vec3 *center, vec3 *extent);
extern "C" EXPORT_API void			mpScanAll(int context, mpHitHandler handler);
extern "C" EXPORT_API void			mpScanAllParallel(int context, mpHitHandler handler);

extern "C" EXPORT_API void			mpMoveAll(int context, vec3 *move_amount);


// -1.0f-1.0f
float mpGenRand();

// 0.0f-1.0f
float mpGenRand1();



#endif // MassParticle_h
