//#include "stdafx.h"
#include <tbb/tbb.h>
#include <algorithm>
#include "mpTypes.h"


mpKernelParams::mpKernelParams()
{
	(XMFLOAT3&)WorldCenter_x = XMFLOAT3(0.0f, 0.0f, 0.0f);
	(XMFLOAT3&)WorldSize_x = XMFLOAT3(10.24f, 10.24f, 10.24f);
	(XMFLOAT3&)Scale_x = XMFLOAT3(1.0f, 1.0f, 1.0f);

	SolverType = mpSolver_Impulse;
	LifeTime = 3600.0f;
	Timestep = 0.01f;
	Decelerate = 0.995f;
	PressureStiffness = 500.0f;
	WallStiffness = 1500.0f;
	MaxParticles = 200000;
	ParticleSize = 0.08f;

	SPHRestDensity = 1000.0f;
	SPHParticleMass = 0.002f;
	SPHViscosity = 0.1f;
}


mpPerformanceCounter::mpPerformanceCounter()
{
	reset();
}

void mpPerformanceCounter::reset()
{
	::QueryPerformanceCounter(&m_start);
}

float mpPerformanceCounter::getElapsedSecond()
{
	LARGE_INTEGER freq;
	::QueryPerformanceCounter(&m_end);
	::QueryPerformanceFrequency(&freq);
	return ((float)(m_end.QuadPart - m_start.QuadPart) / (float)freq.QuadPart);
}

float mpPerformanceCounter::getElapsedMillisecond()
{
	return getElapsedSecond()*1000.0f;
}



mpWorld g_mpWorld;
mpRenderer *g_mpRenderer;


const int32 SIMD_LANES = 8;

template<class T>
T clamp(T v, T minv, T maxv)
{
	return std::min<T>(std::max<T>(v, minv), maxv);
}

int32 soa_blocks(int32 i)
{
	return (i/SIMD_LANES) + (i%SIMD_LANES>0 ? 1 : 0);
}

template<class T>
inline void simd_store(void *address, T v)
{
	_mm_store_ps((float*)address, (const simdvec4&)v);
}

void mpSoAnize( int32 num, const mpParticle *particles, ispc::Particle_SOA8 *out )
{
	int32 blocks = soa_blocks(num);
	for(int32 bi=0; bi<blocks; ++bi) {
		int32 i = SIMD_LANES*bi;
		ist::soavec34 soav;
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
		ist::soavec44 aos_pos[2] = {
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
		ist::soavec44 aos_vel[2] = {
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
			out[i+ei].params.density = particles[bi].density[ei];
			out[i+ei].params.hit_prev = out[i + ei].params.hit;
			out[i+ei].params.hit = particles[bi].hit[ei];
		}
	}
}

inline uint32 mpGenHash(mpWorld &world, const mpParticle &particle)
{
	const mpKernelParams &p = world.getKernelParams();
	mpTempParams &t = world.getTempParams();
	XMFLOAT3 &bl = (XMFLOAT3&)t.WorldBBBL;
	XMFLOAT3 &rcpCell = (XMFLOAT3&)t.RcpCellSize;
	XMFLOAT3 &ppos = (XMFLOAT3&)particle.position;

	uint32 r =  clamp<int32>(int32((ppos.x-bl.x)*rcpCell.x), 0, p.WorldDiv_x-1) |
			   (clamp<int32>(int32((ppos.z-bl.z)*rcpCell.z), 0, p.WorldDiv_z-1) << (t.WorldDivBits_x)) |
			   (clamp<int32>(int32((ppos.y-bl.y)*rcpCell.y), 0, p.WorldDiv_y-1) << (t.WorldDivBits_x+t.WorldDivBits_z)) ;

	if(particle.params.lifetime<=0.0f) { r |= 0x80000000; }
	return r;
}

inline void mpGenIndex(mpWorld &world, uint32 hash, int32 &xi, int32 &yi, int32 &zi)
{
	const mpKernelParams &p = world.getKernelParams();
	mpTempParams &t = world.getTempParams();
	xi =  hash & (p.WorldDiv_x - 1);
	zi = (hash >> (t.WorldDivBits_x)) & (p.WorldDiv_z - 1);
	yi = (hash >> (t.WorldDivBits_x+t.WorldDivBits_z)) & (p.WorldDiv_y - 1);
}


mpWorld::mpWorld()
	: m_num_active_particles(0)
{
	m_sphere_colliders.reserve(64);

	m_particles.resize(m_kparams.MaxParticles);
	clearParticles();
}

mpWorld::~mpWorld()
{
}

void mpWorld::addParticles(mpParticle *p, int num)
{
	num = std::min<uint32_t>(num, m_kparams.MaxParticles - m_num_active_particles);
	for (int i = 0; i<num; ++i) {
		m_particles[m_num_active_particles+i] = p[i];
		m_particles[m_num_active_particles+i].params.lifetime = m_kparams.LifeTime;
	}
	m_num_active_particles += num;
}

void mpWorld::addSphereColliders(ispc::SphereCollider *col, int num)
{
	m_sphere_colliders.insert(m_sphere_colliders.end(), col, col+num);
}

void mpWorld::addPlaneColliders(ispc::PlaneCollider *col, int num)
{
	m_plane_colliders.insert(m_plane_colliders.end(), col, col+num);
}

void mpWorld::addBoxColliders(ispc::BoxCollider *col, int num)
{
	m_box_colliders.insert(m_box_colliders.end(), col, col+num);
}

void mpWorld::addForces(ispc::Force *force, int num)
{
	m_forces.insert(m_forces.end(), force, force+num);
}

void mpWorld::clearParticles()
{
	for (uint32 i = 0; i < m_particles.size(); ++i) {
		m_particles[i].params.lifetime = 0.0f;
	}
}

void mpWorld::clearCollidersAndForces()
{
	m_sphere_colliders.clear();
	m_plane_colliders.clear();
	m_box_colliders.clear();
	m_forces.clear();
}

void mpWorld::getViewProjection(XMFLOAT4X4 &out_mat, XMFLOAT3 &out_camerapos)
{
	out_mat = m_viewproj;
	out_camerapos = m_camerapos;
}

void mpWorld::setViewProjection(const XMFLOAT4X4 &mat, const XMFLOAT3 &camerapos)
{
	m_viewproj = mat;
	m_camerapos = camerapos;
}


void mpWorld::update(float32 dt)
{
	mpKernelParams &p = m_kparams;
	mpTempParams &t = m_tparams;
	int cell_num = p.WorldDiv_x*p.WorldDiv_y*p.WorldDiv_z;
	int particles_par_task = 2048;
	int cells_par_task = 256;

	{
		XMFLOAT3 &wpos = (XMFLOAT3&)p.WorldCenter_x;
		XMFLOAT3 &wsize = (XMFLOAT3&)p.WorldSize_x;
		XMFLOAT3 &rcpCell = (XMFLOAT3&)t.RcpCellSize;
		XMFLOAT3 &bl = (XMFLOAT3&)t.WorldBBBL;
		XMFLOAT3 &ur = (XMFLOAT3&)t.WorldBBUR;
		p.ParticleSize = std::max<float>(p.ParticleSize, 0.00001f);
		p.MaxParticles = std::max<int>(p.MaxParticles, 128);
		m_num_active_particles = std::min<int>(m_num_active_particles, p.MaxParticles);
		p.WorldDiv_x = std::max<int>(p.WorldDiv_x, 1);
		p.WorldDiv_y = std::max<int>(p.WorldDiv_y, 1);
		p.WorldDiv_z = std::max<int>(p.WorldDiv_z, 1);
		t.WorldDivBits_x = mpMSB(p.WorldDiv_x);
		t.WorldDivBits_y = mpMSB(p.WorldDiv_y);
		t.WorldDivBits_z = mpMSB(p.WorldDiv_z);
		rcpCell = XMFLOAT3(1.0f, 1.0f, 1.0f) / (wsize*2.0f / XMFLOAT3((float)p.WorldDiv_x, (float)p.WorldDiv_y, (float)p.WorldDiv_z));
		bl = wpos - wsize;
		ur = wpos + wsize;

		int SOADataNum = std::max<int>(cell_num, p.MaxParticles / 8);
		m_cells.resize(cell_num);
		m_particles.resize(p.MaxParticles);
		m_particles_soa.resize(SOADataNum);
		m_imd.resize(SOADataNum);
	}

	mpGridData			*ce = &m_cells[0];
	mpForce				*fs = m_forces.empty() ? nullptr : &m_forces[0];
	mpSphereCollider	*point_c = m_sphere_colliders.empty() ? nullptr : &m_sphere_colliders[0];
	mpPlaneCollider		*plane_c = m_plane_colliders.empty() ? nullptr : &m_plane_colliders[0];
	mpBoxCollider		*box_c = m_box_colliders.empty() ? nullptr : &m_box_colliders[0];

	ispc::sphUpdateConstants(m_kparams);

	// clear grid
	tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, cells_par_task),
		[&](const tbb::blocked_range<int> &r) {
			for(int i=r.begin(); i!=r.end(); ++i) {
				ce[i].begin = ce[i].end = 0;
			}
		});

	// gen hash
	tbb::parallel_for(tbb::blocked_range<int>(0, (int32)m_num_active_particles, particles_par_task),
		[&](const tbb::blocked_range<int> &r) {
			for(int i=r.begin(); i!=r.end(); ++i) {
				XMFLOAT3 &ppos = (XMFLOAT3&)m_particles[i].position;
				XMFLOAT3 &bl = m_tparams.WorldBBBL;
				XMFLOAT3 &ur = m_tparams.WorldBBUR;
				if (ppos.x<bl.x || ppos.y<bl.y || ppos.z<bl.z ||
					ppos.x>ur.x || ppos.y>ur.y || ppos.z>ur.z)
				{
					m_particles[i].params.lifetime = std::min<float>(m_particles[i].params.lifetime, 20.0f);
				}
				m_particles[i].params.lifetime = std::max<float32>(m_particles[i].params.lifetime - dt, 0.0f);
				m_particles[i].params.hash = mpGenHash(*this, m_particles[i]);
			}
		});

	// パーティクルを hash で sort
	tbb::parallel_sort(&m_particles[0], &m_particles[0]+m_num_active_particles, 
		[&](const mpParticle &a, const mpParticle &b) { return a.params.hash < b.params.hash; } );

	// パーティクルがどの grid に入っているかを算出
	tbb::parallel_for(tbb::blocked_range<int>(0, (int32)m_num_active_particles, particles_par_task),
		[&](const tbb::blocked_range<int> &r) {
			for(int i=r.begin(); i!=r.end(); ++i) {
				const uint32 G_ID = i;
				uint32 G_ID_PREV = G_ID-1;
				uint32 G_ID_NEXT = G_ID+1;

				uint32 cell = m_particles[G_ID].params.hash;
				uint32 cell_prev = (G_ID_PREV==-1) ? -1 : m_particles[G_ID_PREV].params.hash;
				uint32 cell_next = (G_ID_NEXT == p.MaxParticles) ? -2 : m_particles[G_ID_NEXT].params.hash;
				if((cell & 0x80000000) != 0) { // 最上位 bit が立っていたら死んでいる扱い
					if((cell_prev & 0x80000000) == 0) { // 
						m_num_active_particles = G_ID;
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
		if( (m_particles[0].params.hash & 0x80000000) != 0 ) {
			m_num_active_particles = 0;
		}
		else if ((m_particles[p.MaxParticles - 1].params.hash & 0x80000000) == 0) {
			m_num_active_particles = p.MaxParticles;
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
	tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, cells_par_task),
		[&](const tbb::blocked_range<int> &r) {
			for(int i=r.begin(); i!=r.end(); ++i) {
				int32 n = ce[i].end - ce[i].begin;
				if(n == 0) { continue; }
				mpParticle *p = &m_particles[ce[i].begin];
				ispc::Particle_SOA8 *t = &m_particles_soa[ce[i].soai];
				mpSoAnize(n, p, t);
			}
	});

	if (m_kparams.SolverType == mpSolver_Impulse) {
		// impulse
		tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, cells_par_task),
			[&](const tbb::blocked_range<int> &r) {
			for (int i = r.begin(); i != r.end(); ++i) {
				int32 n = ce[i].end - ce[i].begin;
				if (n == 0) { continue; }
				int xi, yi, zi;
				mpGenIndex(*this, i, xi, yi, zi);
				ispc::impUpdateVelocity(
					(ispc::Particle*)&m_particles_soa[0],
					(ispc::ParticleIMData*)&m_imd[0],
					ce, xi, yi, zi);
			}
		});
		tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, cells_par_task),
			[&](const tbb::blocked_range<int> &r) {
			for (int i = r.begin(); i != r.end(); ++i) {
				int32 n = ce[i].end - ce[i].begin;
				if (n == 0) { continue; }
				int xi, yi, zi;
				mpGenIndex(*this, i, xi, yi, zi);
				ispc::sphProcessExternalForce(
					(ispc::Particle*)&m_particles_soa[0],
					(ispc::ParticleIMData*)&m_imd[0],
					ce, xi, yi, zi,
					fs, (int32)m_forces.size());
				ispc::sphProcessCollision(
					(ispc::Particle*)&m_particles_soa[0],
					(ispc::ParticleIMData*)&m_imd[0],
					ce, xi, yi, zi,
					point_c, (int32)m_sphere_colliders.size(),
					plane_c, (int32)m_plane_colliders.size(),
					box_c, (int32)m_box_colliders.size());
				ispc::impIntegrate(
					(ispc::Particle*)&m_particles_soa[0],
					(ispc::ParticleIMData*)&m_imd[0],
					ce, xi, yi, zi);
			}
		});
	}
	else if (m_kparams.SolverType == mpSolver_SPH || m_kparams.SolverType == mpSolver_SPHEst) {
		if (m_kparams.SolverType == mpSolver_SPH) {
			tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, cells_par_task),
				[&](const tbb::blocked_range<int> &r) {
				for (int i = r.begin(); i != r.end(); ++i) {
					int32 n = ce[i].end - ce[i].begin;
					if (n == 0) { continue; }
					int xi, yi, zi;
					mpGenIndex(*this, i, xi, yi, zi);
					ispc::sphUpdateDensity(
						(ispc::Particle*)&m_particles_soa[0],
						(ispc::ParticleIMData*)&m_imd[0],
						ce, xi, yi, zi);
				}
			});
		}
		else if (m_kparams.SolverType == mpSolver_SPHEst) {
			tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, cells_par_task),
				[&](const tbb::blocked_range<int> &r) {
				for (int i = r.begin(); i != r.end(); ++i) {
					int32 n = ce[i].end - ce[i].begin;
					if (n == 0) { continue; }
					int xi, yi, zi;
					mpGenIndex(*this, i, xi, yi, zi);
					ispc::sphUpdateDensityEst1(
						(ispc::Particle*)&m_particles_soa[0],
						(ispc::ParticleIMData*)&m_imd[0],
						ce, xi, yi, zi);
				}
			});
			tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, cells_par_task),
				[&](const tbb::blocked_range<int> &r) {
				for (int i = r.begin(); i != r.end(); ++i) {
					int32 n = ce[i].end - ce[i].begin;
					if (n == 0) { continue; }
					int xi, yi, zi;
					mpGenIndex(*this, i, xi, yi, zi);
					ispc::sphUpdateDensityEst2(
						(ispc::Particle*)&m_particles_soa[0],
						(ispc::ParticleIMData*)&m_imd[0],
						ce, xi, yi, zi);
				}
			});
		}

		tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, cells_par_task),
			[&](const tbb::blocked_range<int> &r) {
			for (int i = r.begin(); i != r.end(); ++i) {
				int32 n = ce[i].end - ce[i].begin;
				if (n == 0) { continue; }
				int xi, yi, zi;
				mpGenIndex(*this, i, xi, yi, zi);
				ispc::sphUpdateForce(
					(ispc::Particle*)&m_particles_soa[0],
					(ispc::ParticleIMData*)&m_imd[0],
					ce, xi, yi, zi);
			}
		});
		tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, cells_par_task),
			[&](const tbb::blocked_range<int> &r) {
			for (int i = r.begin(); i != r.end(); ++i) {
				int32 n = ce[i].end - ce[i].begin;
				if (n == 0) { continue; }
				int xi, yi, zi;
				mpGenIndex(*this, i, xi, yi, zi);
				ispc::sphProcessExternalForce(
					(ispc::Particle*)&m_particles_soa[0],
					(ispc::ParticleIMData*)&m_imd[0],
					ce, xi, yi, zi,
				   fs, (int32)m_forces.size());
				ispc::sphProcessCollision(
					(ispc::Particle*)&m_particles_soa[0],
					(ispc::ParticleIMData*)&m_imd[0],
					ce, xi, yi, zi,
					&m_sphere_colliders[0], (int32)m_sphere_colliders.size(),
					&m_plane_colliders[0], (int32)m_plane_colliders.size(),
					&m_box_colliders[0], (int32)m_box_colliders.size());
				ispc::sphIntegrate(
					(ispc::Particle*)&m_particles_soa[0],
					(ispc::ParticleIMData*)&m_imd[0],
					ce, xi, yi, zi);
			}
		});
	}
	else if (m_kparams.SolverType==mpSolver_NoInteraction) {
		tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, cells_par_task),
			[&](const tbb::blocked_range<int> &r) {
			for (int i = r.begin(); i != r.end(); ++i) {
				int32 n = ce[i].end - ce[i].begin;
				if (n == 0) { continue; }
				int xi, yi, zi;
				mpGenIndex(*this, i, xi, yi, zi);
				ispc::impZeroAccel(
					(ispc::Particle*)&m_particles_soa[0],
					(ispc::ParticleIMData*)&m_imd[0],
					ce, xi, yi, zi);
				ispc::sphProcessExternalForce(
					(ispc::Particle*)&m_particles_soa[0],
					(ispc::ParticleIMData*)&m_imd[0],
					ce, xi, yi, zi,
					fs, (int32)m_forces.size());
				ispc::sphProcessCollision(
					(ispc::Particle*)&m_particles_soa[0],
					(ispc::ParticleIMData*)&m_imd[0],
					ce, xi, yi, zi,
					point_c, (int32)m_sphere_colliders.size(),
					plane_c, (int32)m_plane_colliders.size(),
					box_c, (int32)m_box_colliders.size());
				ispc::impIntegrate(
					(ispc::Particle*)&m_particles_soa[0],
					(ispc::ParticleIMData*)&m_imd[0],
					ce, xi, yi, zi);
			}
		});
	}

	// SoA -> AoS
	tbb::parallel_for(tbb::blocked_range<int>(0, cell_num, cells_par_task),
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
}
