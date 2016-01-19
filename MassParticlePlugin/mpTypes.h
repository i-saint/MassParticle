#ifndef _SPH_types_h_
#define _SPH_types_h_

#define GLM_FORCE_RADIANS
#ifdef _WIN64
#   define GLM_FORCE_SSE4
#endif

#include <cstdint>
#include <vector>
#include <mutex>
#include <glm/glm.hpp>
#include <glm/gtx/simd_vec4.hpp>
#include <glm/gtx/simd_mat4.hpp>
#include <tbb/tbb.h>
#include <tbb/combinable.h>
#include "mpCore_ispc.h"
#include "SoA.h"

#ifdef max
#undef max
#undef min
#endif // max

#ifdef _MSC_VER
#   define mpCountof(exp) _countof(exp)
#else
#   define mpCountof(exp) (sizeof(exp)/sizeof(exp[0]))
#endif

typedef int16_t         i16;
typedef uint16_t        u16;
typedef int32_t         i32;
typedef uint32_t        u32;
typedef float           f32;

typedef __m128 simd128;

using ist::vec4soa2;
using ist::vec4soa3;
using ist::vec4soa4;

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::mat3;
using glm::mat4;
typedef glm::simdVec4 simdvec4;
typedef glm::simdMat4 simdmat4;

typedef ispc::Context                   mpKernelContext;
typedef ispc::Cell                      mpCell;

typedef ispc::Plane                     mpPlane;
typedef ispc::Sphere                    mpSphere;
typedef ispc::Capsule                   mpCapsule;
typedef ispc::Box                       mpBox;

typedef ispc::ColliderProperties        mpColliderProperties;
typedef ispc::PlaneCollider             mpPlaneCollider;
typedef ispc::SphereCollider            mpSphereCollider;
typedef ispc::CapsuleCollider           mpCapsuleCollider;
typedef ispc::BoxCollider               mpBoxCollider;

typedef ispc::ForceProperties           mpForceProperties;
typedef ispc::Force                     mpForce;

namespace glm {
    inline float length_sq(const vec2 &v) { return dot(v, v); }
    inline float length_sq(const vec3 &v) { return dot(v, v); }
    inline float length_sq(const vec4 &v) { return dot(v, v); }
} // namespace glm

template<class IntType>
inline IntType ceildiv(IntType a, IntType b)
{
    return a / b + (a%b == 0 ? 0 : 1);
}

inline int msb(int a)
{
#ifdef _MSC_VER
    ULONG r;
    _BitScanReverse(&r, (ULONG)a);
    return (int)r;
#else  // _MSC_VER
    return a == 0 ? 0 : 31 - __builtin_clz(a);
#endif // _MSC_VER
}

// -1.0f-1.0f
float mpGenRand();

// 0.0f-1.0f
float mpGenRand1();


struct mpKernelParams : ispc::KernelParams
{
    mpKernelParams();
};

struct mpTempParams
{
    vec3 cell_size;
    vec3 rcp_cell_size;
    vec3 world_bounds_bl;
    vec3 world_bounds_ur;
    ivec3 world_div_bits;
};


struct mpParticle
{
    union {
        simd128 position;
        struct {
            vec3 position3f;
            u32 id;
        };
    };
    simd128 velocity;
    union {
        u32 hash;
        f32 density;
    };
    f32 lifetime;
    struct {
        u16 hit;
        u16 hit_prev;
    };
    int userdata;

    mpParticle() {}
    mpParticle(const mpParticle &v) { *this = v; }
    mpParticle& operator=(const mpParticle &v)
    {
        simd128 *dst = (simd128*)this;
        simd128 *src = (simd128*)&v;
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        return *this;
    }
};

// intermediate data
struct mpParticleIM
{
    simd128 accel;
};

struct mpParticleForce
{
    int num_hits;
    int pad[3];
    simd128 position;
    simd128 force;

    mpParticleForce() { clear(); }
    void clear() { memset(this, 0, sizeof(*this)); }
    mpParticleForce& operator=(const mpParticleForce &v)
    {
        simd128 *dst = (simd128*)this;
        simd128 *src = (simd128*)&v;
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        return *this;
    }
};
typedef void(__stdcall *mpHitHandler)(mpParticle *p);
typedef void(__stdcall *mpForceHandler)(mpParticleForce *p);

struct mpSpawnParams
{
    vec3 velocity_base;
    float velocity_random_diffuse;
    float lifetime;
    float lifetime_random_diffuse;
    int userdata;
    mpHitHandler handler;
};

void* mpAlignedAlloc(size_t size, size_t align);
void mpAlignedFree(void *p);


template<typename T, int Align=32>
class mpAlignedAllocator {
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    template<typename U> struct rebind { typedef mpAlignedAllocator<U> other; };
public:
    mpAlignedAllocator() {}
    mpAlignedAllocator(const mpAlignedAllocator&) {}
    template<typename U> mpAlignedAllocator(const mpAlignedAllocator<U>&) {}
    ~mpAlignedAllocator() {}
    pointer address(reference r) { return &r; }
    const_pointer address(const_reference r) { return &r; }
    pointer allocate(size_type cnt, const void *p = nullptr) { return (pointer)mpAlignedAlloc(cnt * sizeof(T), Align); }
    void deallocate(pointer p, size_type) { mpAlignedFree(p); }
    size_type max_size() const { return std::numeric_limits<size_type>::max() / sizeof(T); }
    void construct(pointer p, const T& t) { new(p)T(t); }
    void destroy(pointer p) { p->~T(); }
    bool operator==(mpAlignedAllocator const&) { return true; }
    bool operator!=(mpAlignedAllocator const& a) { return !operator==(a); }
};
template<class T, typename Alloc> inline bool operator==(const mpAlignedAllocator<T>& l, const mpAlignedAllocator<T>& r) { return (l.equals(r)); }
template<class T, typename Alloc> inline bool operator!=(const mpAlignedAllocator<T>& l, const mpAlignedAllocator<T>& r) { return (!(l == r)); }

typedef std::vector<float, mpAlignedAllocator<float> >                          mpFloatArray;
typedef std::vector<int, mpAlignedAllocator<int> >                              mpIntArray;
typedef std::vector<mpParticle, mpAlignedAllocator<mpParticle> >                mpParticleCont;
typedef std::vector<mpParticleIM, mpAlignedAllocator<mpParticleIM> >            mpParticleIMCont;
typedef std::vector<mpParticleForce, mpAlignedAllocator<mpParticleForce> >      mpPForceCont;
typedef tbb::combinable<mpPForceCont>                                           mpPForceConbinable;
typedef std::vector<mpCell, mpAlignedAllocator<mpCell> >                            mpCellCont;
typedef std::vector<mpColliderProperties*, mpAlignedAllocator<mpPlaneCollider> >    mpColliderPropertiesCont;
typedef std::vector<mpPlaneCollider, mpAlignedAllocator<mpPlaneCollider> >      mpPlaneColliderCont;
typedef std::vector<mpSphereCollider, mpAlignedAllocator<mpSphereCollider> >    mpSphereColliderCont;
typedef std::vector<mpCapsuleCollider, mpAlignedAllocator<mpCapsuleCollider> >  mpCapsuleColliderCont;
typedef std::vector<mpBoxCollider, mpAlignedAllocator<mpBoxCollider> >          mpBoxColliderCont;
typedef std::vector<mpForce, mpAlignedAllocator<mpForce> >                      mpForceCont;

struct mpSoAData
{
    struct Reference
    {
        vec3 pos;
        vec3 vel;
        vec3 acl;
        float speed;
        float density;
        float affection;
        int hit;
    };

    mpFloatArray pos_x;
    mpFloatArray pos_y;
    mpFloatArray pos_z;
    mpFloatArray vel_x;
    mpFloatArray vel_y;
    mpFloatArray vel_z;
    mpFloatArray acl_x;
    mpFloatArray acl_y;
    mpFloatArray acl_z;
    mpFloatArray speed;
    mpFloatArray density;
    mpFloatArray affection;
    mpIntArray hit;

    void resize(size_t n);
};

class mpWorld;

class mpRenderer
{
public:
    virtual ~mpRenderer() {}
    virtual void updateDataTexture(void *tex, int width, int height, const void *data, size_t data_size) = 0;
};
mpRenderer* mpCreateRendererD3D9(void *dev);
mpRenderer* mpCreateRendererD3D11(void *dev);
mpRenderer* mpCreateRendererOpenGL(void *dev);

const int mpDataTextureWidth = 3072;
const int mpDataTextureHeight = 256;
const int mpTexelsEachParticle = 3;
const int mpParticlesEachLine = mpDataTextureWidth / mpTexelsEachParticle;



#endif // _SPH_types_h_
