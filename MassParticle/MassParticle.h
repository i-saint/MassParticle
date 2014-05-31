#ifndef MassParticle_h
#define MassParticle_h

#include <xnamath.h>
#include "UnityPluginInterface.h"
#include "mpCore_ispc.h"

//external
extern "C" EXPORT_API void          mpReloadShader();

extern "C" EXPORT_API void          mpClearParticles();
extern "C" EXPORT_API void          mpUpdate(float dt);

extern "C" EXPORT_API void          mpSetViewProjectionMatrix(XMFLOAT4X4 view, XMFLOAT4X4 proj);
extern "C" EXPORT_API ispc::KernelParams mpGetKernelParams();
extern "C" EXPORT_API void          mpSetKernelParams(ispc::KernelParams *params);

extern "C" EXPORT_API uint32_t      mpGetNumParticles();
extern "C" EXPORT_API mpParticle*   mpGetParticles();
extern "C" EXPORT_API int32_t       mpPutParticles(mpParticle *particles, int32_t num_particles);
extern "C" EXPORT_API void          mpUpdateParticle(uint32_t index, mpParticleRaw particle);
extern "C" EXPORT_API int32_t       mpScatterParticlesSphererical(XMFLOAT3 center, float radius, int32_t num);
extern "C" EXPORT_API int32_t       mpScatterParticlesBox(XMFLOAT3 center, XMFLOAT3 size, XMFLOAT4X4 transform, int32_t num);

extern "C" EXPORT_API uint32_t      mpAddBoxCollider(int32_t owner, XMFLOAT4X4 transform, XMFLOAT3 size);
extern "C" EXPORT_API uint32_t      mpAddSphereCollider(int32_t owner, XMFLOAT3 center, float radius);
extern "C" EXPORT_API uint32_t      mpAddDirectionalForce(XMFLOAT3 direction, float strength);




inline float mpGenRand()
{
    return float((rand() - (RAND_MAX / 2)) * 2) / (float)RAND_MAX;
}
// 0.0f-1.0f
inline float mpGenRand1() { return (float)rand() / RAND_MAX; }

// -1.0f-1.0f
inline float mpGenRand2() { return (mpGenRand1() - 0.5f)*2.0f; }



#endif // MassParticle_h
