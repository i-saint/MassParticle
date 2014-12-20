//#include "stdafx.h"
#include <algorithm>
#include "MassParticle.h"
#include "mpTypes.h"


mpKernelParams::mpKernelParams()
{
    (vec3&)world_center = vec3(0.0f, 0.0f, 0.0f);
    (vec3&)world_extent = vec3(10.24f, 10.24f, 10.24f);
    (vec3&)coord_scaler = vec3(1.0f, 1.0f, 1.0f);

    enable_interaction = 1;
    enable_colliders = 1;
    enable_forces = 1;

    solver_type = mpSolver_Impulse;
    lifetime = 30.0f;
    timestep = 0.01f;
    decelerate = 0.99f;
    advection = 0.5f;

    pressure_stiffness = 500.0f;
    wall_stiffness = 1500.0f;
    max_particles = 100000;
    particle_size = 0.08f;

    SPHRestDensity = 1000.0f;
    SPHParticleMass = 0.002f;
    SPHViscosity = 0.1f;
}



mpRenderer *g_mpRenderer;


const int32 SIMD_LANES = 8;

template<class T>
T clamp(T v, T minv, T maxv)
{
    return std::min<T>(std::max<T>(v, minv), maxv);
}

int32 soa_blocks(int32 i)
{
    return ceildiv(i, SIMD_LANES);
}

template<class T>
inline void simd_store(void *address, T v)
{
    _mm_store_ps((float*)address, (const simd128&)v);
}

void mpSoAnize( int32 num, const mpParticle *particles, ispc::Particle_SOA8 *out )
{
    int32 blocks = soa_blocks(num);
    for(int32 bi=0; bi<blocks; ++bi) {
        int32 i = SIMD_LANES*bi;
        ist::vec4soa3 soav;
        soav = ist::soa_transpose34(particles[i+0].position, particles[i+1].position, particles[i+2].position, particles[i+3].position);
        simd_store(out[bi].x+0, soav.x());
        simd_store(out[bi].y+0, soav.y());
        simd_store(out[bi].z+0, soav.z());
        soav = ist::soa_transpose34(particles[i+0].velocity, particles[i+1].velocity, particles[i+2].velocity, particles[i+3].velocity);
        simd_store(out[bi].vx+0, soav.x());
        simd_store(out[bi].vy+0, soav.y());
        simd_store(out[bi].vz+0, soav.z());

        soav = ist::soa_transpose34(particles[i+4].position, particles[i+5].position, particles[i+6].position, particles[i+7].position);
        simd_store(out[bi].x+4, soav.x());
        simd_store(out[bi].y+4, soav.y());
        simd_store(out[bi].z+4, soav.z());
        soav = ist::soa_transpose34(particles[i+4].velocity, particles[i+5].velocity, particles[i+6].velocity, particles[i+7].velocity);
        simd_store(out[bi].vx+4, soav.x());
        simd_store(out[bi].vy+4, soav.y());
        simd_store(out[bi].vz+4, soav.z());

        simd_store(out[bi].hit+0, _mm_set1_epi32(-1));
        simd_store(out[bi].hit+4, _mm_set1_epi32(-1));

        //// 
        //soas = ist::simdvec4_set(particles[i+0].params.density, particles[i+1].params.density, particles[i+2].params.density, particles[i+3].params.density);
        //_mm_store_ps(out[bi].density+0, soas);
        //soas = ist::simdvec4_set(particles[i+4].params.density, particles[i+5].params.density, particles[i+6].params.density, particles[i+7].params.density);
        //_mm_store_ps(out[bi].density+4, soas);
    }
}

void mpAoSnize( int32 num, const ispc::Particle_SOA8 *particles, mpParticle *out )
{
    int32 blocks = soa_blocks(num);
    for(int32 bi=0; bi<blocks; ++bi) {
        int32 i = 8*bi;
        ist::vec4soa4 aos_pos[2] = {
            ist::soa_transpose44(
                _mm_load_ps(particles[bi].x + 0),
                _mm_load_ps(particles[bi].y + 0),
                _mm_load_ps(particles[bi].z + 0),
                _mm_set1_ps(1.0f) ),
            ist::soa_transpose44(
                _mm_load_ps(particles[bi].x + 4),
                _mm_load_ps(particles[bi].y + 4),
                _mm_load_ps(particles[bi].z + 4),
                _mm_set1_ps(1.0f) ),
        };
        ist::vec4soa4 aos_vel[2] = {
            ist::soa_transpose44(
                _mm_load_ps(particles[bi].vx + 0),
                _mm_load_ps(particles[bi].vy + 0),
                _mm_load_ps(particles[bi].vz + 0),
                _mm_load_ps(particles[bi].speed + 0)),
            ist::soa_transpose44(
                _mm_load_ps(particles[bi].vx + 4),
                _mm_load_ps(particles[bi].vy + 4),
                _mm_load_ps(particles[bi].vz + 4),
                _mm_load_ps(particles[bi].speed + 4)),
        };

        int32 e = std::min<int32>(SIMD_LANES, num-i);
        for(int32 ei=0; ei<e; ++ei) {
            out[i+ei].position = aos_pos[ei/4][ei%4];
            out[i+ei].velocity = aos_vel[ei/4][ei%4];
            out[i+ei].density = particles[bi].density[ei];
            out[i+ei].hit_prev = out[i + ei].hit;
            out[i+ei].hit = particles[bi].hit[ei];
        }
    }
}

inline uint32 mpGenHash(mpWorld &world, const mpParticle &particle)
{
    const mpKernelParams &p = world.getKernelParams();
    mpTempParams &t = world.getTempParams();
    vec3 &bl = (vec3&)t.world_bounds_bl;
    vec3 &rcpCell = (vec3&)t.rcp_cell_size;
    vec3 &ppos = (vec3&)particle.position;

    uint32 xb = clamp<int32>(int32((ppos.x-bl.x)*rcpCell.x), 0, p.world_div.x-1);
    uint32 zb =	clamp<int32>(int32((ppos.z-bl.z)*rcpCell.z), 0, p.world_div.z-1) << (t.world_div_bits.x);
    uint32 yb = clamp<int32>(int32((ppos.y-bl.y)*rcpCell.y), 0, p.world_div.y-1) << (t.world_div_bits.x+t.world_div_bits.z);
    uint32 r = xb | zb | yb;
    if(particle.lifetime<=0.0f) { r |= 0x80000000; }
    return r;
}

inline void mpGenIndex(mpWorld &world, uint32 hash, int32 &xi, int32 &yi, int32 &zi)
{
    const mpKernelParams &p = world.getKernelParams();
    mpTempParams &t = world.getTempParams();
    xi =  hash & (p.world_div.x - 1);
    zi = (hash >> (t.world_div_bits.x)) & (p.world_div.z - 1);
    yi = (hash >> (t.world_div_bits.x+t.world_div_bits.z)) & (p.world_div.y - 1);
}



static const int g_particles_par_task = 2048;
static const int g_cells_par_task = 256;

mpWorld::mpWorld()
    : m_num_particles(0)
    , m_has_hithandler(false)
    , m_has_forcehandler(false)
    , m_trail_enabled(false)
    , m_num_particles_gpu(0)
    , m_num_particles_gpu_prev(0)
#ifdef mpWithCppScript
    , m_mono_array()
    , m_mono_gchandle()
#endif // mpWithCppScript
{
    m_sphere_colliders.reserve(64);

    m_particles.resize(m_kparams.max_particles);
    for (size_t i = 0; i < m_particles.size(); ++i) {
        m_particles[i].hash = 0x80000000;
    }
    clearParticles();
}

mpWorld::~mpWorld()
{
#ifdef mpWithCppScript
    if (m_mono_array) {
        cpsUnpin(m_mono_gchandle);
    }
#endif // mpWithCppScript
}

void mpWorld::addParticles(mpParticle *p, int num)
{
    num = std::min<uint32_t>(num, m_kparams.max_particles - m_num_particles);
    for (int i = 0; i<num; ++i) {
        m_particles[m_num_particles+i] = p[i];
        m_particles[m_num_particles+i].lifetime = m_kparams.lifetime;
    }
    m_num_particles += num;
}

void mpWorld::addPlaneColliders(mpPlaneCollider *col, int num)
{
    m_plane_colliders.insert(m_plane_colliders.end(), col, col + num);
}

void mpWorld::addSphereColliders(mpSphereCollider *col, int num)
{
    m_sphere_colliders.insert(m_sphere_colliders.end(), col, col+num);
}

void mpWorld::addCapsuleColliders(mpCapsuleCollider *col, int num)
{
    m_capsule_colliders.insert(m_capsule_colliders.end(), col, col+num);
}

void mpWorld::addBoxColliders(mpBoxCollider *col, int num)
{
    m_box_colliders.insert(m_box_colliders.end(), col, col+num);
}

void mpWorld::addForces(mpForce *force, int num)
{
    m_forces.insert(m_forces.end(), force, force+num);
}


inline ivec3 Position2Index(mpWorld &w, const vec3 &pos)
{
    const mpKernelParams &p = w.getKernelParams();
    mpTempParams &t = w.getTempParams();
    vec3 &bl = (vec3&)t.world_bounds_bl;
    vec3 &rcpCell = (vec3&)t.rcp_cell_size;
    int xb = clamp<int32>(int32((pos.x - bl.x)*rcpCell.x), 0, p.world_div.x - 1);
    int zb = clamp<int32>(int32((pos.z - bl.z)*rcpCell.z), 0, p.world_div.z - 1);
    int yb = clamp<int32>(int32((pos.y - bl.y)*rcpCell.y), 0, p.world_div.y - 1);
    return ivec3(xb, yb, zb);
}

inline bool testSphere_AABB(const vec3 &sphere_pos, float sphere_radius, vec3 aabb_bl, vec3 &aabb_ur)
{
    aabb_bl -= sphere_radius;
    aabb_ur += sphere_radius;
    return
        sphere_pos.x > aabb_bl.x && sphere_pos.x < aabb_ur.x &&
        sphere_pos.y > aabb_bl.y && sphere_pos.y < aabb_ur.y &&
        sphere_pos.z > aabb_bl.z && sphere_pos.z < aabb_ur.z;
}

inline bool testSphere_Sphere(const vec3 &sphere1_pos, float sphere1_radius, const vec3 &sphere2_pos, float sphere2_radius)
{
    float r = sphere1_radius + sphere2_radius;
    return glm::length_sq(sphere2_pos - sphere1_pos) < r*r;
}

// 0: not overlapped, 1: overlapped, 2: completely inside
inline int overlapAABB_AABB(const vec3 &aabb1_bl, const vec3 &aabb1_ur, const vec3 &aabb2_bl, const vec3 &aabb2_ur)
{
    if (aabb1_ur.x < aabb2_bl.x || aabb1_bl.x > aabb2_ur.x ||
        aabb1_ur.y < aabb2_bl.y || aabb1_bl.y > aabb2_ur.y ||
        aabb1_ur.z < aabb2_bl.z || aabb1_bl.z > aabb2_ur.z)
    {
        return 0;
    }
    else if (
        aabb1_ur.x < aabb2_ur.x && aabb1_bl.x > aabb2_bl.x &&
        aabb1_ur.y < aabb2_ur.y && aabb1_bl.y > aabb2_bl.y &&
        aabb1_ur.z < aabb2_ur.z && aabb1_bl.z > aabb2_bl.z)
    {
        return 2;
    }
    else if (
        aabb2_ur.x < aabb1_ur.x && aabb2_bl.x > aabb1_bl.x &&
        aabb2_ur.y < aabb1_ur.y && aabb2_bl.y > aabb1_bl.y &&
        aabb2_ur.z < aabb1_ur.z && aabb2_bl.z > aabb1_bl.z)
    {
        return 2;
    }
    return 1;
}


// 0: not overlapped, 1: overlapped, 2: completely inside
inline int overlapSphere_AABB(const vec3 &sphere_pos, float sphere_radius, const vec3 &aabb_bl, const vec3 &aabb_ur)
{
    int r = overlapAABB_AABB(sphere_pos - sphere_radius, sphere_pos + sphere_radius, aabb_bl, aabb_ur);
    if (r == 2) {
        float r2 = sphere_radius*sphere_radius;
        if (glm::length_sq(aabb_bl - sphere_pos) > r2 || glm::length_sq(aabb_ur - sphere_pos) > r2) {
            r = 1;
        }
    }
    return r;
}


// f: [](const mpCell &cell, const ivec3 &cell_index)
template<class F>
inline void ScanCells(mpWorld &w, const ivec3 &imin, const ivec3 &imax, const F &f)
{
    const mpCellCont &cells = w.getCells();
    ivec3 bits = w.getTempParams().world_div_bits;
    mpParticle *particles = w.getParticles();
    for (int iy = imin.y; iy < imax.y; ++iy) {
        for (int iz = imin.z; iz < imax.z; ++iz) {
            for (int ix = imin.x; ix < imax.x; ++ix) {
                uint32 ci = ix | (iz << bits.x) | (iy << (bits.x + bits.z));
                f(cells[ci], ivec3(ix, iy, iz));
            }
        }
    }
}

// f: [](const mpCell &cell, const ivec3 &cell_index)
template<class F>
inline void ScanCellsParallel(mpWorld &w, const ivec3 &imin, const ivec3 &imax, const F &f)
{
    const mpCellCont &cells = w.getCells();
    ivec3 bits = w.getTempParams().world_div_bits;
    mpParticle *particles = w.getParticles();
    int lz = imax.z - imin.z;
    int ly = imax.y - imin.y;
    if (ly > 4) {
        tbb::parallel_for(imin.y, imax.y, [&](int iy){
            for (int iz = imin.z; iz < imax.z; ++iz) {
                for (int ix = imin.x; ix < imax.x; ++ix) {
                    uint32 ci = ix | (iz << bits.x) | (iy << (bits.x + bits.z));
                    f(cells[ci], ivec3(ix, iy, iz));
                }
            }
        });
    }
    else if(lz > 4) {
        for (int iy = imin.y; iy < imax.y; ++iy) {
            tbb::parallel_for(imin.z, imax.z, [&](int iz){
                for (int ix = imin.x; ix < imax.x; ++ix) {
                    uint32 ci = ix | (iz << bits.x) | (iy << (bits.x + bits.z));
                    f(cells[ci], ivec3(ix, iy, iz));
                }
            });
        }
    }
    else {
        ScanCells(w, imin, imax, f);
    }
}

void mpWorld::scanSphere(mpHitHandler handler, const vec3 &pos, float radius)
{
    const mpKernelParams &k = m_kparams;
    const mpTempParams &t = m_tparams;
    ivec3 imin = Position2Index(*this, pos - radius);
    ivec3 imax = Position2Index(*this, pos + radius) + 1;
    int r = 0;
    ScanCells(*this, imin, imax, [&](const mpCell &cell, const ivec3 &ci){
        vec3 cell_bl = t.world_bounds_bl + (t.cell_size * vec3(ci));
        vec3 cell_ur = cell_bl + t.cell_size;
        int o = overlapSphere_AABB(pos, radius, cell_bl, cell_ur);
        if (o == 2) {
            for (int i = cell.begin; i < cell.end; ++i) {
                handler(&m_particles[i]);
            }
        }
        else if (o == 1) {
            for (int i = cell.begin; i < cell.end; ++i) {
                if (testSphere_Sphere((vec3&)m_particles[i].position, k.particle_size, pos, radius)) {
                    handler(&m_particles[i]);
                }
            }
        }
    });
}

void mpWorld::scanAABB(mpHitHandler handler, const vec3 &center, const vec3 &extent)
{
    const mpKernelParams &k = m_kparams;
    const mpTempParams &t = m_tparams;
    ivec3 imin = Position2Index(*this, center - extent);
    ivec3 imax = Position2Index(*this, center + extent) + 1;
    ScanCells(*this, imin, imax, [&](const mpCell &cell, const ivec3 &ci){
        vec3 cell_bl = t.world_bounds_bl + (t.cell_size * vec3(ci));
        vec3 cell_ur = cell_bl + t.cell_size;
        int o = overlapAABB_AABB(center - extent, center + extent, cell_bl, cell_ur);
        if (o == 2) {
            for (int i = cell.begin; i < cell.end; ++i) {
                handler(&m_particles[i]);
            }
        }
        else if (o == 1) {
            for (int i = cell.begin; i < cell.end; ++i) {
                if (testSphere_AABB((vec3&)m_particles[i].position, k.particle_size, center - extent, center + extent)) {
                    handler(&m_particles[i]);
                }
            }
        }
    });
}

void mpWorld::scanSphereParallel(mpHitHandler handler, const vec3 &pos, float radius)
{
    const mpKernelParams &k = m_kparams;
    const mpTempParams &t = m_tparams;
    ivec3 imin = Position2Index(*this, pos - radius);
    ivec3 imax = Position2Index(*this, pos + radius) + 1;
    int r = 0;
    ScanCellsParallel(*this, imin, imax, [&](const mpCell &cell, const ivec3 &ci){
        vec3 cell_bl = t.world_bounds_bl + (t.cell_size * vec3(ci));
        vec3 cell_ur = cell_bl + t.cell_size;
        int o = overlapSphere_AABB(pos, radius, cell_bl, cell_ur);
        if (o == 2) {
            for (int i = cell.begin; i < cell.end; ++i) {
                handler(&m_particles[i]);
            }
        }
        else if (o == 1) {
            for (int i = cell.begin; i < cell.end; ++i) {
                if (testSphere_Sphere((vec3&)m_particles[i].position, k.particle_size, pos, radius)) {
                    handler(&m_particles[i]);
                }
            }
        }
    });
}

void mpWorld::scanAABBParallel(mpHitHandler handler, const vec3 &center, const vec3 &extent)
{
    const mpKernelParams &k = m_kparams;
    const mpTempParams &t = m_tparams;
    ivec3 imin = Position2Index(*this, center - extent);
    ivec3 imax = Position2Index(*this, center + extent) + 1;
    ScanCellsParallel(*this, imin, imax, [&](const mpCell &cell, const ivec3 &ci){
        vec3 cell_bl = t.world_bounds_bl + (t.cell_size * vec3(ci));
        vec3 cell_ur = cell_bl + t.cell_size;
        int o = overlapAABB_AABB(center - extent, center + extent, cell_bl, cell_ur);
        if (o == 2) {
            for (int i = cell.begin; i < cell.end; ++i) {
                handler(&m_particles[i]);
            }
        }
        else if (o == 1) {
            for (int i = cell.begin; i < cell.end; ++i) {
                if (testSphere_AABB((vec3&)m_particles[i].position, k.particle_size, center - extent, center + extent)) {
                    handler(&m_particles[i]);
                }
            }
        }
    });
}


void mpWorld::clearParticles()
{
    m_num_particles = 0;
    for (uint32 i = 0; i < m_particles.size(); ++i) {
        m_particles[i].lifetime = 0.0f;
        m_particles[i].hash = 0x80000000;
    }
}

void mpWorld::clearCollidersAndForces()
{
    m_hitdata.clear();
    m_plane_colliders.clear();
    m_sphere_colliders.clear();
    m_capsule_colliders.clear();
    m_box_colliders.clear();
    m_forces.clear();

    m_has_hithandler = false;
    m_has_forcehandler = false;
}



void mpWorld::beginUpdate(float dt)
{
    m_taskgroup.run([=](){ update(dt); });
}

void mpWorld::endUpdate()
{
    m_taskgroup.wait();
}

void mpWorld::update(float32 dt)
{
    if (m_num_particles == 0) { return; }

    mpKernelParams &kp = m_kparams;
    mpTempParams &tp = m_tparams;
    int cell_num = kp.world_div.x*kp.world_div.y*kp.world_div.z;

    {
        vec3 &wpos = (vec3&)kp.world_center;
        vec3 &wsize = (vec3&)kp.world_extent;
        vec3 &cellsize = (vec3&)tp.cell_size;
        vec3 &cellsize_r = (vec3&)tp.rcp_cell_size;
        vec3 &bl = (vec3&)tp.world_bounds_bl;
        vec3 &ur = (vec3&)tp.world_bounds_ur;
        kp.particle_size = std::max<float>(kp.particle_size, 0.00001f);
        kp.max_particles = std::max<int>(kp.max_particles, 128);
        m_num_particles = std::min<int>(m_num_particles, kp.max_particles);
        kp.world_div.x = std::max<int>(kp.world_div.x, 1);
        kp.world_div.y = std::max<int>(kp.world_div.y, 1);
        kp.world_div.z = std::max<int>(kp.world_div.z, 1);
        tp.world_div_bits.x = mpMSB(kp.world_div.x);
        tp.world_div_bits.y = mpMSB(kp.world_div.y);
        tp.world_div_bits.z = mpMSB(kp.world_div.z);
        cellsize = (wsize*2.0f / vec3((float)kp.world_div.x, (float)kp.world_div.y, (float)kp.world_div.z));
        cellsize_r = vec3(1.0f, 1.0f, 1.0f) / cellsize;
        bl = wpos - wsize;
        ur = wpos + wsize;

        int reserve_size = mpParticlesEachLine * (ceildiv(kp.max_particles, mpParticlesEachLine));
        int SOADataNum = std::max<int>(cell_num, kp.max_particles / 8);
        m_cells.resize(cell_num);
        m_particles.resize(kp.max_particles);
        m_particles_soa.resize(SOADataNum);
        m_imd_soa.resize(SOADataNum);
        m_particles_gpu.reserve(reserve_size);
        m_particles_gpu.resize(kp.max_particles);
    }

    mpCell				*ce = &m_cells[0];
    mpForce				*forces = m_forces.empty() ? nullptr : &m_forces[0];
    mpPlaneCollider		*planes = m_plane_colliders.empty() ? nullptr : &m_plane_colliders[0];
    mpSphereCollider	*spheres = m_sphere_colliders.empty() ? nullptr : &m_sphere_colliders[0];
    mpCapsuleCollider	*capsules = m_capsule_colliders.empty() ? nullptr : &m_capsule_colliders[0];
    mpBoxCollider		*boxes = m_box_colliders.empty() ? nullptr : &m_box_colliders[0];

    int num_colliders =  m_plane_colliders.size() + m_sphere_colliders.size() + m_capsule_colliders.size() + m_box_colliders.size();

    {
        const float PI = 3.14159265359f;
        m_kparams.RcpParticleSize2 = 1.0f / (m_kparams.particle_size*2.0f);
        m_kparams.SPHDensityCoef = m_kparams.SPHParticleMass * 315.0f / (64.0f * PI * pow(m_kparams.particle_size, 9));
        m_kparams.SPHGradPressureCoef = m_kparams.SPHParticleMass * -45.0f / (PI * pow(m_kparams.particle_size, 6));
        m_kparams.SPHLapViscosityCoef = m_kparams.SPHParticleMass * m_kparams.SPHViscosity * 45.0f / (PI * pow(m_kparams.particle_size, 6));
    }

    // clear grid
    tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, g_cells_par_task),
        [&](const tbb::blocked_range<int> &r) {
            for(int i=r.begin(); i!=r.end(); ++i) {
                ce[i].begin = ce[i].end = 0;
            }
        });

    // gen hash
    tbb::parallel_for(tbb::blocked_range<int>(0, (int32)m_num_particles, g_particles_par_task),
        [&](const tbb::blocked_range<int> &r) {
            for(int i=r.begin(); i!=r.end(); ++i) {
                vec3 &ppos = (vec3&)m_particles[i].position;
                vec3 &bl = m_tparams.world_bounds_bl;
                vec3 &ur = m_tparams.world_bounds_ur;
                if (ppos.x<bl.x || ppos.y<bl.y || ppos.z<bl.z ||
                    ppos.x>ur.x || ppos.y>ur.y || ppos.z>ur.z)
                {
                    m_particles[i].lifetime = std::min<float>(m_particles[i].lifetime, 0.333f);
                }
                m_particles[i].lifetime = std::max<float32>(m_particles[i].lifetime - dt, 0.0f);
                m_particles[i].hash = mpGenHash(*this, m_particles[i]);
            }
        });

    // sort by hash
    tbb::parallel_sort(&m_particles[0], &m_particles[0]+m_num_particles, 
        [&](const mpParticle &a, const mpParticle &b) { return a.hash < b.hash; } );

    // count num particles
    tbb::parallel_for(tbb::blocked_range<int>(0, (int32)m_num_particles, g_particles_par_task),
        [&](const tbb::blocked_range<int> &r) {
            for(int i=r.begin(); i!=r.end(); ++i) {
                const uint32 G_ID = i;
                uint32 G_ID_PREV = G_ID-1;
                uint32 G_ID_NEXT = G_ID+1;

                uint32 cell = m_particles[G_ID].hash;
                uint32 cell_prev = (G_ID_PREV==-1) ? -1 : m_particles[G_ID_PREV].hash;
                uint32 cell_next = (G_ID_NEXT == kp.max_particles) ? -2 : m_particles[G_ID_NEXT].hash;
                if((cell & 0x80000000) != 0) { // highest bit is live flag
                    if((cell_prev & 0x80000000) == 0) { // 
                        m_num_particles = G_ID;
                    }
                }
                else {
                    if(cell != cell_prev) {
                        ce[cell].begin = G_ID;
                    }
                    if(cell != cell_next) {
                        ce[cell].end = G_ID + 1;
                    }
                }
            }
    });
    {
        if( (m_particles[0].hash & 0x80000000) != 0 ) {
            m_num_particles = 0;
        }
        else if ((m_particles[kp.max_particles - 1].hash & 0x80000000) == 0) {
            m_num_particles = kp.max_particles;
        }
    }

    {
        int32 soai = 0;
        for(int i=0; i!=cell_num; ++i) {
            ce[i].soai = soai;
            soai += soa_blocks(ce[i].end-ce[i].begin);
        }
    }

    // AoS -> SoA
    tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, g_cells_par_task),
        [&](const tbb::blocked_range<int> &r) {
            for(int i=r.begin(); i!=r.end(); ++i) {
                int32 n = ce[i].end - ce[i].begin;
                if(n == 0) { continue; }
                mpParticle *p = &m_particles[ce[i].begin];
                ispc::Particle_SOA8 *t = &m_particles_soa[ce[i].soai];
                mpSoAnize(n, p, t);
            }
    });

    if (m_kparams.solver_type == mpSolver_Impulse) {
        // impulse
        tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, g_cells_par_task),
            [&](const tbb::blocked_range<int> &r) {
            for (int i = r.begin(); i != r.end(); ++i) {
                int32 n = ce[i].end - ce[i].begin;
                if (n == 0) { continue; }
                int xi, yi, zi;
                mpGenIndex(*this, i, xi, yi, zi);
                if (kp.enable_interaction) {
                    ispc::impUpdatePressure(
                        m_kparams,
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi);
                }
                if (kp.enable_forces) {
                    ispc::ProcessExternalForce(
                        m_kparams,
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi,
                        forces, (int32)m_forces.size());
                }
                if (kp.enable_colliders) {
                    ispc::ProcessColliders(
                        m_kparams,
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi,
                        planes, (int32)m_plane_colliders.size(),
                        spheres, (int32)m_sphere_colliders.size(),
                        capsules, (int32)m_capsule_colliders.size(),
                        boxes, (int32)m_box_colliders.size());
                }
            }
        });
        tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, g_cells_par_task),
            [&](const tbb::blocked_range<int> &r) {
            for (int i = r.begin(); i != r.end(); ++i) {
                int32 n = ce[i].end - ce[i].begin;
                if (n == 0) { continue; }
                int xi, yi, zi;
                mpGenIndex(*this, i, xi, yi, zi);
                ispc::Integrate(
                    m_kparams,
                    (ispc::Particle*)&m_particles_soa[0],
                    (ispc::ParticleIMData*)&m_imd_soa[0],
                    ce, xi, yi, zi);
            }
        });
    }
    else if (m_kparams.solver_type == mpSolver_SPH || m_kparams.solver_type == mpSolver_SPHEst) {
        if (kp.enable_interaction && m_kparams.solver_type == mpSolver_SPH) {
            tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, g_cells_par_task),
                [&](const tbb::blocked_range<int> &r) {
                for (int i = r.begin(); i != r.end(); ++i) {
                    int32 n = ce[i].end - ce[i].begin;
                    if (n == 0) { continue; }
                    int xi, yi, zi;
                    mpGenIndex(*this, i, xi, yi, zi);
                    ispc::sphUpdateDensity(
                        m_kparams,
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi);
                }
            });
            tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, g_cells_par_task),
                [&](const tbb::blocked_range<int> &r) {
                for (int i = r.begin(); i != r.end(); ++i) {
                    int32 n = ce[i].end - ce[i].begin;
                    if (n == 0) { continue; }
                    int xi, yi, zi;
                    mpGenIndex(*this, i, xi, yi, zi);
                    ispc::sphUpdateForce(
                        m_kparams,
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi);
                }
            });
        }
        else if (kp.enable_interaction && m_kparams.solver_type == mpSolver_SPHEst) {
            tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, g_cells_par_task),
                [&](const tbb::blocked_range<int> &r) {
                for (int i = r.begin(); i != r.end(); ++i) {
                    int32 n = ce[i].end - ce[i].begin;
                    if (n == 0) { continue; }
                    int xi, yi, zi;
                    mpGenIndex(*this, i, xi, yi, zi);
                    ispc::sphUpdateDensityEst1(
                        m_kparams,
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi);
                }
            });
            tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, g_cells_par_task),
                [&](const tbb::blocked_range<int> &r) {
                for (int i = r.begin(); i != r.end(); ++i) {
                    int32 n = ce[i].end - ce[i].begin;
                    if (n == 0) { continue; }
                    int xi, yi, zi;
                    mpGenIndex(*this, i, xi, yi, zi);
                    ispc::sphUpdateDensityEst2(
                        m_kparams,
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi);
                }
            });
            tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, g_cells_par_task),
                [&](const tbb::blocked_range<int> &r) {
                for (int i = r.begin(); i != r.end(); ++i) {
                    int32 n = ce[i].end - ce[i].begin;
                    if (n == 0) { continue; }
                    int xi, yi, zi;
                    mpGenIndex(*this, i, xi, yi, zi);
                    ispc::sphUpdateForce(
                        m_kparams,
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi);
                }
            });
        }

        tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, g_cells_par_task),
            [&](const tbb::blocked_range<int> &r) {
            for (int i = r.begin(); i != r.end(); ++i) {
                int32 n = ce[i].end - ce[i].begin;
                if (n == 0) { continue; }
                int xi, yi, zi;
                mpGenIndex(*this, i, xi, yi, zi);
                if (kp.enable_forces) {
                    ispc::ProcessExternalForce(
                        m_kparams,
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi,
                        forces, (int32)m_forces.size());
                }
                if (kp.enable_colliders) {
                    ispc::ProcessColliders(
                        m_kparams,
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi,
                        planes, (int32)m_plane_colliders.size(),
                        spheres, (int32)m_sphere_colliders.size(),
                        capsules, (int32)m_capsule_colliders.size(),
                        boxes, (int32)m_box_colliders.size());
                }
                ispc::Integrate(
                    m_kparams,
                    (ispc::Particle*)&m_particles_soa[0],
                    (ispc::ParticleIMData*)&m_imd_soa[0],
                    ce, xi, yi, zi);
            }
        });
    }

    // SoA -> AoS
    tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, g_cells_par_task),
        [&](const tbb::blocked_range<int> &r) {
            for(int i=r.begin(); i!=r.end(); ++i) {
                int32 n = ce[i].end - ce[i].begin;
                if(n > 0) {
                    mpParticle *p = &m_particles[ce[i].begin];
                    mpParticleSOA8 *t = &m_particles_soa[ce[i].soai];
                    mpAoSnize(n, t, p);
                }
            }
    });

    // make clone data for GPU
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        int num_particles_needs_copy = std::max<int>(m_num_particles, m_num_particles_gpu);
        m_num_particles_gpu = m_num_particles;
        if (num_particles_needs_copy > 0) {
            memcpy(&m_particles_gpu[0], &m_particles[0], sizeof(mpParticle)*num_particles_needs_copy);
        }
    }
}

void mpWorld::callHandlers()
{
    int num_colliders = 0;
    if (!m_plane_colliders.empty()) { num_colliders = std::max(num_colliders, m_plane_colliders.back().props.owner_id + 1); }
    if (!m_sphere_colliders.empty()) { num_colliders = std::max(num_colliders, m_sphere_colliders.back().props.owner_id + 1); }
    if (!m_capsule_colliders.empty()) { num_colliders = std::max(num_colliders, m_capsule_colliders.back().props.owner_id + 1); }
    if (!m_box_colliders.empty()) { num_colliders = std::max(num_colliders, m_box_colliders.back().props.owner_id + 1); }
    m_collider_properties.resize(num_colliders);

    for (auto &c : m_plane_colliders) { m_collider_properties[c.props.owner_id] = &c.props; }
    for (auto &c : m_sphere_colliders) { m_collider_properties[c.props.owner_id] = &c.props; }
    for (auto &c : m_capsule_colliders) { m_collider_properties[c.props.owner_id] = &c.props; }
    for (auto &c : m_box_colliders) { m_collider_properties[c.props.owner_id] = &c.props; }

    tbb::parallel_invoke(
        [&](){
            for (auto p : m_collider_properties) {
                if (p->hit_handler != nullptr) {
                    m_has_hithandler = true;
                    break;
                }
            }
        },
        [&](){
            for (auto p : m_collider_properties) {
                if (p->force_handler != nullptr) {
                    m_has_forcehandler = true;
                    break;
                }
            }
        }
    );

    if (m_has_hithandler) {
        for (int i = 0; i < m_num_particles; ++i) {
            mpParticle &p = m_particles[i];
            if (p.hit != -1) {
                mpHitHandler handler = (mpHitHandler)(m_collider_properties[p.hit]->hit_handler);
                if (handler) { handler(&p); }
            }
        }
    }
    if (m_has_forcehandler) {
        m_hitdata.resize(num_colliders);
        memset((void*)&m_hitdata[0], 0, sizeof(mpHitData)*m_hitdata.size());
        tbb::parallel_for(tbb::blocked_range<int>(0, m_num_particles, g_particles_par_task),
            [&](const tbb::blocked_range<int> &r) {
            mpHitDataCont &hits = m_hitdata_work.local();
            hits.resize(num_colliders);
            for (int i = r.begin(); i != r.end(); ++i) {
                mpParticle &p = m_particles[i];
                if (p.hit != -1) {
                    (simdvec4&)hits[p.hit].position += (simdvec4&)p.position;
                    (simdvec4&)hits[p.hit].velocity += (simdvec4&)p.velocity;
                    ++hits[p.hit].num_hits;
                }
            }
        });
        m_hitdata_work.combine_each([&](const mpHitDataCont &hits){
            for (int i = 0; i < (int)hits.size(); ++i) {
                const mpHitData &h = hits[i];
                (simdvec4&)m_hitdata[i].position += h.position;
                (simdvec4&)m_hitdata[i].velocity += h.velocity;
                m_hitdata[i].num_hits += h.num_hits;
            }
            memset((void*)&hits[0], 0, sizeof(mpHitData)*hits.size());
        });

        for (int i = 0; i < num_colliders; ++i) {
            if (m_hitdata[i].num_hits == 0) continue;

            (vec3&)m_hitdata[i].position /= m_hitdata[i].num_hits;

            mpForceHandler handler = (mpForceHandler)m_collider_properties[i]->force_handler;
            if (handler) {
                handler(&m_hitdata[i]);
            }
        }
    }
}


inline vec2 mpComputeDataTextureCoord(int nth)
{
    static const float xd = 1.0f / mpDataTextureWidth;
    static const float yd = 1.0f / mpDataTextureHeight;
    int xi = (nth * 3) % mpDataTextureWidth;
    int yi = (nth * 3) / mpDataTextureWidth;
    return vec2(xd*xi + xd*0.5f, yd*yi + yd*0.5f);
}

void mpWorld::generatePointMesh(int mi, mpMeshData *mds)
{
    const int batch_size = 65000;

    mpMeshData &md = *mds;
    int begin = batch_size * mi;
    int end = batch_size * (mi+1);
    for (int pi = begin; pi != end; ++pi) {
        int si = pi - begin;
        vec2 t = mpComputeDataTextureCoord(pi);
        md.vertices[si] = vec3(0.0f,0.0f,0.0f);
        md.texcoords[si] = t;
        md.indices[si] = si;
    }
}

void mpWorld::generateCubeMesh(int mi, mpMeshData *mds)
{
    const int batch_size = 2700;
    const float s = 0.5f / 100.0f;
    const float p = 1.0f;
    const float n = -1.0f;
    const float z = 0.0f;

    const vec3 c_vertices[] =
    {
        vec3(-s,-s, s), vec3( s,-s, s), vec3( s, s, s), vec3(-s, s, s),
        vec3(-s, s,-s), vec3( s, s,-s), vec3( s, s, s), vec3(-s, s, s),
        vec3(-s,-s,-s), vec3( s,-s,-s), vec3( s,-s, s), vec3(-s,-s, s),
        vec3(-s,-s, s), vec3(-s,-s,-s), vec3(-s, s,-s), vec3(-s, s, s),
        vec3( s,-s, s), vec3( s,-s,-s), vec3( s, s,-s), vec3( s, s, s),
        vec3(-s,-s,-s), vec3( s,-s,-s), vec3( s, s,-s), vec3(-s, s,-s),
    };
    const vec3 c_normals[] = {
        vec3(z, z, p), vec3(z, z, p), vec3(z, z, p), vec3(z, z, p),
        vec3(z, p, z), vec3(z, p, z), vec3(z, p, z), vec3(z, p, z),
        vec3(z, n, z), vec3(z, n, z), vec3(z, n, z), vec3(z, n, z),
        vec3(n, z, z), vec3(n, z, z), vec3(n, z, z), vec3(n, z, z),
        vec3(p, z, z), vec3(p, z, z), vec3(p, z, z), vec3(p, z, z),
        vec3(z, z, n), vec3(z, z, n), vec3(z, z, n), vec3(z, z, n),
    };
    const int c_indices[] =
    {
        0,1,3, 3,1,2,
        5,4,6, 6,4,7,
        8,9,11, 11,9,10,
        13,12,14, 14,12,15,
        16,17,19, 19,17,18,
        21,20,22, 22,20,23,
    };

    mpMeshData &md = *mds;
    int begin = batch_size * mi;
    int end = batch_size * (mi+1);
    for (int pi = begin; pi != end; ++pi) {
        int si = pi - begin;
        vec2 t = mpComputeDataTextureCoord(pi);
        for (int vi = 0; vi < 24; ++vi) {
            md.vertices[24 * si + vi] = c_vertices[vi];
            md.normals[24 * si + vi] = c_normals[vi];
            md.texcoords[24 * si + vi] = t;
        }
        for (int vi = 0; vi < 36; ++vi) {
            md.indices[36 * si + vi] = 24 * si + c_indices[vi];
        }
    }
}


int mpWorld::updateDataTexture(void *tex)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (g_mpRenderer && !m_particles_gpu.empty()) {
        int num_needs_copy = std::max<int>(m_num_particles_gpu, m_num_particles_gpu_prev);
        m_num_particles_gpu_prev = m_num_particles_gpu;
        g_mpRenderer->updateDataTexture(tex, &m_particles_gpu[0], sizeof(mpParticle)*num_needs_copy);
    }
    return m_num_particles_gpu;
}

#ifdef mpWithCppScript
int mpWorld::updateDataBuffer(UnityEngine::ComputeBuffer buf)
{
    if (!m_mono_array) {
        m_mono_array = cpsArray::create(cpsTypeinfo<uint8_t>(), sizeof(mpParticle)*m_kparams.max_particles);
        m_mono_gchandle = cpsPin(m_mono_array);
    }

    memcpy(m_mono_array.getDataPtr(), &m_particles_gpu[0], sizeof(mpParticle)*m_num_particles_gpu);
    buf.SetData(m_mono_array);
    return m_num_particles_gpu;
}
#endif // mpWithCppScript
