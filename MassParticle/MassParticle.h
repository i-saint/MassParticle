#ifndef MassParticle_h
#define MassParticle_h

#include <xnamath.h>
#include "UnityPluginInterface.h"

//external
extern "C" EXPORT_API mpWorld*      mpCreateContext(uint32_t max_particles);
extern "C" EXPORT_API void          mpDeleteContext(mpWorld *ctx);
extern "C" EXPORT_API void          mpResetState(mpWorld *ctx);
extern "C" EXPORT_API void          mpClearParticles();
extern "C" EXPORT_API void          mpUpdate(float dt);

extern "C" EXPORT_API void          mpSetViewProjectionMatrix(XMFLOAT4X4 view, XMFLOAT4X4 proj);

extern "C" EXPORT_API uint32_t      mpGetNumParticles(mpWorld *ctx);
extern "C" EXPORT_API mpParticle*   mpGetParticles(mpWorld *ctx);
extern "C" EXPORT_API uint32_t      mpPutParticles(mpWorld *ctx, mpParticle *particles, uint32_t num_particles);
extern "C" EXPORT_API void          mpUpdateParticle(mpWorld *ctx, uint32_t index, mpParticleRaw particle);
extern "C" EXPORT_API uint32_t      mpScatterParticlesSphererical(mpWorld *ctx, XMFLOAT3 center, float radius, uint32 num);

extern "C" EXPORT_API uint32_t      mpAddBoxCollider(mpWorld *ctx, XMFLOAT4X4 transform, XMFLOAT3 size);
extern "C" EXPORT_API uint32_t      mpAddSphereCollider(mpWorld *ctx, XMFLOAT3 center, float radius);
extern "C" EXPORT_API uint32_t      mpAddDirectionalForce(mpWorld *ctx, XMFLOAT3 direction, float strength);

// internal
bool mpInitializeDevice(void *dev);
void mpFinalize();
void mpCleanupDevice();
void mpRender();



inline float mpGenRand()
{
    return float((rand() - (RAND_MAX / 2)) * 2) / (float)RAND_MAX;
}
// 0.0f-1.0f
inline float mpGenRand1() { return (float)rand() / RAND_MAX; }

// -1.0f-1.0f
inline float mpGenRand2() { return (mpGenRand1() - 0.5f)*2.0f; }


class mpPerspectiveCamera
{
private:
    XMMATRIX m_viewproj;
    XMMATRIX m_view;
    XMMATRIX m_proj;
    XMVECTOR m_eye;
    XMVECTOR m_focus;
    XMVECTOR m_up;
    FLOAT m_fovy;
    FLOAT m_aspect;
    FLOAT m_near;
    FLOAT m_far;

public:
    mpPerspectiveCamera() {}

    const XMMATRIX& getViewProjectionMatrix() const { return m_viewproj; }
    const XMMATRIX& getViewMatrix() const           { return m_view; }
    const XMMATRIX& getProjectionMatrix() const     { return m_proj; }
    XMVECTOR getEye() const     { return m_eye; }
    XMVECTOR getFocus() const   { return m_focus; }
    XMVECTOR getUp() const      { return m_up; }
    FLOAT getFovy() const       { return m_fovy; }
    FLOAT getAspect() const     { return m_aspect; }
    FLOAT getNear() const       { return m_near; }
    FLOAT getFar() const        { return m_far; }

    void setEye(XMVECTOR v) { m_eye = v; }

    void setView(XMVECTOR eye, XMVECTOR focus, XMVECTOR up)
    {
        m_eye = eye;
        m_focus = focus;
        m_up = up;
    }

    void setProjection(FLOAT fovy, FLOAT aspect, FLOAT _near, FLOAT _far)
    {
        m_fovy = fovy;
        m_aspect = aspect;
        m_near = _near;
        m_far = _far;
    }

    void updateMatrix()
    {
        m_view = XMMatrixLookAtLH(m_eye, m_focus, m_up);
        m_proj = XMMatrixPerspectiveFovLH(m_fovy, m_aspect, m_near, m_far);
        m_viewproj = XMMatrixMultiply(m_view, m_proj);
    }

    void forceSetMatrix(const XMFLOAT4X4 &view, const XMFLOAT4X4 &proj)
    {
        m_view = (FLOAT*)&view;
        m_proj = (FLOAT*)&proj;
        m_viewproj = XMMatrixMultiply(m_view, m_proj);
        m_eye = m_view.r[3];
    }
};

class PerformanceCounter
{
private:
    LARGE_INTEGER m_start;
    LARGE_INTEGER m_end;

public:
    PerformanceCounter()
    {
        reset();
    }

    void reset()
    {
        ::QueryPerformanceCounter(&m_start);
    }

    float getElapsedSecond()
    {
        LARGE_INTEGER freq;
        ::QueryPerformanceCounter(&m_end);
        ::QueryPerformanceFrequency(&freq);
        return ((float)(m_end.QuadPart - m_start.QuadPart) / (float)freq.QuadPart);
    }

    float getElapsedMillisecond()
    {
        return getElapsedSecond()*1000.0f;
    }
};

#endif // MassParticle_h
