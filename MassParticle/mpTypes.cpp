//#include "stdafx.h"
#include <algorithm>
#include "MassParticle.h"
#include "mpTypes.h"


mpKernelParams::mpKernelParams()
{
    (vec3&)world_center = vec3(0.0f, 0.0f, 0.0f);
    (vec3&)world_extent = vec3(10.24f, 10.24f, 10.24f);
    (vec3&)coord_scaler = vec3(1.0f, 1.0f, 1.0f);

    enable_interactions = 1;
    enable_colliders = 1;
    enable_forces = 1;

    solver_type = mpSolver_Impulse;
    lifetime = 30.0f;
    timestep = 0.01f;
    decelerate = 0.995f;
    pressure_stiffness = 500.0f;
    wall_stiffness = 1500.0f;
    max_particles = 200000;
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

        //// 不要
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
    , m_trail_enabled(false)
    , m_update_task(nullptr)
    , m_hitdata_needs_update(false)
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
    m_hitdata.resize(m_hitdata.size() + num);
}

void mpWorld::addSphereColliders(mpSphereCollider *col, int num)
{
    m_sphere_colliders.insert(m_sphere_colliders.end(), col, col+num);
    m_hitdata.resize(m_hitdata.size() + num);
}

void mpWorld::addCapsuleColliders(mpCapsuleCollider *col, int num)
{
    m_capsule_colliders.insert(m_capsule_colliders.end(), col, col+num);
    m_hitdata.resize(m_hitdata.size() + num);
}

void mpWorld::addBoxColliders(mpBoxCollider *col, int num)
{
    m_box_colliders.insert(m_box_colliders.end(), col, col+num);
    m_hitdata.resize(m_hitdata.size() + num);
}

void mpWorld::addForces(mpForce *force, int num)
{
    m_forces.insert(m_forces.end(), force, force+num);
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
}


class mpUpdateTask : public tbb::task
{
private:
    typedef std::function<void()> func_t;
    func_t func;
public:
    mpUpdateTask(const func_t &f) : func(f)
    {
        set_ref_count(2);
    }

    tbb::task* execute()
    {
        func();
        decrement_ref_count();
        return nullptr;
    }
};

void mpWorld::beginUpdate(float dt)
{
    m_update_task = new (tbb::task::allocate_root()) mpUpdateTask([=](){ update(dt); });
    tbb::task::enqueue(*m_update_task);
}

void mpWorld::endUpdate()
{
    if (m_update_task) {
        m_update_task->wait_for_all();
        m_update_task = nullptr;
    }
}

void mpWorld::update(float32 dt)
{
    if (m_num_particles == 0) { return; }

    m_hitdata_needs_update = true;
    mpKernelParams &kp = m_kparams;
    mpTempParams &tp = m_tparams;
    int cell_num = kp.world_div.x*kp.world_div.y*kp.world_div.z;

    {
        vec3 &wpos = (vec3&)kp.world_center;
        vec3 &wsize = (vec3&)kp.world_extent;
        vec3 &rcpCell = (vec3&)tp.rcp_cell_size;
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
        rcpCell = vec3(1.0f, 1.0f, 1.0f) / (wsize*2.0f / vec3((float)kp.world_div.x, (float)kp.world_div.y, (float)kp.world_div.z));
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
    m_hitdata.resize(num_colliders);


    ispc::UpdateConstants(m_kparams);

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

    // パーティクルを hash で sort
    tbb::parallel_sort(&m_particles[0], &m_particles[0]+m_num_particles, 
        [&](const mpParticle &a, const mpParticle &b) { return a.hash < b.hash; } );

    // パーティクルがどの grid に入っているかを算出
    tbb::parallel_for(tbb::blocked_range<int>(0, (int32)m_num_particles, g_particles_par_task),
        [&](const tbb::blocked_range<int> &r) {
            for(int i=r.begin(); i!=r.end(); ++i) {
                const uint32 G_ID = i;
                uint32 G_ID_PREV = G_ID-1;
                uint32 G_ID_NEXT = G_ID+1;

                uint32 cell = m_particles[G_ID].hash;
                uint32 cell_prev = (G_ID_PREV==-1) ? -1 : m_particles[G_ID_PREV].hash;
                uint32 cell_next = (G_ID_NEXT == kp.max_particles) ? -2 : m_particles[G_ID_NEXT].hash;
                if((cell & 0x80000000) != 0) { // 最上位 bit が立っていたら死んでいる扱い
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
                if (kp.enable_interactions) {
                    ispc::impUpdatePressure(
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi);
                }
                if (kp.enable_forces) {
                    ispc::ProcessExternalForce(
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi,
                        forces, (int32)m_forces.size());
                }
                if (kp.enable_colliders) {
                    ispc::ProcessColliders(
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
                    (ispc::Particle*)&m_particles_soa[0],
                    (ispc::ParticleIMData*)&m_imd_soa[0],
                    ce, xi, yi, zi);
            }
        });
    }
    else if (m_kparams.solver_type == mpSolver_SPH || m_kparams.solver_type == mpSolver_SPHEst) {
        if (kp.enable_interactions && m_kparams.solver_type == mpSolver_SPH) {
            tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, g_cells_par_task),
                [&](const tbb::blocked_range<int> &r) {
                for (int i = r.begin(); i != r.end(); ++i) {
                    int32 n = ce[i].end - ce[i].begin;
                    if (n == 0) { continue; }
                    int xi, yi, zi;
                    mpGenIndex(*this, i, xi, yi, zi);
                    ispc::sphUpdateDensity(
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
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi);
                }
            });
        }
        else if (kp.enable_interactions && m_kparams.solver_type == mpSolver_SPHEst) {
            tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, g_cells_par_task),
                [&](const tbb::blocked_range<int> &r) {
                for (int i = r.begin(); i != r.end(); ++i) {
                    int32 n = ce[i].end - ce[i].begin;
                    if (n == 0) { continue; }
                    int xi, yi, zi;
                    mpGenIndex(*this, i, xi, yi, zi);
                    ispc::sphUpdateDensityEst1(
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
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi,
                        forces, (int32)m_forces.size());
                }
                if (kp.enable_colliders) {
                    ispc::ProcessColliders(
                        (ispc::Particle*)&m_particles_soa[0],
                        (ispc::ParticleIMData*)&m_imd_soa[0],
                        ce, xi, yi, zi,
                        planes, (int32)m_plane_colliders.size(),
                        spheres, (int32)m_sphere_colliders.size(),
                        capsules, (int32)m_capsule_colliders.size(),
                        boxes, (int32)m_box_colliders.size());
                }
                ispc::Integrate(
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

int mpWorld::getNumHitData() const
{
    return (int)m_hitdata.size();
}

mpHitData* mpWorld::getHitData()
{
    if (m_hitdata_needs_update) {
        m_hitdata_needs_update = false;

        // reduce hit data

        int num_colliders = getNumHitData();
        if (num_colliders > 0) {
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
                mpHitData &h = m_hitdata[i];
                (simdvec4&)h.position /= simdvec4((float)h.num_hits);
            }
        }
    }
    return m_hitdata.empty() ? nullptr : &m_hitdata[0];
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
    const float s = 0.01f;
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
