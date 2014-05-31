#ifndef _SPH_types_h_
#define _SPH_types_h_

#include <vector>
#include "mpConst.h"
#include "mpCore_ispc.h"
#include "SoA.h"
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

enum mpSolverType
{
    mpSolver_Impulse,
    mpSolver_SPH,
    mpSolver_SPHEst,
};

struct mpKernelParams : ispc::KernelParams
{
    mpKernelParams()
    {
        SolverType = mpSolver_Impulse;
        LifeTime = 3600.0f;
        Timestep = 0.01f;
        Decelerate = 0.995f;
        PressureStiffness = 500.0f;
        WallStiffness = 1500.0f;
        XScaler = 1.0f;
        YScaler = 1.0f;
        ZScaler = 1.0f;

        SPHRestDensity = 1000.0f;
        SPHParticleMass = 0.002f;
        SPHViscosity = 0.1f;
    }
};


struct mpParticle
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
struct mpParticleRaw
{
    char data[sizeof(mpParticle)];
};


class mpCamera
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
    mpCamera() {}

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

        {
            // see CalculateDeviceProjectionMatrix()
            float *v = (float*)&m_proj;
            v[4*0 + 2] = v[4*0 + 2] * 0.5f + v[4*0+3] * 0.5f;
            v[4*1 + 2] = v[4*1 + 2] * 0.5f + v[4*1+3] * 0.5f;
            v[4*2 + 2] = v[4*2 + 2] * 0.5f + v[4*2+3] * 0.5f;
            v[4*3 + 2] = v[4*3 + 2] * 0.5f + v[4*3+3] * 0.5f;
        }

        m_viewproj = XMMatrixMultiply(m_view, m_proj);
        m_eye = m_view.r[3];
    }
};

class mpPerformanceCounter
{
private:
    LARGE_INTEGER m_start;
    LARGE_INTEGER m_end;

public:
    mpPerformanceCounter()
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

class mpRenderer
{
public:
    virtual ~mpRenderer() {}
    virtual void render() = 0;
    virtual void reloadShader() = 0;
};
class mpWorld;
mpRenderer* mpCreateRendererD3D9(void *dev, mpWorld &world);
mpRenderer* mpCreateRendererD3D11(void *dev, mpWorld &world);
mpRenderer* mpCreateRendererOpenGL(void *dev, mpWorld &world);


class mpWorld
{
public:
    // todo: refactoring
    mpParticle particles[mpMaxParticleNum];
    sphParticleSOA8 particles_soa[mpMaxParticleNum];
    sphGridData cell[mpWorldDivNum][mpWorldDivNum];
    uint32_t m_num_active_particles;

    std::vector<ispc::SphereCollider>   collision_spheres;
    std::vector<ispc::PlaneCollider>    collision_planes;
    std::vector<ispc::BoxCollider>      collision_boxes;

    std::vector<ispc::PointForce>       force_point;
    std::vector<ispc::DirectionalForce> force_directional;
    std::vector<ispc::BoxForce>         force_box;

    std::mutex m_mutex;
    mpKernelParams m_params;
    mpCamera m_camera;
    mpRenderer *m_renderer;

public:
    mpWorld();
    void clearParticles();
    void clearCollidersAndForces();
    void addParticles(mpParticle *p, uint32_t num_particles);
    void update(float32 dt);
};


#endif // _SPH_types_h_
