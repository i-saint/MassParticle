// コンパイルは↓みたいなカスタムビルドステップで。
// ispc %(FullPath) -o $(TargetDir)%(Filename).obj -h $(TargetDir)%(Filename)_ispc.h --target=sse2,sse4,avx --arch=x86-64 --opt=fast-masked-vload --opt=fast-math
// 
// Intel SPMD はこちら
// http://ispc.github.com/


#include "ispc_vectormath.h"
#include "mpCollision.h"

static uniform vec3  mpWorldCenter;
static uniform vec3  mpWorldExtent;
static uniform ivec3 mpWorldDiv;
static uniform vec3  mpCoordScale;

static uniform float mpTimestep;
static uniform float mpDecelerate;
static uniform float mpPressureStiffness;
static uniform float mpWallStiffness;
static uniform float mpParticleSize;
static uniform float mpRcpParticleSize2;

static uniform float mpSPHRestDensity;
static uniform float mpSPHParticleMass;
static uniform float mpSPHViscosity;
static uniform float mpSPHDensityCoef;
static uniform float mpSPHGradPressureCoef;
static uniform float mpSPHLapViscosityCoef;


export void UpdateConstants(uniform KernelParams &params)
{
	store3(mpWorldCenter,x,y,z, params.world_center);
	store3(mpWorldExtent,x,y,z, params.world_extent);
	store3(mpWorldDiv,x,y,z, params.world_div);
	store3(mpCoordScale,x,y,z, params.coord_scaler);
	mpParticleSize		= params.particle_size;
	mpRcpParticleSize2	= 1.0f / (mpParticleSize*2.0f);

	mpTimestep			= params.timestep;
	mpDecelerate		= params.decelerate;
	mpPressureStiffness	= params.pressure_stiffness;
	mpWallStiffness		= params.wall_stiffness;

	mpSPHRestDensity	= params.SPHRestDensity;
	mpSPHParticleMass	= params.SPHParticleMass;
	mpSPHViscosity		= params.SPHViscosity;

	mpSPHDensityCoef		= mpSPHParticleMass * 315.0f / (64.0f * PI * pow(mpParticleSize, 9));
	mpSPHGradPressureCoef	= mpSPHParticleMass * -45.0f / (PI * pow(mpParticleSize, 6));
	mpSPHLapViscosityCoef	= mpSPHParticleMass * mpSPHViscosity * 45.0f / (PI * pow(mpParticleSize, 6));
}

export void GetConstants(uniform KernelParams &params)
{
	store3(params.world_center,x,y,z, mpWorldCenter);
	store3(params.world_extent,x,y,z, mpWorldExtent);
	store3(params.world_div,x,y,z, mpWorldDiv);
	store3(params.coord_scaler,x,y,z, mpCoordScale);

	params.timestep         = mpTimestep;
	params.decelerate       = mpDecelerate;
	params.pressure_stiffness= mpPressureStiffness;
	params.wall_stiffness    = mpWallStiffness;

	params.SPHRestDensity   = mpSPHRestDensity;
	params.SPHParticleMass  = mpSPHParticleMass;
	params.SPHViscosity     = mpSPHViscosity;
}


vec3 ComputeGridBox(uniform int xi, uniform int yi, uniform int zi, uniform vec3 &o_bl, uniform vec3 &o_ur)
{
	uniform vec3 cell_size = mpWorldExtent*2.0f / mpWorldDiv;
	o_bl.x = mpWorldCenter.x - mpWorldExtent.x + cell_size.x*xi;
	o_bl.y = mpWorldCenter.y - mpWorldExtent.y + cell_size.y*yi;
	o_bl.z = mpWorldCenter.z - mpWorldExtent.z + cell_size.z*zi;
	o_ur.x = mpWorldCenter.x - mpWorldExtent.x + cell_size.x*(xi+1);
	o_ur.y = mpWorldCenter.y - mpWorldExtent.y + cell_size.y*(yi+1);
	o_ur.z = mpWorldCenter.z - mpWorldExtent.z + cell_size.z*(zi+1);
}

#define repulse(p, f, n, d, props)\
	{\
		p.hit = props.owner_id;\
		vec3 accel = gather3(f,ax,ay,az);\
		accel += n * (-d * mpWallStiffness);\
		store3(f,ax,ay,az, accel);\
	}


bool IsGridOverrapedAABB(uniform int xi, uniform int yi, uniform int zi, uniform const BoundingBox &bb)
{
	uniform vec3 grid_bl;
	uniform vec3 grid_ur;
	ComputeGridBox(xi,yi,zi, grid_bl, grid_ur);
	uniform vec3 bb_bl = to_vec3(bb.bl);
	uniform vec3 bb_ur = to_vec3(bb.ur);
	if( grid_ur.x < bb_bl.x || grid_bl.x > bb_ur.x ||
		grid_ur.y < bb_bl.y || grid_bl.y > bb_ur.y ||
		grid_ur.z < bb_bl.z || grid_bl.z > bb_ur.z )
	{
		return false;
	}
	return true;
}

export void ProcessColliders(
	soa<8> Particle all_particles[],
	soa<8> ParticleIMData imds[],
	Cell uniform grid[],
	uniform int xi, uniform int yi, uniform int zi,
	uniform PlaneCollider planes[], uniform int num_planes,
	uniform SphereCollider spheres[], uniform int num_spheres,
	uniform CapsuleCollider capsules[], uniform int num_capsules,
	uniform BoxCollider boxes[], uniform int num_boxes )
{
	uniform const Cell &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
	uniform const int particle_num = gd.end - gd.begin;
	soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
	soa<8> ParticleIMData * uniform imd = &imds[gd.soai*8];

	uniform float particle_radius = mpParticleSize;

	// 適切に関数に分けたかったが、struct の参照渡しができないとか謎のコンパイラクラッシュとかで断念。
	// 全部 inline で書きます。

	// Plane
	for(uniform int s=0; s<num_planes; ++s) {
		uniform const PlaneCollider &col = planes[s];
		uniform const Plane &shape = col.shape;
		if(!IsGridOverrapedAABB(xi,yi,zi, col.bounds)) { continue; }

		uniform const vec3 plane_normal = to_vec3(shape.normal);
		uniform const float plane_distance = shape.distance;
		foreach(i=0 ... particle_num) {
			vec3 particles_pos = to_vec3(particles[i]);
			float distance = dot3(particles_pos, plane_normal) + plane_distance;
			if(distance < 0.0f) {
				repulse(particles[i], imd[i], plane_normal, distance, col.props);
			}
		}
	}

	// Sphere
	for(uniform int s=0; s<num_spheres; ++s) {
		uniform const SphereCollider &col = spheres[s];
		uniform const Sphere &shape = col.shape;
		if(!IsGridOverrapedAABB(xi,yi,zi, col.bounds)) { continue; }

		uniform const vec3 sphere_pos = to_vec3(shape.center);
		uniform const float sphere_radius = shape.radius;
		foreach(i=0 ... particle_num) {
			vec3 particles_pos = to_vec3(particles[i]);
			vec3 diff = particles_pos-sphere_pos;
			float len = length3(diff);
			float distance = len - sphere_radius;
			if(distance <= 0.0f) {
				vec3 dir = diff / len;
				repulse(particles[i], imd[i], dir, distance, col.props);
			}
		}
	}
	
	// Capsules
	for(uniform int s=0; s<num_capsules; ++s) {
		uniform const CapsuleCollider &col = capsules[s];
		uniform const Capsule &shape = col.shape;
		if(!IsGridOverrapedAABB(xi,yi,zi, col.bounds)) { continue; }

		uniform const vec3 pos1 = to_vec3(shape.pos1);
		uniform const vec3 pos2 = to_vec3(shape.pos2);
		uniform const float radius = shape.radius;
		uniform float rcp_lensq = shape.rcp_lensq;
		foreach(i=0 ... particle_num) {
			vec3 particles_pos = to_vec3(particles[i]);
			const float t = dot3(particles_pos-pos1, pos2-pos1) * rcp_lensq;
			vec3 diff;
			if(t<=0.0f) {
				diff = particles_pos-pos1;
			}
			else if(t>=1.0f) {
				diff = particles_pos-pos2;
			}
			else {
				vec3 nearest = pos1 + (pos2-pos1)*t;
				diff = particles_pos-nearest;
			}
			float len = length3(diff);
			float distance = len - radius;
			if(distance <= 0.0f) {
				vec3 dir = diff / len;
				repulse(particles[i], imd[i], dir, distance, col.props);
			}
		}
	}

	// Box
	for(uniform int s=0; s<num_boxes; ++s) {
		uniform const BoxCollider &col = boxes[s];
		uniform const Box &shape = col.shape;
		if(!IsGridOverrapedAABB(xi,yi,zi, col.bounds)) { continue; }

		uniform vec3 box_pos = to_vec3(shape.center);
		foreach(i=0 ... particle_num) {
			vec3 particles_pos = to_vec3(particles[i]);

			int inside = 0;
			float closest_distance = -9999.0f;
			vec3 closest_normal;
			vec3 ppos = particles_pos - box_pos;
			for(uniform int p=0; p<6; ++p) {
				uniform const vec3 plane_normal = to_vec3(shape.planes[p].normal);
				uniform const float plane_distance = shape.planes[p].distance;
				float distance = dot3(ppos, plane_normal) + plane_distance;
				if(distance < 0.0f) {
					inside++;
					if(distance > closest_distance) {
						closest_distance = distance;
						closest_normal = plane_normal;
					}
				}
			}
			if(inside==6) {
				repulse(particles[i], imd[i], closest_normal, closest_distance, col.props);
			}
		}
	}
}
#undef repulse


export void ProcessExternalForce(
	soa<8> Particle all_particles[],
	soa<8> ParticleIMData imds[],
	Cell uniform grid[],
	uniform int xi, uniform int yi, uniform int zi,
	uniform Force forces[], uniform int num_forces )
{
	uniform const Cell &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
	uniform const int particle_num = gd.end - gd.begin;
	soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
	soa<8> ParticleIMData * uniform imd = &imds[gd.soai*8];

	uniform float particle_radius = mpParticleSize;

	for(uniform int fi=0; fi<num_forces; ++fi) {
		uniform const Force &force = forces[fi];
		uniform const ForceProperties &props = force.props;

		if(props.shape_type==FS_AffectAll) {
			foreach(i=0 ... particle_num) {
				imd[i].inside_force = 1.0f;
			}
		}
		else if(props.shape_type==FS_Sphere) {
			if(!IsGridOverrapedAABB(xi,yi,zi, force.bounds)) { continue; }

			uniform const Sphere &sphere = force.sphere;
			float radius_sq = sphere.radius * sphere.radius;
			vec3 center = to_vec3(sphere.center);
			foreach(i=0 ... particle_num) {
				vec3 particles_pos = to_vec3(particles[i]);
				vec3 diff = particles_pos - center;
				float distance_sq = length_sq3(diff);
				imd[i].inside_force = distance_sq<=radius_sq ? 1.0f : 0.0f;
			}
		}
		else if(props.shape_type==FS_Capsule) {
			if(!IsGridOverrapedAABB(xi,yi,zi, force.bounds)) { continue; }

			// todo
		}
		else if(props.shape_type==FS_Box) {
			if(!IsGridOverrapedAABB(xi,yi,zi, force.bounds)) { continue; }

			uniform const Box &box = force.box;
			uniform vec3 box_pos = to_vec3(box.center);
			foreach(i=0 ... particle_num) {
				vec3 particles_pos = to_vec3(particles[i]);
				int inside = 0;
				vec3 ppos = particles_pos - box_pos;
				for(uniform int p=0; p<6; ++p) {
					uniform const vec3 plane_normal = to_vec3(box.planes[p].normal);
					uniform const float plane_distance = box.planes[p].distance;
					float distance = dot3(ppos, plane_normal) + plane_distance;
					if(distance < 0.0f) {
						inside++;
					}
				}
				imd[i].inside_force = inside==6 ? 1.0f : 0.0f;
			}
		}

		if(props.dir_type==FD_Directional) {
			foreach(i=0 ... particle_num) {
				vec3 dir = to_vec3(props.directional_dir);
				vec3 accel = gather3(imd[i], ax,ay,az);
				float affection = 1.0f;
				accel += dir * (lerp(props.strength_far, props.strength_near, pow(affection, props.attenuation_exp)) * imd[i].inside_force);
				store3(imd[i],ax,ay,az, accel);
			}
		}
		else if(props.dir_type==FD_Radial) {
			vec3 center = to_vec3(props.radial_center);
			foreach(i=0 ... particle_num) {
				vec3 particles_pos = to_vec3(particles[i]);
				vec3 diff = particles_pos - center;
				vec3 dir = normalize3(diff);
				vec3 accel = gather3(imd[i], ax,ay,az);
				float affection = 1.0f;
				accel += dir * (lerp(props.strength_far, props.strength_near, pow(affection, props.attenuation_exp)) * imd[i].inside_force);
				store3(imd[i],ax,ay,az, accel);
			}
		}
		else if(props.dir_type==FD_RadialCapsule) {
			// todo
		}
		else if(props.dir_type==FD_Vortex) {
			// todo
		}
		else if(props.dir_type==FD_Spline) {
			// todo
		}
		else if(props.dir_type==FD_VectorField) {
			// todo
		}
	}
}




static inline float sphComputeDensity(
	vec3 pos1,
	vec3 pos2 )
{
	uniform const float h_sq = mpParticleSize * mpParticleSize;
	vec3 diff = pos2 - pos1;
	float r_sq = dot3(diff, diff);
	if(r_sq < h_sq) {
		// Implements this equation:
		// W_poly6(r, h) = 315 / (64 * pi * h^9) * (h^2 - r^2)^3
		// g_fDensityCoef = fParticleMass * 315.0f / (64.0f * PI * fSmoothlen^9)
		return mpSPHDensityCoef * (h_sq - r_sq) * (h_sq - r_sq) * (h_sq - r_sq);
	}
	return 0.0f;
}

export void sphUpdateDensity(
	soa<8> Particle all_particles[],
	soa<8> ParticleIMData imds[],
	Cell uniform grid[],
	uniform int xi, uniform int yi, uniform int zi )
{
	uniform const Cell &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
	uniform const int particle_num = gd.end - gd.begin;
	soa<8> Particle * uniform particles = &all_particles[gd.soai*8];

	uniform const int nx_beg = max(xi-1, 0);
	uniform const int nx_end = min(xi+1, mpWorldDiv.x-1);
	uniform const int ny_beg = max(yi-1, 0);
	uniform const int ny_end = min(yi+1, mpWorldDiv.y-1);
	uniform const int nz_beg = max(zi-1, 0);
	uniform const int nz_end = min(zi+1, mpWorldDiv.z-1);

	for(uniform int i=0; i<particle_num; ++i) {
		uniform vec3 pos1 = to_vec3(particles[i]);
		float density = 0.0f;
		for(uniform int nyi=ny_beg; nyi<=ny_end; ++nyi) {
			for(uniform int nzi=nz_beg; nzi<=nz_end; ++nzi) {
				for(uniform int nxi=nx_beg; nxi<=nx_end; ++nxi) {
					uniform const Cell &ngd = grid[mpWorldDiv.x*mpWorldDiv.z*nyi + mpWorldDiv.x*nzi + nxi];
					uniform const int neighbor_num = ngd.end - ngd.begin;
					soa<8> Particle * uniform neighbors = &all_particles[ngd.soai*8];
					foreach(t=0 ... neighbor_num) {
						vec3 pos2 = to_vec3(neighbors[t]);
						density += sphComputeDensity(pos1, pos2);
					}
				}
			}
		}
		particles[i].density = reduce_add(density);
	}
}


export void sphUpdateDensityEst1(
	soa<8> Particle all_particles[],
	soa<8> ParticleIMData imds[],
	Cell uniform grid[],
	uniform int xi, uniform int yi, uniform int zi )
{
	uniform Cell &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
	uniform const int particle_num = gd.end - gd.begin;
	soa<8> Particle * uniform particles = &all_particles[gd.soai*8];

	for(uniform int i=0; i<particle_num; ++i) {
		uniform vec3 pos1 = to_vec3(particles[i]);
		float density = 0.0f;

		uniform const Cell &ngd = gd;
		uniform const int neighbor_num = ngd.end - ngd.begin;
		soa<8> Particle * uniform neighbors = &all_particles[ngd.soai*8];
		foreach(t=0 ... neighbor_num) {
			vec3 pos2 = to_vec3(neighbors[t]);
			density += sphComputeDensity(pos1, pos2);
		}
		particles[i].density = reduce_add(density);
	}
	{
		gd.density = 0.0f;
		float total_density = 0.0f;
		foreach(i=0 ... particle_num) {
			total_density += particles[i].density;
		}
		gd.density = reduce_add(total_density) / particle_num;
	}
}

export void sphUpdateDensityEst2(
	soa<8> Particle all_particles[],
	soa<8> ParticleIMData imds[],
	Cell uniform grid[],
	uniform int xi, uniform int yi, uniform int zi )
{
	uniform const Cell &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
	uniform const int particle_num = gd.end - gd.begin;
	soa<8> Particle * uniform particles = &all_particles[gd.soai*8];

	uniform const int nx_beg = max(xi-1, 0);
	uniform const int nx_end = min(xi+1, mpWorldDiv.x-1);
	uniform const int ny_beg = max(yi-1, 0);
	uniform const int ny_end = min(yi+1, mpWorldDiv.y-1);
	uniform const int nz_beg = max(zi-1, 0);
	uniform const int nz_end = min(zi+1, mpWorldDiv.z-1);

	for(uniform int nyi=ny_beg; nyi<=ny_end; ++nyi) {
		for(uniform int nzi=nz_beg; nzi<=nz_end; ++nzi) {
			for(uniform int nxi=nx_beg; nxi<=nx_end; ++nxi) {
				uniform const Cell &ngd = grid[mpWorldDiv.x*mpWorldDiv.z*nyi + mpWorldDiv.x*nzi + nxi];
				foreach(i=0 ... particle_num) {
					particles[i].density += ngd.density*0.05f;
				}
			}
		}
	}
}


static inline float sphCalculatePressure(float density)
{
	// Implements this equation:
	// Pressure = B * ((rho / rho_0)^y  - 1)
	return mpPressureStiffness * max(pow(density / mpSPHRestDensity, 3) - 1.0f, 0.0f);
}
static inline uniform float sphCalculatePressure(uniform float density)
{
	return mpPressureStiffness * max(pow(density / mpSPHRestDensity, 3) - 1.0f, 0.0f);
}


static inline vec3 sphCalculateGradPressure(float r, float P_pressure, float N_pressure, float N_density, vec3 diff)
{
	const float h = mpParticleSize;
	float avg_pressure = 0.5f * (N_pressure + P_pressure);
	// Implements this equation:
	// W_spkiey(r, h) = 15 / (pi * h^6) * (h - r)^3
	// GRAD( W_spikey(r, h) ) = -45 / (pi * h^6) * (h - r)^2
	// g_fGradPressureCoef = fParticleMass * -45.0f / (PI * fSmoothlen^6)
	return (mpSPHGradPressureCoef * avg_pressure / N_density * (h - r) * (h - r) / r) * diff;
}

static inline vec3 sphCalculateLapVelocity(float r, vec3 P_velocity, vec3 N_velocity, float N_density)
{
	const float h = mpParticleSize;
	vec3 vel_diff = N_velocity - P_velocity;
	// Implements this equation:
	// W_viscosity(r, h) = 15 / (2 * pi * h^3) * (-r^3 / (2 * h^3) + r^2 / h^2 + h / (2 * r) - 1)
	// LAPLACIAN( W_viscosity(r, h) ) = 45 / (pi * h^6) * (h - r)
	// g_fLapViscosityCoef = fParticleMass * fViscosity * 45.0f / (PI * fSmoothlen^6)
	return  (mpSPHLapViscosityCoef / N_density * (h - r)) * vel_diff;
}

static inline float sphCalculateDensity(float r_sq)
{
	const float h_sq = mpParticleSize * mpParticleSize;
	// Implements this equation:
	// W_poly6(r, h) = 315 / (64 * pi * h^9) * (h^2 - r^2)^3
	// g_fDensityCoef = fParticleMass * 315.0f / (64.0f * PI * fSmoothlen^9)
	return mpSPHDensityCoef * (h_sq - r_sq) * (h_sq - r_sq) * (h_sq - r_sq);
}

static inline vec3 sphComputeAccel(
	vec3 pos1,
	vec3 pos2,
	vec3 vel1,
	vec3 vel2,
	float pressure1,
	float density2 )
{
	uniform const float h_sq = mpParticleSize * mpParticleSize;
	vec3 accel = {0.0f, 0.0f, 0.0f};
	vec3 diff = pos2 - pos1;
	float r_sq = dot3(diff, diff);
	if(r_sq < h_sq && r_sq > 0.0f) {
		float pressure2 = sphCalculatePressure(density2);
		float r = sqrt(r_sq);

		// Pressure Term
		accel += sphCalculateGradPressure(r, pressure1, pressure2, density2, diff);

		// Viscosity Term
		accel += sphCalculateLapVelocity(r, vel1, vel2, density2);
	}
	return accel;
}

export void sphUpdateForce(
	soa<8> Particle all_particles[],
	soa<8> ParticleIMData imds[],
	Cell uniform grid[],
	uniform int xi, uniform int yi, uniform int zi )
{
	uniform const Cell &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
	uniform const int particle_num = gd.end - gd.begin;
	soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
	soa<8> ParticleIMData * uniform imd = &imds[gd.soai*8];

	uniform const int nx_beg = max(xi-1, 0);
	uniform const int nx_end = min(xi+1, mpWorldDiv.x-1);
	uniform const int ny_beg = max(yi-1, 0);
	uniform const int ny_end = min(yi+1, mpWorldDiv.y-1);
	uniform const int nz_beg = max(zi-1, 0);
	uniform const int nz_end = min(zi+1, mpWorldDiv.z-1);

	for(uniform int i=0; i<particle_num; ++i) {
		uniform vec3 pos1 = to_vec3(particles[i]);
		uniform vec3 vel1 = gather3(particles[i],vx,vy,vz);
		uniform float density1 = particles[i].density;
		uniform float pressure1 = sphCalculatePressure(density1);

		vec3 accel = {0.0f, 0.0f, 0.0f};
		for(uniform int nyi=ny_beg; nyi<=ny_end; ++nyi) {
			for(uniform int nzi=nz_beg; nzi<=nz_end; ++nzi) {
				for(uniform int nxi=nx_beg; nxi<=nx_end; ++nxi) {
					uniform const Cell &ngd = grid[mpWorldDiv.x*mpWorldDiv.z*nyi + mpWorldDiv.x*nzi + nxi];
					uniform const int neighbor_num = ngd.end - ngd.begin;
					soa<8> Particle * uniform neighbors = &all_particles[ngd.soai*8];
					foreach(t=0 ... neighbor_num) {
						vec3 pos2 = to_vec3(neighbors[t]);
						vec3 vel2 = gather3(neighbors[t],vx,vy,vz);
						float density2 = neighbors[t].density;
						accel += sphComputeAccel(pos1, pos2, vel1, vel2, pressure1, density2);
					}
				}
			}
		}

		imd[i].ax = reduce_add(accel.x);
		imd[i].ay = reduce_add(accel.y);
		imd[i].az = reduce_add(accel.z);
	}
}


export void sphIntegrate(
	soa<8> Particle all_particles[],
	soa<8> ParticleIMData imds[],
	Cell uniform grid[],
	uniform int xi, uniform int yi, uniform int zi )
{
	uniform const Cell &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
	uniform const int particle_num = gd.end - gd.begin;
	soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
	soa<8> ParticleIMData * uniform imd = &imds[gd.soai*8];

	foreach(i=0 ... particle_num) {
		vec3 vel = gather3(particles[i],vx,vy,vz);
		vec3 accel = gather3(imd[i],ax,ay,az);
		vec3 pos = to_vec3(particles[i]);

		vel += accel * mpTimestep;
		vel *= mpDecelerate;
		vel *= mpCoordScale;

		pos += vel * mpTimestep;
		pos *= mpCoordScale;

		store3(particles[i],x,y,z, pos);
		store3(particles[i],vx,vy,vz, vel);
		particles[i].speed = length3(vel);
	}
}



static inline void impComputeParticleInteraction(
	vec3 pos1,
	vec3 pos2,
	vec3 &accel )
{
	vec3 diff = pos2 - pos1;
	float d = length3(diff);
	if(d > 0.0f) { // d==0: 衝突元＆衝突相手が同一
		vec3 dir = diff * mpRcpParticleSize2; // vec3 dir = diff / d;
		accel += dir * (min(0.0f, d-(mpParticleSize*2.0f)) * mpPressureStiffness);
	}
}

export void impZeroAccel(
	soa<8> Particle all_particles[],
	soa<8> ParticleIMData imds[],
	Cell uniform grid[],
	uniform int xi, uniform int yi, uniform int zi )
{
	uniform const Cell &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
	uniform const int particle_num = gd.end - gd.begin;
	soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
	soa<8> ParticleIMData * uniform imd = &imds[gd.soai*8];
	foreach(i=0 ... particle_num) {
		imd[i].ax = 0.0f;
		imd[i].ay = 0.0f;
		imd[i].az = 0.0f;
	}
}

export void impUpdatePressure(
	soa<8> Particle all_particles[],
	soa<8> ParticleIMData imds[],
	Cell uniform grid[],
	uniform int xi, uniform int yi, uniform int zi )
{
	uniform const Cell &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
	uniform const int particle_num = gd.end - gd.begin;
	soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
	soa<8> ParticleIMData * uniform imd = &imds[gd.soai*8];

	uniform const int nx_beg = max(xi-1, 0);
	uniform const int nx_end = min(xi+1, mpWorldDiv.x-1);
	uniform const int ny_beg = max(yi-1, 0);
	uniform const int ny_end = min(yi+1, mpWorldDiv.y-1);
	uniform const int nz_beg = max(zi-1, 0);
	uniform const int nz_end = min(zi+1, mpWorldDiv.z-1);

	for(uniform int i=0; i<particle_num; ++i) {
		uniform vec3 pos1 = to_vec3(particles[i]);
		vec3 accel = {0.0f, 0.0f, 0.0f};
		for(uniform int nyi=ny_beg; nyi<=ny_end; ++nyi) {
			for(uniform int nzi=nz_beg; nzi<=nz_end; ++nzi) {
				for(uniform int nxi=nx_beg; nxi<=nx_end; ++nxi) {
					uniform const Cell &ngd = grid[mpWorldDiv.x*mpWorldDiv.z*nyi + mpWorldDiv.x*nzi + nxi];
					soa<8> Particle * uniform neighbors = &all_particles[ngd.soai*8];
					uniform const int neighbor_num = ngd.end - ngd.begin;
					foreach(t=0 ... neighbor_num) {
						vec3 pos2 = to_vec3(neighbors[t]);
						impComputeParticleInteraction(pos1, pos2, accel);
					}
				}
			}
		}

		imd[i].ax = reduce_add(accel.x);
		imd[i].ay = reduce_add(accel.y);
		imd[i].az = reduce_add(accel.z);
	}
}

export void impIntegrate(
	soa<8> Particle all_particles[],
	soa<8> ParticleIMData imds[],
	Cell uniform grid[],
	uniform int xi, uniform int yi, uniform int zi )
{
	uniform const Cell &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
	uniform const int particle_num = gd.end - gd.begin;
	soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
	soa<8> ParticleIMData * uniform imd = &imds[gd.soai*8];

	foreach(i=0 ... particle_num) {
		vec3 pos = to_vec3(particles[i]);
		vec3 accel = gather3(imd[i],ax,ay,az);
		vec3 vel = gather3(particles[i],vx,vy,vz);

		vel += accel * mpTimestep;
		// 自然減速
		vel *= mpDecelerate;
		vel *= mpCoordScale;

		pos += vel * mpTimestep;
		pos *= mpCoordScale;

		store3(particles[i],x,y,z, pos);
		store3(particles[i],vx,vy,vz, vel);
		particles[i].speed = length3(vel);
	}
}
