#include "pch.h"
#include "mpFoundation.h"
#include "mpCore_ispc.h"
#include "mpWorld.h"
#include "MassParticle.h"



mpRenderer *g_mpRenderer;


const i32 SOA_BOCK_SIZE = 8;

template<class T>
T clamp(T v, T minv, T maxv)
{
    return std::min<T>(std::max<T>(v, minv), maxv);
}

i32 soa_blocks(i32 i)
{
    return ceildiv(i, SOA_BOCK_SIZE);
}

template<class T>
inline void simd_store(void *address, T v)
{
    _mm_store_ps((float*)address, (const simd128&)v);
}

void mpSoAnize(const mpCell &cell, const mpParticleCont &particles, mpSoAData &soa)
{
    int num = cell.end - cell.begin;
    i32 si = cell.soai * SOA_BOCK_SIZE;
    i32 blocks = soa_blocks(num);
    float *pos_x = &soa.pos_x[si];
    float *pos_y = &soa.pos_y[si];
    float *pos_z = &soa.pos_z[si];
    float *vel_x = &soa.vel_x[si];
    float *vel_y = &soa.vel_y[si];
    float *vel_z = &soa.vel_z[si];
    float *acl_x = &soa.acl_x[si];
    float *acl_y = &soa.acl_y[si];
    float *acl_z = &soa.acl_z[si];
    float *speed = &soa.speed[si];
    float *density = &soa.density[si];
    int *hit = &soa.hit[si];

    ist::vec4soa3 soav;
    for (i32 bi = 0; bi < blocks; ++bi) {
        i32 i = bi*SOA_BOCK_SIZE;
        i32 pi = cell.begin + i;

        soav = ist::soa_transpose34(particles[pi + 0].position, particles[pi + 1].position, particles[pi + 2].position, particles[pi + 3].position);
        simd_store(&pos_x[i + 0], soav.x());
        simd_store(&pos_y[i + 0], soav.y());
        simd_store(&pos_z[i + 0], soav.z());
        soav = ist::soa_transpose34(particles[pi + 0].velocity, particles[pi + 1].velocity, particles[pi + 2].velocity, particles[pi + 3].velocity);
        simd_store(&vel_x[i + 0], soav.x());
        simd_store(&vel_y[i + 0], soav.y());
        simd_store(&vel_z[i + 0], soav.z());

        soav = ist::soa_transpose34(particles[pi + 4].position, particles[pi + 5].position, particles[pi + 6].position, particles[pi + 7].position);
        simd_store(&pos_x[i + 4], soav.x());
        simd_store(&pos_y[i + 4], soav.y());
        simd_store(&pos_z[i + 4], soav.z());
        soav = ist::soa_transpose34(particles[pi + 4].velocity, particles[pi + 5].velocity, particles[pi + 6].velocity, particles[pi + 7].velocity);
        simd_store(&vel_x[i + 4], soav.x());
        simd_store(&vel_y[i + 4], soav.y());
        simd_store(&vel_z[i + 4], soav.z());

        simd_store(&hit[i + 0], _mm_set1_epi32(0));
        simd_store(&hit[i + 4], _mm_set1_epi32(0));
    }
}
void mpAoSnize(const mpCell &cell, const mpSoAData &soa, mpParticleCont &particles, mpParticleIMCont &im)
{
    int num = cell.end - cell.begin;
    i32 si = cell.soai * SOA_BOCK_SIZE;
    i32 blocks = soa_blocks(num);
    const float *pos_x = &soa.pos_x[si];
    const float *pos_y = &soa.pos_y[si];
    const float *pos_z = &soa.pos_z[si];
    const float *vel_x = &soa.vel_x[si];
    const float *vel_y = &soa.vel_y[si];
    const float *vel_z = &soa.vel_z[si];
    const float *acl_x = &soa.acl_x[si];
    const float *acl_y = &soa.acl_y[si];
    const float *acl_z = &soa.acl_z[si];
    const float *speed = &soa.speed[si];
    const float *density = &soa.density[si];
    const int *hit = &soa.hit[si];

    for (i32 bi = 0; bi < blocks; ++bi) {
        i32 i = bi*SOA_BOCK_SIZE;
        ist::vec4soa4 aos_pos[2] = {
            ist::soa_transpose44(
            _mm_load_ps(&pos_x[i + 0]),
                _mm_load_ps(&pos_y[i + 0]),
                _mm_load_ps(&pos_z[i + 0]),
                _mm_set1_ps(1.0f)),
            ist::soa_transpose44(
                _mm_load_ps(&pos_x[i + 4]),
                _mm_load_ps(&pos_y[i + 4]),
                _mm_load_ps(&pos_z[i + 4]),
                _mm_set1_ps(1.0f)),
        };
        ist::vec4soa4 aos_vel[2] = {
            ist::soa_transpose44(
            _mm_load_ps(&vel_x[i + 0]),
                _mm_load_ps(&vel_y[i + 0]),
                _mm_load_ps(&vel_z[i + 0]),
                _mm_load_ps(&speed[i + 0])),
            ist::soa_transpose44(
                _mm_load_ps(&vel_x[i + 4]),
                _mm_load_ps(&vel_y[i + 4]),
                _mm_load_ps(&vel_z[i + 4]),
                _mm_load_ps(&speed[i + 4])),
        };
        ist::vec4soa4 aos_axl[2] = {
            ist::soa_transpose44(
            _mm_load_ps(&acl_x[i + 0]),
                _mm_load_ps(&acl_y[i + 0]),
                _mm_load_ps(&acl_z[i + 0]),
                _mm_set1_ps(0.0f)),
            ist::soa_transpose44(
                _mm_load_ps(&acl_x[i + 4]),
                _mm_load_ps(&acl_y[i + 4]),
                _mm_load_ps(&acl_z[i + 4]),
                _mm_set1_ps(0.0f)),
        };

        i32 pi = cell.begin + i;
        i32 e = std::min<i32>(SOA_BOCK_SIZE, num - i);
        for (i32 ei = 0; ei < e; ++ei) {
            u32 id = particles[pi + ei].id;
            im[pi + ei].accel = aos_axl[ei / 4][ei % 4];
            particles[pi + ei].position = aos_pos[ei / 4][ei % 4];
            particles[pi + ei].velocity = aos_vel[ei / 4][ei % 4];
            particles[pi + ei].density = ((float*)density)[bi*SOA_BOCK_SIZE + ei];
            particles[pi + ei].hit_prev = particles[pi + ei].hit;
            particles[pi + ei].hit = (u16)((int*)hit)[bi*SOA_BOCK_SIZE + ei];
            particles[pi + ei].id = id;
        }
    }
}


inline u32 mpGenHash(mpWorld &world, const mpParticle &particle)
{
    const mpKernelParams &p = world.getKernelParams();
    mpTempParams &t = world.getTempParams();
    vec3 &bl = (vec3&)t.world_bounds_bl;
    vec3 &rcpCell = (vec3&)t.rcp_cell_size;
    vec3 &ppos = (vec3&)particle.position;

    u32 xb = clamp<i32>(i32((ppos.x - bl.x)*rcpCell.x), 0, p.world_div.x - 1);
    u32 zb = clamp<i32>(i32((ppos.z - bl.z)*rcpCell.z), 0, p.world_div.z - 1) << (t.world_div_bits.x);
    u32 yb = clamp<i32>(i32((ppos.y - bl.y)*rcpCell.y), 0, p.world_div.y - 1) << (t.world_div_bits.x + t.world_div_bits.z);
    u32 r = xb | zb | yb;
    if (particle.lifetime <= 0.0f) { r |= 0x80000000; }
    return r;
}

inline void mpGenIndex(mpWorld &world, u32 hash, ispc::vec3i &idx)
{
    const mpKernelParams &p = world.getKernelParams();
    mpTempParams &t = world.getTempParams();
    idx.x = hash & (p.world_div.x - 1);
    idx.z = (hash >> (t.world_div_bits.x)) & (p.world_div.z - 1);
    idx.y = (hash >> (t.world_div_bits.x + t.world_div_bits.z)) & (p.world_div.y - 1);
}



static const int g_particles_par_task = 2048;
static const int g_cells_par_task = 256;

mpWorld::mpWorld()
    : m_id_seed(0)
    , m_num_particles(0)
    , m_has_hithandler(false)
    , m_has_forcehandler(false)
    , m_num_particles_gpu(0)
    , m_num_particles_gpu_prev(0)
{
}

mpWorld::~mpWorld()
{
    endUpdate();
}

const mpKernelParams& mpWorld::getKernelParams() const  { return m_kparams; }

void mpWorld::setKernelParams(const mpKernelParams &v)
{
    m_kparams = v;

    if (m_kparams.max_particles != (int)m_particles.size()) {
        mpParticle blank;
        blank.lifetime = 0.0f;

        m_particles.resize(m_kparams.max_particles, blank);
        m_num_particles = std::min<int>(m_num_particles, (int)m_kparams.max_particles);
    }
}

mpTempParams& mpWorld::getTempParams()  { return m_tparams; }
const mpCellCont& mpWorld::getCells()   { return m_cells; }

void mpWorld::forceSetNumParticles(int v)
{
    v = std::min<int>(v, (int)m_kparams.max_particles);

    if (v > m_num_particles) {
        for (int i = m_num_particles; i < v; ++i) {
            //m_particles[i].lifetime = 1.0f;
        }
    }
    else if (v < m_num_particles) {
        for (int i = v; i < m_num_particles; ++i) {
            m_particles[i].lifetime = 0.0f;
        }
    }

    m_num_particles = std::min<int>(v, (int)m_kparams.max_particles);
}

int         mpWorld::getNumParticles() const { return m_num_particles; }
mpParticle* mpWorld::getParticles() { return m_particles.empty() ? nullptr : &m_particles[0]; }
int         mpWorld::getNumParticlesGPU() const { return m_num_particles_gpu; }
mpParticle* mpWorld::getParticlesGPU() { return m_particles_gpu.empty() ? nullptr : &m_particles_gpu[0]; }

mpParticleIM& mpWorld::getIntermediateData(int i) { return m_imd[i]; }
mpParticleIM& mpWorld::getIntermediateData() { return m_imd[m_current]; }

std::mutex& mpWorld::getMutex() { return m_mutex; }


void mpWorld::addParticles(mpParticle *p, size_t num)
{
    num = std::min<size_t>(num, m_kparams.max_particles - m_num_particles);
    for (int i = 0; i < (int)num; ++i) {
        m_particles[m_num_particles + i] = p[i];
    }
    if (m_kparams.id_as_float) {
        for (int i = 0; i < (int)num; ++i) {
            (float&)m_particles[m_num_particles + i].id = float(++m_id_seed);
        }
    }
    else {
        for (int i = 0; i < (int)num; ++i) {
            m_particles[m_num_particles + i].id = ++m_id_seed;
        }
    }
    m_num_particles += (int)num;
}

void mpWorld::addPlaneColliders(mpPlaneCollider *col, size_t num)
{
    m_plane_colliders.insert(m_plane_colliders.end(), col, col + num);
}

void mpWorld::addSphereColliders(mpSphereCollider *col, size_t num)
{
    m_sphere_colliders.insert(m_sphere_colliders.end(), col, col + num);
}

void mpWorld::addCapsuleColliders(mpCapsuleCollider *col, size_t num)
{
    m_capsule_colliders.insert(m_capsule_colliders.end(), col, col + num);
}

void mpWorld::addBoxColliders(mpBoxCollider *col, size_t num)
{
    m_box_colliders.insert(m_box_colliders.end(), col, col + num);
}


void mpWorld::removeCollider(mpColliderProperties &props)
{
    {
        auto i = std::lower_bound(m_box_colliders.begin(), m_box_colliders.end(), props.owner_id,
            [&](const mpBoxCollider &c, int i) { return c.props.owner_id < i; });
        if (i != m_box_colliders.end()) {
            i->props.hit_handler = nullptr;
            i->props.force_handler = nullptr;
            return;
        }
    }
    {
        auto i = std::lower_bound(m_sphere_colliders.begin(), m_sphere_colliders.end(), props.owner_id,
            [&](const mpSphereCollider &c, int i) { return c.props.owner_id < i; });
        if (i != m_sphere_colliders.end()) {
            i->props.hit_handler = nullptr;
            i->props.force_handler = nullptr;
            return;
        }
    }
    {
        auto i = std::lower_bound(m_capsule_colliders.begin(), m_capsule_colliders.end(), props.owner_id,
            [&](const mpCapsuleCollider &c, int i) { return c.props.owner_id < i; });
        if (i != m_capsule_colliders.end()) {
            i->props.hit_handler = nullptr;
            i->props.force_handler = nullptr;
            return;
        }
    }
    {
        auto i = std::lower_bound(m_plane_colliders.begin(), m_plane_colliders.end(), props.owner_id,
            [&](const mpPlaneCollider &c, int i) { return c.props.owner_id < i; });
        if (i != m_plane_colliders.end()) {
            i->props.hit_handler = nullptr;
            i->props.force_handler = nullptr;
            return;
        }
    }
}


void mpWorld::addForces(mpForce *force, size_t num)
{
    m_forces.insert(m_forces.end(), force, force + num);
}


inline ivec3 Position2Index(mpWorld &w, const vec3 &pos)
{
    const mpKernelParams &p = w.getKernelParams();
    mpTempParams &t = w.getTempParams();
    vec3 &bl = (vec3&)t.world_bounds_bl;
    vec3 &rcpCell = (vec3&)t.rcp_cell_size;
    int xb = clamp<i32>(i32((pos.x - bl.x)*rcpCell.x), 0, p.world_div.x - 1);
    int zb = clamp<i32>(i32((pos.z - bl.z)*rcpCell.z), 0, p.world_div.z - 1);
    int yb = clamp<i32>(i32((pos.y - bl.y)*rcpCell.y), 0, p.world_div.y - 1);
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
    if (cells.empty()) return;
    ivec3 bits = w.getTempParams().world_div_bits;
    mpParticle *particles = w.getParticles();
    for (int iy = imin.y; iy < imax.y; ++iy) {
        for (int iz = imin.z; iz < imax.z; ++iz) {
            for (int ix = imin.x; ix < imax.x; ++ix) {
                u32 ci = ix | (iz << bits.x) | (iy << (bits.x + bits.z));
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
    if (cells.empty()) return;
    ivec3 bits = w.getTempParams().world_div_bits;
    mpParticle *particles = w.getParticles();
    int lz = imax.z - imin.z;
    int ly = imax.y - imin.y;
    if (ly > 4) {
        ist::parallel_for(imin.y, imax.y, [&](int iy) {
            for (int iz = imin.z; iz < imax.z; ++iz) {
                for (int ix = imin.x; ix < imax.x; ++ix) {
                    u32 ci = ix | (iz << bits.x) | (iy << (bits.x + bits.z));
                    f(cells[ci], ivec3(ix, iy, iz));
                }
            }
        });
    }
    else if (lz > 4) {
        for (int iy = imin.y; iy < imax.y; ++iy) {
            ist::parallel_for(imin.z, imax.z, [&](int iz) {
                for (int ix = imin.x; ix < imax.x; ++ix) {
                    u32 ci = ix | (iz << bits.x) | (iy << (bits.x + bits.z));
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
    ScanCells(*this, imin, imax, [&](const mpCell &cell, const ivec3 &ci) {
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
    ScanCells(*this, imin, imax, [&](const mpCell &cell, const ivec3 &ci) {
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
    ScanCellsParallel(*this, imin, imax, [&](const mpCell &cell, const ivec3 &ci) {
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
    ScanCellsParallel(*this, imin, imax, [&](const mpCell &cell, const ivec3 &ci) {
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

void mpWorld::scanAll(mpHitHandler handler)
{
    for (int i = 0; i < m_num_particles; ++i) {
        handler(&m_particles[i]);
    }
}

void mpWorld::scanAllParallel(mpHitHandler handler)
{
    ist::parallel_for(0, m_num_particles, g_particles_par_task,
        [&](int i) {
            handler(&m_particles[i]);
        });
}

void mpWorld::moveAll(const vec3 &move)
{
    ist::parallel_for(0, m_num_particles, g_particles_par_task,
        [&](int i) {
            (vec3&)m_particles[i].position += move;
        });
}


void mpWorld::clearParticles()
{
    m_num_particles = 0;
    for (u32 i = 0; i < m_particles.size(); ++i) {
        m_particles[i].lifetime = 0.0f;
    }
}

void mpWorld::clearCollidersAndForces()
{
    m_plane_colliders.clear();
    m_sphere_colliders.clear();
    m_capsule_colliders.clear();
    m_box_colliders.clear();
    m_forces.clear();

    m_has_hithandler = false;
    m_has_forcehandler = false;
}


void mpWorld::update(float dt)
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
        tp.world_div_bits.x = msb(kp.world_div.x);
        tp.world_div_bits.y = msb(kp.world_div.y);
        tp.world_div_bits.z = msb(kp.world_div.z);
        cellsize = (wsize*2.0f / vec3((float)kp.world_div.x, (float)kp.world_div.y, (float)kp.world_div.z));
        cellsize_r = vec3(1.0f, 1.0f, 1.0f) / cellsize;
        bl = wpos - wsize;
        ur = wpos + wsize;

        vec3 &apos = (vec3&)kp.active_region_center;
        vec3 &asize = (vec3&)kp.active_region_extent;
        if (glm::dot(asize, vec3(1.0f)) == 0.0f) {
            apos = wpos;
            asize = wsize;
        }

        int reserve_size = mpParticlesEachLine * (ceildiv(kp.max_particles, mpParticlesEachLine));
        m_cells.resize(cell_num);
        m_particles.resize(kp.max_particles);
        m_imd.resize(kp.max_particles);
        m_particles_gpu.reserve(reserve_size);
        m_particles_gpu.resize(kp.max_particles);

        int num_soa_data_blocks = cell_num + std::max<int>((kp.max_particles - cell_num) / 8, 0);
        m_soa.resize(num_soa_data_blocks * 8);
    }

    mpCell              *ce = &m_cells[0];
    mpPlaneCollider     *planes = m_plane_colliders.empty() ? nullptr : &m_plane_colliders[0];
    mpSphereCollider    *spheres = m_sphere_colliders.empty() ? nullptr : &m_sphere_colliders[0];
    mpCapsuleCollider   *capsules = m_capsule_colliders.empty() ? nullptr : &m_capsule_colliders[0];
    mpBoxCollider       *boxes = m_box_colliders.empty() ? nullptr : &m_box_colliders[0];
    mpForce             *forces = m_forces.empty() ? nullptr : &m_forces[0];

    int num_colliders = int(m_plane_colliders.size() + m_sphere_colliders.size() + m_capsule_colliders.size() + m_box_colliders.size());

    {
        const float PI = 3.14159265359f;
        kp.RcpParticleSize2 = 1.0f / (m_kparams.particle_size*2.0f);
        kp.SPHDensityCoef = m_kparams.SPHParticleMass * 315.0f / (64.0f * PI * pow(m_kparams.particle_size, 9));
        kp.SPHGradPressureCoef = m_kparams.SPHParticleMass * -45.0f / (PI * pow(m_kparams.particle_size, 6));
        kp.SPHLapViscosityCoef = m_kparams.SPHParticleMass * m_kparams.SPHViscosity * 45.0f / (PI * pow(m_kparams.particle_size, 6));
    }

    mpKernelContext kcontext = {
        &kp, ce,
        &m_soa.pos_x[0], &m_soa.pos_y[0], &m_soa.pos_z[0],
        &m_soa.vel_x[0], &m_soa.vel_y[0], &m_soa.vel_z[0],
        &m_soa.acl_x[0], &m_soa.acl_y[0], &m_soa.acl_z[0],
        &m_soa.speed[0], &m_soa.density[0], &m_soa.affection[0], &m_soa.hit[0],
        planes, spheres, capsules, boxes, forces,
        (int)m_plane_colliders.size(), (int)m_sphere_colliders.size(), (int)m_capsule_colliders.size(), (int)m_box_colliders.size(), (int)m_forces.size()
    };

    // clear grid
    ist::parallel_for(0, cell_num, g_cells_par_task,
        [&](int i) {
            ce[i].begin = ce[i].end = 0;
        });

    // gen hash
    ist::parallel_for(0, m_num_particles, g_particles_par_task,
        [&](int i) {
            vec3 rel = glm::abs((vec3&)m_particles[i].position - (vec3&)kp.active_region_center);
            if (rel.x > kp.active_region_extent.x ||
                rel.y > kp.active_region_extent.y ||
                rel.z > kp.active_region_extent.z)
            {
                m_particles[i].lifetime = 0.0f;
            }
            m_particles[i].lifetime = std::max<f32>(m_particles[i].lifetime - dt, 0.0f);
            m_particles[i].hash = mpGenHash(*this, m_particles[i]);
        });

    // sort by hash
    ist::parallel_sort(&m_particles[0], &m_particles[0] + m_num_particles,
        [&](const mpParticle &a, const mpParticle &b) { return a.hash < b.hash; });

    // count num particles
    ist::parallel_for(0, m_num_particles, g_particles_par_task,
        [&](int i) {
            const u32 G_ID = i;
            u32 G_ID_PREV = G_ID - 1;
            u32 G_ID_NEXT = G_ID + 1;

            u32 cell = m_particles[G_ID].hash;
            u32 cell_prev = (G_ID_PREV == -1) ? -1 : m_particles[G_ID_PREV].hash;
            u32 cell_next = (G_ID_NEXT == kp.max_particles) ? -2 : m_particles[G_ID_NEXT].hash;
            if ((cell & 0x80000000) != 0) { // highest bit is live flag
                if ((cell_prev & 0x80000000) == 0) { // 
                    m_num_particles = G_ID;
                }
            }
            else {
                if (cell != cell_prev) {
                    ce[cell].begin = G_ID;
                }
                if (cell != cell_next) {
                    ce[cell].end = G_ID + 1;
                }
            }
        });
    if ((m_particles[0].hash & 0x80000000) != 0) {
        m_num_particles = 0;
    }

    {
        i32 soai = 0;
        for (int i = 0; i < cell_num; ++i) {
            ce[i].soai = soai;
            soai += soa_blocks(ce[i].end - ce[i].begin);
        }
    }

    // AoS -> SoA
    ist::parallel_for(0, cell_num, g_cells_par_task,
        [&](int i) {
            i32 n = ce[i].end - ce[i].begin;
            if (n == 0) { return; }
            mpSoAnize(ce[i], m_particles, m_soa);
        });

    if (m_kparams.solver_type == mpSolverType_Impulse) {
        // impulse
        ist::parallel_for(0, cell_num, g_cells_par_task,
            [&](int i) {
                i32 n = ce[i].end - ce[i].begin;
                if (n == 0) { return; }
                ispc::vec3i idx;
                mpGenIndex(*this, i, idx);
                if (kp.enable_interaction) {
                    ispc::impUpdatePressure(kcontext, idx);
                }
                if (kp.enable_forces) {
                    ispc::ProcessExternalForce(kcontext, idx);
                }
                if (kp.enable_colliders) {
                    ispc::ProcessColliders(kcontext, idx);
                }
            });
        ist::parallel_for(0, cell_num, g_cells_par_task,
            [&](int i) {
                i32 n = ce[i].end - ce[i].begin;
                if (n == 0) { return; }
                ispc::vec3i idx;
                mpGenIndex(*this, i, idx);
                ispc::Integrate(kcontext, idx);
            });
    }
    else if (m_kparams.solver_type == mpSolverType_SPH || m_kparams.solver_type == mpSolverType_SPHEst) {
        if (kp.enable_interaction && m_kparams.solver_type == mpSolverType_SPH) {
            ist::parallel_for(0, cell_num, g_cells_par_task,
                [&](int i) {
                    i32 n = ce[i].end - ce[i].begin;
                    if (n == 0) { return; }
                    ispc::vec3i idx;
                    mpGenIndex(*this, i, idx);
                    ispc::sphUpdateDensity(kcontext, idx);
                });
            ist::parallel_for(0, cell_num, g_cells_par_task,
                [&](int i) {
                    i32 n = ce[i].end - ce[i].begin;
                    if (n == 0) { return; }
                    ispc::vec3i idx;
                    mpGenIndex(*this, i, idx);
                    ispc::sphUpdateForce(kcontext, idx);
                });
        }
        else if (kp.enable_interaction && m_kparams.solver_type == mpSolverType_SPHEst) {
            ist::parallel_for(0, cell_num, g_cells_par_task,
                [&](int i) {
                i32 n = ce[i].end - ce[i].begin;
                    if (n == 0) { return; }
                    ispc::vec3i idx;
                    mpGenIndex(*this, i, idx);
                    ispc::sphUpdateDensityEst1(kcontext, idx);
                });
            ist::parallel_for(0, cell_num, g_cells_par_task,
                [&](int i) {
                    i32 n = ce[i].end - ce[i].begin;
                    if (n == 0) { return; }
                    ispc::vec3i idx;
                    mpGenIndex(*this, i, idx);
                    ispc::sphUpdateDensityEst2(kcontext, idx);
                });
            ist::parallel_for(0, cell_num, g_cells_par_task,
                [&](int i) {
                    i32 n = ce[i].end - ce[i].begin;
                    if (n == 0) { return; }
                    ispc::vec3i idx;
                    mpGenIndex(*this, i, idx);
                    ispc::sphUpdateForce(kcontext, idx);
                });
        }

        ist::parallel_for(0, cell_num, g_cells_par_task,
            [&](int i) {
                i32 n = ce[i].end - ce[i].begin;
                if (n == 0) { return; }
                ispc::vec3i idx;
                mpGenIndex(*this, i, idx);
                if (kp.enable_forces) {
                    ispc::ProcessExternalForce(kcontext, idx);
                }
                if (kp.enable_colliders) {
                    ispc::ProcessColliders(kcontext, idx);
                }
                ispc::Integrate(kcontext, idx);
            });
    }

    // SoA -> AoS
    ist::parallel_for(0, cell_num, g_cells_par_task,
        [&](int i) {
            i32 n = ce[i].end - ce[i].begin;
            if (n == 0) { return; }
            mpAoSnize(ce[i], m_soa, m_particles, m_imd);
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

void mpWorld::beginUpdate(float dt)
{
    m_taskgroup.run([=]() { update(dt); });
}

void mpWorld::endUpdate()
{
    m_taskgroup.wait();
}


void mpWorld::callHandlers()
{
    int num_colliders = 0;
    // search max id and allocate properties
    if (!m_plane_colliders.empty()) { num_colliders = std::max(num_colliders, m_plane_colliders.back().props.owner_id + 1); }
    if (!m_sphere_colliders.empty()) { num_colliders = std::max(num_colliders, m_sphere_colliders.back().props.owner_id + 1); }
    if (!m_capsule_colliders.empty()) { num_colliders = std::max(num_colliders, m_capsule_colliders.back().props.owner_id + 1); }
    if (!m_box_colliders.empty()) { num_colliders = std::max(num_colliders, m_box_colliders.back().props.owner_id + 1); }
    m_collider_properties.resize(num_colliders);

    for (auto &c : m_plane_colliders) { m_collider_properties[c.props.owner_id] = &c.props; }
    for (auto &c : m_sphere_colliders) { m_collider_properties[c.props.owner_id] = &c.props; }
    for (auto &c : m_capsule_colliders) { m_collider_properties[c.props.owner_id] = &c.props; }
    for (auto &c : m_box_colliders) { m_collider_properties[c.props.owner_id] = &c.props; }

    ist::parallel_invoke(
        [&]() {
            for (auto p : m_collider_properties) {
                if (p && p->hit_handler != nullptr) {
                    m_has_hithandler = true;
                    break;
                }
            }
        },
        [&]() {
            for (auto p : m_collider_properties) {
                if (p && p->force_handler != nullptr) {
                    m_has_forcehandler = true;
                    break;
                }
            }
        }
    );

    if (m_has_hithandler) {
        for (int i = 0; i < m_num_particles; ++i) {
            mpParticle &p = m_particles[i];
            if (p.hit != 0) {
                m_current = i;
                mpHitHandler handler = (mpHitHandler)(m_collider_properties[p.hit]->hit_handler);
                if (handler) { handler(&p); }
            }
        }
    }
    if (m_has_forcehandler) {

        m_pforce.resize(num_colliders);
        memset((void*)&m_pforce[0], 0, sizeof(mpParticleForce)*m_pforce.size());
        ist::parallel_for_blocked(0, m_num_particles, g_particles_par_task,
            [&](int begin, int end) {
                mpPForceCont &pf = m_pcombinable.local();
                pf.resize(num_colliders);
                for (int i = begin; i != end; ++i) {
                    mpParticle &p = m_particles[i];
                    if (p.hit != 0) {
                        (simdvec4&)pf[p.hit].position += (simdvec4&)p.position;
                        (simdvec4&)pf[p.hit].force += (simdvec4&)m_imd[i].accel;
                        ++pf[p.hit].num_hits;
                    }
                }
            });
        m_pcombinable.combine_each([&](const mpPForceCont &pf) {
            for (int i = 0; i < (int)pf.size(); ++i) {
                const mpParticleForce &h = pf[i];
                (simdvec4&)m_pforce[i].position += h.position;
                (simdvec4&)m_pforce[i].force += h.force;
                m_pforce[i].num_hits += h.num_hits;
            }
            memset((void*)&pf[0], 0, sizeof(mpParticleForce)*pf.size());
        });

        for (int i = 0; i < num_colliders; ++i) {
            if (m_pforce[i].num_hits == 0) continue;

            mpForceHandler handler = (mpForceHandler)m_collider_properties[i]->force_handler;
            if (handler) {
                (vec3&)m_pforce[i].position /= m_pforce[i].num_hits;
                handler(&m_pforce[i]);
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

int mpWorld::updateDataTexture(void *tex, int width, int height)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (g_mpRenderer && !m_particles_gpu.empty()) {
        int num_needs_copy = std::max<int>(m_num_particles_gpu, m_num_particles_gpu_prev);
        m_num_particles_gpu_prev = m_num_particles_gpu;
        g_mpRenderer->updateDataTexture(tex, width, height, &m_particles_gpu[0], sizeof(mpParticle)*m_kparams.max_particles);
    }
    return m_num_particles_gpu;
}
