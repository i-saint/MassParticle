#ifndef _SPH_types_h_
#define _SPH_types_h_

#define GLM_FORCE_RADIANS

#include <vector>
#include <mutex>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/simd_vec4.hpp>
#include <glm/gtx/simd_mat4.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

typedef short           int16;
typedef unsigned short  uint16;
typedef int             int32;
typedef unsigned int    uint32;
typedef float           float32;

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

typedef ispc::Particle_SOA8			mpParticleSOA8;
typedef ispc::ParticleIMData_SOA8	mpParticleIMDSOA8;
typedef ispc::Cell					mpCell;

typedef ispc::Plane					mpPlane;
typedef ispc::Sphere				mpSphere;
typedef ispc::Capsule				mpCapsule;
typedef ispc::Box					mpBox;

typedef ispc::ColliderProperties	mpColliderProperties;
typedef ispc::PlaneCollider			mpPlaneCollider;
typedef ispc::SphereCollider		mpSphereCollider;
typedef ispc::CapsuleCollider		mpCapsuleCollider;
typedef ispc::BoxCollider			mpBoxCollider;

typedef ispc::ForceProperties		mpForceProperties;
typedef ispc::Force					mpForce;

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


enum mpSolverType
{
	mpSolver_Impulse,
	mpSolver_SPH,
	mpSolver_SPHEst,
};

enum mpRendererType
{
	mpRenderer_Cube,
	mpRenderer_Line,
	mpRenderer_Sprite,
};

struct mpKernelParams : ispc::KernelParams
{
	mpKernelParams();
};

struct mpTempParams
{
	vec3 rcp_cell_size;
	vec3 world_bounds_bl;
	vec3 world_bounds_ur;
	ivec3 world_div_bits;
};

struct mpMeshData
{
	int *indices;
	vec3 *vertices;
	vec3 *normals;
	vec2 *texcoords;
};


struct mpParticle
{
	simd128 position;
	simd128 velocity;
	float32 density;
	union {
		uint32 hash;
		int32 hit_prev;
	};
	int32 hit;
	float32 lifetime;

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

struct mpHitData
{
	simd128 position;
	simd128 velocity;
	int num_hits;
	int pad[3];

	mpHitData() { clear(); }
	void clear() { memset(this, 0, sizeof(*this));  }
	mpHitData& operator=(const mpHitData &v)
	{
		simd128 *dst = (simd128*)this;
		simd128 *src = (simd128*)&v;
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		return *this;
	}
};


inline void* mpAlignedAlloc(size_t size, size_t align)
{
#ifdef _MSC_VER
	return _aligned_malloc(size, align);
#elif defined(__APPLE__)
	return malloc(size);
#else  // _MSC_VER
	return memalign(align, size);
#endif // _MSC_VER
}

inline void mpAlignedFree(void *p)
{
#ifdef _MSC_VER
	_aligned_free(p);
#else  // _MSC_VER
	free(p);
#endif // _MSC_VER
}


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

typedef std::vector<mpParticle, mpAlignedAllocator<mpParticle> >				mpParticleCont;
typedef std::vector<mpParticleSOA8, mpAlignedAllocator<mpParticleSOA8> >		mpParticleSOACont;
typedef std::vector<mpParticleIMDSOA8, mpAlignedAllocator<mpParticleIMDSOA8> >	mpParticleIMDSOACont;
typedef std::vector<mpCell, mpAlignedAllocator<mpCell> >						mpCellCont;
typedef std::vector<mpPlaneCollider, mpAlignedAllocator<mpPlaneCollider> >		mpPlaneColliderCont;
typedef std::vector<mpSphereCollider, mpAlignedAllocator<mpSphereCollider> >	mpSphereColliderCont;
typedef std::vector<mpCapsuleCollider, mpAlignedAllocator<mpCapsuleCollider> >	mpCapsuleColliderCont;
typedef std::vector<mpBoxCollider, mpAlignedAllocator<mpBoxCollider> >			mpBoxColliderCont;
typedef std::vector<mpForce, mpAlignedAllocator<mpForce> >						mpForceCont;
typedef std::vector<mpHitData, mpAlignedAllocator<mpHitData> >					mpHitDataCont;
typedef tbb::combinable<mpHitDataCont>											mpHitDataConbinable;
typedef std::vector<vec4, mpAlignedAllocator<vec4> >							mpTrailCont;

class mpWorld;

class mpRenderer
{
public:
	virtual ~mpRenderer() {}
	virtual void render(mpWorld &world) = 0;
	virtual void reloadShader() = 0;
	virtual void updateDataTexture(void *tex, const void *data, size_t data_size) = 0;
};
mpRenderer* mpCreateRendererD3D9(void *dev);
mpRenderer* mpCreateRendererD3D11(void *dev);
mpRenderer* mpCreateRendererOpenGL(void *dev);

const int mpDataTextureWidth = 3072;
const int mpDataTextureHeight = 2048;
const int mpTexelsEachParticle = 3;
const int mpParticlesEachLine = mpDataTextureWidth / mpTexelsEachParticle;

class mpWorld
{
public:

	mpWorld();
	~mpWorld();
	void onEnable();
	void onDisable();
	void beginUpdate(float dt);
	void endUpdate();
	void update(float dt);
	void render();

	void addParticles(mpParticle *p, int num);
	void addPlaneColliders(mpPlaneCollider *col, int num);
	void addSphereColliders(mpSphereCollider *col, int num);
	void addCapsuleColliders(mpCapsuleCollider *col, int num);
	void addBoxColliders(mpBoxCollider *col, int num);
	void addForces(mpForce *force, int num);
	void clearParticles();
	void clearCollidersAndForces();

	const mpKernelParams& getKernelParams() const { return m_kparams;  }
	mpTempParams& getTempParams() { return m_tparams; }
	void setKernelParams(const mpKernelParams &v) { m_kparams = v; }
	void getViewProjection(mat4 &out_mat, vec3 &out_camerapos);
	void setViewProjection(const mat4 &mat, const vec3 &camerapos);

	int			getNumHitData() const;
	mpHitData*	getHitData();
	int			getNumParticles() const	{ return m_num_particles; }
	mpParticle*	getParticles()			{ return m_particles.empty() ? nullptr : &m_particles[0]; }
	int			getNumParticlesGPU() const	{ return m_num_particles_gpu; }
	mpParticle*	getParticlesGPU()			{ return m_particles_gpu.empty() ? nullptr : &m_particles_gpu[0]; }

	std::mutex& getMutex() { return m_mutex;  }

	void generatePointMesh(int mi, mpMeshData *mds);
	void generateCubeMesh(int mi, mpMeshData *mds);
	int updateDataTexture(void *tex);

private:
	mpParticleCont			m_particles;
	mpParticleSOACont		m_particles_soa;
	mpParticleIMDSOACont	m_imd_soa;
	mpCellCont				m_cells;
	int						m_num_particles;

	mpPlaneColliderCont		m_plane_colliders;
	mpSphereColliderCont	m_sphere_colliders;
	mpCapsuleColliderCont	m_capsule_colliders;
	mpBoxColliderCont		m_box_colliders;
	mpForceCont				m_forces;

	tbb::task				*m_update_task;
	std::mutex				m_mutex;
	mpKernelParams			m_kparams;
	mpTempParams			m_tparams;

	mat4					m_viewproj;
	vec3					m_camerapos;

	bool					m_hitdata_needs_update;
	mpHitDataCont			m_hitdata;
	mpHitDataConbinable		m_hitdata_work;

	int						m_num_particles_gpu;
	int						m_num_particles_gpu_prev;
	mpParticleCont			m_particles_gpu;

	bool					m_trail_enabled;
	int						m_trail_length;
	tbb::task				*m_trail_update_task;
	mpTrailCont				m_trail;
};

inline int mpMSB(int a)
{
#ifdef _MSC_VER
	ULONG r;
	_BitScanReverse(&r, (ULONG)a);
	return (int)r;
#else  // _MSC_VER
	return __builtin_clz(a);
#endif // _MSC_VER
}


#endif // _SPH_types_h_
