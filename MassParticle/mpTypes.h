#ifndef _SPH_types_h_
#define _SPH_types_h_

#include <vector>
#include "mpCore_ispc.h"
#include "SoA.h"
#include <xnamath.h>
#include <mutex>

#ifdef max
#undef max
#undef min
#endif // max

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

typedef ispc::Particle_SOA8			mpParticleSOA8;
typedef ispc::ParticleIMData_SOA8	mpParticleIMDSOA8;
typedef ispc::GridData				mpGridData;
typedef ispc::SphereCollider		mpSphereCollider;
typedef ispc::PlaneCollider			mpPlaneCollider;
typedef ispc::BoxCollider			mpBoxCollider;
typedef ispc::Force					mpForce;

#define set_xyz(v, _x, _y, _z)  v.x =_x; v.y =_y; v.z =_z;
#define set_nxyz(v, _x, _y, _z) v.nx=_x; v.ny=_y; v.nz=_z;
#define set_vxyz(v, _x, _y, _z) v.vx=_x; v.vy=_y; v.vz=_z;

inline XMFLOAT3 operator+(const XMFLOAT3 &l, const XMFLOAT3 &r) { return XMFLOAT3(l.x+r.x, l.y+r.y, l.z+r.z); }
inline XMFLOAT3 operator-(const XMFLOAT3 &l, const XMFLOAT3 &r) { return XMFLOAT3(l.x-r.x, l.y-r.y, l.z-r.z); }
inline XMFLOAT3 operator*(const XMFLOAT3 &l, const XMFLOAT3 &r) { return XMFLOAT3(l.x*r.x, l.y*r.y, l.z*r.z); }
inline XMFLOAT3 operator*(const XMFLOAT3 &l, float r) { return XMFLOAT3(l.x*r, l.y*r, l.z*r); }
inline XMFLOAT3 operator/(const XMFLOAT3 &l, const XMFLOAT3 &r) { return XMFLOAT3(l.x/r.x, l.y/r.y, l.z/r.z); }
inline XMFLOAT3 operator/(const XMFLOAT3 &l, float r) { return XMFLOAT3(l.x / r, l.y / r, l.z / r); }

enum mpSolverType
{
	mpSolver_Impulse,
	mpSolver_SPH,
	mpSolver_SPHEst,
	mpSolver_NoInteraction,
};

struct mpKernelParams : ispc::KernelParams
{
	mpKernelParams();
};

struct mpTempParams
{
	XMFLOAT3 RcpCellSize;
	XMFLOAT3 WorldBBBL;
	XMFLOAT3 WorldBBUR;
	int WorldDivBits_x;
	int WorldDivBits_y;
	int WorldDivBits_z;
};


struct mpParticle
{
	simdvec4 position;
	simdvec4 velocity;
	union {
		struct {
			float32 density;
			union {
				uint32 hash;
				int32 hit_prev;
			};
			int32 hit;
			float32 lifetime;
		} params;
		simdvec4 paramsv;
	};
};


class mpPerformanceCounter
{
public:
	mpPerformanceCounter();
	void reset();
	float getElapsedSecond();
	float getElapsedMillisecond();

private:
	LARGE_INTEGER m_start;
	LARGE_INTEGER m_end;
};

template<typename T>
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
	pointer allocate(size_type cnt, const void *p = nullptr) { p; return (pointer)_aligned_malloc(cnt * sizeof(T), 16); }
	void deallocate(pointer p, size_type) { _aligned_free(p); }
	size_type max_size() const { return std::numeric_limits<size_type>::max() / sizeof(T); }
	void construct(pointer p, const T& t) { new(p)T(t); }
	void destroy(pointer p) { p; p->~T(); }
	bool operator==(mpAlignedAllocator const&) { return true; }
	bool operator!=(mpAlignedAllocator const& a) { return !operator==(a); }
};
template<class T, typename Alloc> inline bool operator==(const mpAlignedAllocator<T>& l, const mpAlignedAllocator<T>& r) { return (l.equals(r)); }
template<class T, typename Alloc> inline bool operator!=(const mpAlignedAllocator<T>& l, const mpAlignedAllocator<T>& r) { return (!(l == r)); }

typedef std::vector<mpParticle, mpAlignedAllocator<mpParticle> > mpParticleVector;
typedef std::vector<mpParticleSOA8, mpAlignedAllocator<mpParticleSOA8> > mpParticleSOA8Vector;
typedef std::vector<mpParticleIMDSOA8, mpAlignedAllocator<mpParticleIMDSOA8> > mpsphParticleIMDSOA88Vector;

class mpWorld;

class mpRenderer
{
public:
	virtual ~mpRenderer() {}
	virtual void render(mpWorld &world) = 0;
	virtual void reloadShader() = 0;
};
mpRenderer* mpCreateRendererD3D9(void *dev);
mpRenderer* mpCreateRendererD3D11(void *dev);
mpRenderer* mpCreateRendererOpenGL(void *dev);


class mpWorld
{
public:
	mpWorld();
	~mpWorld();
	void update(float dt);
	void render();

	void addParticles(mpParticle *p, int num);
	void addSphereColliders(ispc::SphereCollider *col, int num);
	void addPlaneColliders(ispc::PlaneCollider *col, int num);
	void addBoxColliders(ispc::BoxCollider *col, int num);
	void addForces(ispc::Force *force, int num);
	void clearParticles();
	void clearCollidersAndForces();

	const mpKernelParams& getKernelParams() const { return m_kparams;  }
	mpTempParams& getTempParams() { return m_tparams; }
	void setKernelParams(const mpKernelParams &v) { m_kparams = v; }
	void getViewProjection(XMFLOAT4X4 &out_mat, XMFLOAT3 &out_camerapos);
	void setViewProjection(const XMFLOAT4X4 &mat, const XMFLOAT3 &camerapos);
	int getNumParticles() { return m_num_active_particles; }
	mpParticle* getParticles() { return m_particles.empty() ? nullptr : &m_particles[0]; }
	std::mutex& getMutex() { return m_mutex;  }

private:
	mpParticleVector m_particles;
	mpParticleSOA8Vector m_particles_soa;
	mpsphParticleIMDSOA88Vector m_imd;
	std::vector<mpGridData> m_cells;
	int m_num_active_particles;

	std::vector<mpSphereCollider>   m_sphere_colliders;
	std::vector<mpPlaneCollider>    m_plane_colliders;
	std::vector<mpBoxCollider>      m_box_colliders;
	std::vector<mpForce>            m_forces;

	std::mutex		m_mutex;
	mpKernelParams	m_kparams;
	mpTempParams	m_tparams;
	XMFLOAT4X4		m_viewproj;
	XMFLOAT3		m_camerapos;
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
