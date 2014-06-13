// コンパイルは↓みたいなカスタムビルドステップで。
// ispc %(FullPath) -o $(TargetDir)%(Filename).obj -h $(TargetDir)%(Filename)_ispc.h --target=sse2,sse4,avx --arch=x86-64 --opt=fast-masked-vload --opt=fast-math
// 
// Intel SPMD はこちら
// http://ispc.github.com/


#include "ispc_vectormath.h"
#include "mpConst.h"
#include "mpCollision.h"
typedef unsigned int32 uint32;

static uniform vec3  mpWorldCenter;
static uniform vec3  mpWorldSize;
static uniform ivec3 mpWorldDiv;
static uniform vec3  mpCoordScale;

static uniform float mpTimestep;
static uniform float mpDecelerate;
static uniform float mpPressureStiffness;
static uniform float mpWallStiffness;

static uniform float mpSPHRestDensity;
static uniform float mpSPHParticleMass;
static uniform float mpSPHViscosity;
static uniform float mpSPHDensityCoef;
static uniform float mpSPHGradPressureCoef;
static uniform float mpSPHLapViscosityCoef;

static soa<8> ParticleIMData g_imds[mpMaxParticleNum];


export void sphUpdateConstants(uniform KernelParams &params)
{
    mpWorldCenter.x     = params.WorldCenter_x;
    mpWorldCenter.y     = params.WorldCenter_y;
    mpWorldCenter.z     = params.WorldCenter_z;
    mpWorldSize.x       = params.WorldSize_x;
    mpWorldSize.y       = params.WorldSize_y;
    mpWorldSize.z       = params.WorldSize_z;
    mpWorldDiv.x        = params.WorldDiv_x;
    mpWorldDiv.y        = params.WorldDiv_y;
    mpWorldDiv.z        = params.WorldDiv_z;
    mpCoordScale.x      = params.Scale_x;
    mpCoordScale.y      = params.Scale_y;
    mpCoordScale.z      = params.Scale_z;

    mpTimestep          = params.Timestep;
    mpDecelerate        = params.Decelerate;
    mpPressureStiffness = params.PressureStiffness;
    mpWallStiffness     = params.WallStiffness;

    mpSPHRestDensity    = params.SPHRestDensity;
    mpSPHParticleMass   = params.SPHParticleMass;
    mpSPHViscosity      = params.SPHViscosity;

    mpSPHDensityCoef        = mpSPHParticleMass * 315.0f / (64.0f * PI * pow(mpParticleSize, 9));
    mpSPHGradPressureCoef   = mpSPHParticleMass * -45.0f / (PI * pow(mpParticleSize, 6));
    mpSPHLapViscosityCoef   = mpSPHParticleMass * mpSPHViscosity * 45.0f / (PI * pow(mpParticleSize, 6));
}

export void sphGetConstants(uniform KernelParams &params)
{
    params.WorldCenter_x    = mpWorldCenter.x;
    params.WorldCenter_y    = mpWorldCenter.y;
    params.WorldCenter_z    = mpWorldCenter.z;
    params.WorldSize_x      = mpWorldSize.x;
    params.WorldSize_y      = mpWorldSize.y;
    params.WorldSize_z      = mpWorldSize.z;
    params.WorldDiv_x       = mpWorldDiv.x;
    params.WorldDiv_y       = mpWorldDiv.y;
    params.WorldDiv_z       = mpWorldDiv.z;
    params.Scale_x          = mpCoordScale.x;
    params.Scale_y          = mpCoordScale.y;
    params.Scale_z          = mpCoordScale.z;

    params.Timestep         = mpTimestep;
    params.Decelerate       = mpDecelerate;
    params.PressureStiffness= mpPressureStiffness;
    params.WallStiffness    = mpWallStiffness;

    params.SPHRestDensity   = mpSPHRestDensity;
    params.SPHParticleMass  = mpSPHParticleMass;
    params.SPHViscosity     = mpSPHViscosity;
}


vec3 ComputeGridBox(uniform int32 xi, uniform int32 yi, uniform int32 zi, uniform vec3 &o_bl, uniform vec3 &o_ur)
{
    uniform vec3 cell_size = mpWorldSize*2.0f / mpWorldDiv;
    o_bl.x = mpWorldCenter.x - mpWorldSize.x + cell_size.x*xi;
    o_bl.y = mpWorldCenter.y - mpWorldSize.y + cell_size.y*yi;
    o_bl.z = mpWorldCenter.z - mpWorldSize.z + cell_size.z*zi;
    o_ur.x = mpWorldCenter.x - mpWorldSize.x + cell_size.x*(xi+1);
    o_ur.y = mpWorldCenter.y - mpWorldSize.y + cell_size.y*(yi+1);
    o_ur.z = mpWorldCenter.z - mpWorldSize.z + cell_size.z*(zi+1);
}

#define repulse(p, f, n, d, shape)\
    {\
        p.hit = shape.id;\
        vec3 accel = get_accel(f);\
        accel += n * (-d * mpWallStiffness);\
        set_accel(f, accel);\
    }


bool IsGridOverrapedAABB(uniform int32 xi, uniform int32 yi, uniform int32 zi, uniform const BoundingBox &bb)
{
    uniform vec3 grid_bl;
    uniform vec3 grid_ur;
    ComputeGridBox(xi,yi,zi, grid_bl, grid_ur);
    uniform vec3 bb_bl = get_bl(bb);
    uniform vec3 bb_ur = get_ur(bb);
    if( grid_ur.x < bb_bl.x || grid_ur.z < bb_bl.z ||
        grid_bl.x > bb_ur.x || grid_bl.z > bb_ur.z )
    {
        return false;
    }
    return true;
}

export void sphProcessCollision(
    soa<8> Particle all_particles[],
    GridData uniform grid[],
    uniform int32 xi, uniform int32 yi, uniform int32 zi,
    uniform SphereCollider spheres[], uniform int32 num_spheres,
    uniform PlaneCollider planes[], uniform int32 num_planes,
    uniform BoxCollider boxes[], uniform int32 num_boxes )
{
    uniform const GridData &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
    uniform const int32 particle_num = gd.end - gd.begin;
    soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
    soa<8> ParticleIMData * uniform imd = &g_imds[gd.soai*8];

    uniform float particle_radius = mpParticleSize;

    // 適切に関数に分けたかったが、struct の参照渡しができないとか謎のコンパイラクラッシュとかで断念。
    // 全部 inline で書きます。

    // Plane
    for(uniform int32 s=0; s<num_planes; ++s) {
        uniform const PlaneCollider &shape = planes[s];
        uniform const vec3 plane_normal = get_normal(shape);
        uniform const float plane_distance = shape.distance;
        foreach(i=0 ... particle_num) {
            vec3 particles_pos = get_pos(particles[i]);
            float distance = dot3(particles_pos, plane_normal) + plane_distance;
            if(distance < 0.0f) {
                repulse(particles[i], imd[i], plane_normal, distance, shape);
            }
        }
    }

    // Sphere
    for(uniform int32 s=0; s<num_spheres; ++s) {
        uniform const SphereCollider &shape = spheres[s];
        if(!IsGridOverrapedAABB(xi,0,zi, shape.bb)) { continue; }

        uniform const vec3 sphere_pos = get_pos(shape);
        uniform const float sphere_radius = shape.radius;
        foreach(i=0 ... particle_num) {
            vec3 particles_pos = get_pos(particles[i]);
            vec3 diff = particles_pos-sphere_pos;
            float len = length3(diff);
            float distance = len - sphere_radius;
            if(distance <= 0.0f) {
                vec3 dir = diff / len;
                repulse(particles[i], imd[i], dir, distance, shape);
            }
        }
    }

    // Box
    for(uniform int32 s=0; s<num_boxes; ++s) {
        uniform const BoxCollider &shape = boxes[s];
        if(!IsGridOverrapedAABB(xi,0,zi, shape.bb)) { continue; }

        uniform vec3 box_pos = get_pos(shape);
        foreach(i=0 ... particle_num) {
            vec3 particles_pos = get_pos(particles[i]);

            int32 inside = 0;
            float closest_distance = -9999.0f;
            vec3 closest_normal;
            vec3 ppos = particles_pos - box_pos;
            for(uniform int32 p=0; p<6; ++p) {
                uniform const vec3 plane_normal = get_normal(shape.planes[p]);
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
                repulse(particles[i], imd[i], closest_normal, closest_distance, shape);
            }
        }
    }
}
#undef repulse


export void sphProcessExternalForce(
    soa<8> Particle all_particles[],
    GridData uniform grid[],
    uniform int32 xi, uniform int32 yi, uniform int32 zi,
    uniform Force forces[], uniform int32 num_forces )
{
    uniform const GridData &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
    uniform const int32 particle_num = gd.end - gd.begin;
    soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
    soa<8> ParticleIMData * uniform imd = &g_imds[gd.soai*8];

    uniform float particle_radius = mpParticleSize;


    for(uniform int32 fi=0; fi<num_forces; ++fi) {
        uniform const Force &force = forces[fi];

        // todo: 球状 radial force はよく使われると予想されるので特殊化による最適化
        if(force.shape_type==FS_AffectAll) {
            foreach(i=0 ... particle_num) {
                imd[i].affection = 1.0f;
            }
        }
        else if(force.shape_type==FS_Sphere) {
            if(!IsGridOverrapedAABB(xi,0,zi, force.bb)) { continue; }

            uniform const Sphere &sphere = force.shape_sphere;
            float radius_sq = sphere.radius * sphere.radius;
            vec3 force_center = get_pos(sphere);
            foreach(i=0 ... particle_num) {
                vec3 particles_pos = get_pos(particles[i]);
                vec3 diff = particles_pos - force_center;
                float dist_sq = length_sq3(diff);
                imd[i].affection = dist_sq<=radius_sq ? 1.0f : 0.0f;
            }
        }
        else if(force.shape_type==FS_Box) {
            if(!IsGridOverrapedAABB(xi,0,zi, force.bb)) { continue; }

            uniform const Box &box = force.shape_box;
            uniform vec3 box_pos = get_pos(box);
            foreach(i=0 ... particle_num) {
                vec3 particles_pos = get_pos(particles[i]);
                int32 inside = 0;
                vec3 ppos = particles_pos - box_pos;
                for(uniform int32 p=0; p<6; ++p) {
                    uniform const vec3 plane_normal = get_normal(box.planes[p]);
                    uniform const float plane_distance = box.planes[p].distance;
                    float distance = dot3(ppos, plane_normal) + plane_distance;
                    if(distance < 0.0f) {
                        inside++;
                    }
                }
                imd[i].affection = inside==6 ? 1.0f : 0.0f;
            }
        }

        if(force.dir_type==FD_Directional) {
            foreach(i=0 ... particle_num) {
                vec3 dir = get_pos(force.params);
                vec3 accel = get_accel(imd[i]);
                accel += dir * imd[i].affection * force.params.strength;
                set_accel(imd[i], accel);
            }
        }
        else if(force.dir_type==FD_Radial) {
            vec3 force_center = get_pos(force.params);
            foreach(i=0 ... particle_num) {
                vec3 particles_pos = get_pos(particles[i]);
                vec3 diff = particles_pos - force_center;
                vec3 dir = normalize3(diff);
                vec3 accel = get_accel(imd[i]);
                accel += dir * imd[i].affection * force.params.strength;
                set_accel(imd[i], accel);
            }
        }
    }
}




static inline float sphComputeDensity(
    vec3 pos1,
    vec3 pos2 )
{
    static uniform const float h_sq = mpParticleSize * mpParticleSize;
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
    GridData uniform grid[],
    uniform int32 xi, uniform int32 yi, uniform int32 zi )
{
    uniform GridData &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
    uniform const int32 particle_num = gd.end - gd.begin;
    soa<8> Particle * uniform particles = &all_particles[gd.soai*8];

    uniform const int32 nx_beg = max(xi-1, 0);
    uniform const int32 nx_end = min(xi+1, mpWorldDiv.x-1);
    uniform const int32 ny_beg = max(xi-1, 0);
    uniform const int32 ny_end = min(xi+1, mpWorldDiv.y-1);
    uniform const int32 nz_beg = max(zi-1, 0);
    uniform const int32 nz_end = min(zi+1, mpWorldDiv.z-1);

    for(uniform int32 i=0; i<particle_num; ++i) {
        uniform vec3 pos1 = get_pos(particles[i]);
        float density = 0.0f;
        for(uniform int32 nzi=nz_beg; nzi<=nz_end; ++nzi) {
            for(uniform int32 nxi=nx_beg; nxi<=nx_end; ++nxi) {
                uniform const GridData &ngd = grid[mpWorldDiv.x*nzi* + nxi];
                uniform const int32 neighbor_num = ngd.end - ngd.begin;
                soa<8> Particle * uniform neighbors = &all_particles[ngd.soai*8];
                foreach(t=0 ... neighbor_num) {
                    vec3 pos2 = get_pos(neighbors[t]);
                    density += sphComputeDensity(pos1, pos2);
                }
            }
        }
        particles[i].density = reduce_add(density);
    }
}


export void sphUpdateDensityEst1(
    soa<8> Particle all_particles[],
    GridData uniform grid[],
    uniform int32 xi, uniform int32 yi, uniform int32 zi )
{
    uniform GridData &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
    uniform const int32 particle_num = gd.end - gd.begin;
    soa<8> Particle * uniform particles = &all_particles[gd.soai*8];

    for(uniform int32 i=0; i<particle_num; ++i) {
        uniform vec3 pos1 = get_pos(particles[i]);
        float density = 0.0f;

        uniform const GridData &ngd = gd;
        uniform const int32 neighbor_num = ngd.end - ngd.begin;
        soa<8> Particle * uniform neighbors = &all_particles[ngd.soai*8];
        foreach(t=0 ... neighbor_num) {
            vec3 pos2 = get_pos(neighbors[t]);
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
    GridData uniform grid[],
    uniform int32 xi, uniform int32 yi, uniform int32 zi )
{
    uniform GridData &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
    uniform const int32 particle_num = gd.end - gd.begin;
    soa<8> Particle * uniform particles = &all_particles[gd.soai*8];

    uniform const int32 nx_beg = max(xi-1, 0);
    uniform const int32 nx_end = min(xi+1, mpWorldDiv.x-1);
    uniform const int32 ny_beg = max(xi-1, 0);
    uniform const int32 ny_end = min(xi+1, mpWorldDiv.y-1);
    uniform const int32 nz_beg = max(zi-1, 0);
    uniform const int32 nz_end = min(zi+1, mpWorldDiv.z-1);

    for(uniform int32 nzi=nz_beg; nzi<=nz_end; ++nzi) {
        for(uniform int32 nxi=nx_beg; nxi<=nx_end; ++nxi) {
            uniform const GridData &ngd = grid[mpWorldDiv.x*nzi + nxi];
            foreach(i=0 ... particle_num) {
                particles[i].density += ngd.density*0.05f;
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
    static uniform const float h_sq = mpParticleSize * mpParticleSize;
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
    GridData uniform grid[],
    uniform int32 xi, uniform int32 yi, uniform int32 zi )
{
    uniform const GridData &gd = grid[mpWorldDiv.x*zi + xi];
    uniform const int32 particle_num = gd.end - gd.begin;
    soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
    soa<8> ParticleIMData * uniform imd = &g_imds[gd.soai*8];

    uniform const int32 nx_beg = max(xi-1, 0);
    uniform const int32 nx_end = min(xi+1, mpWorldDiv.x-1);
    uniform const int32 ny_beg = max(xi-1, 0);
    uniform const int32 ny_end = min(xi+1, mpWorldDiv.y-1);
    uniform const int32 nz_beg = max(zi-1, 0);
    uniform const int32 nz_end = min(zi+1, mpWorldDiv.z-1);

    for(uniform int32 i=0; i<particle_num; ++i) {
        uniform vec3 pos1 = get_pos(particles[i]);
        uniform vec3 vel1 = get_vel(particles[i]);
        uniform float density1 = particles[i].density;
        uniform float pressure1 = sphCalculatePressure(density1);

        vec3 accel = {0.0f, 0.0f, 0.0f};
        for(uniform int32 nzi=nz_beg; nzi<=nz_end; ++nzi) {
            for(uniform int32 nxi=nx_beg; nxi<=nx_end; ++nxi) {
                uniform const GridData &ngd = grid[mpWorldDiv.x*nzi + nxi];
                uniform const int32 neighbor_num = ngd.end - ngd.begin;
                soa<8> Particle * uniform neighbors = &all_particles[ngd.soai*8];
                foreach(t=0 ... neighbor_num) {
                    vec3 pos2 = get_pos(neighbors[t]);
                    vec3 vel2 = get_vel(neighbors[t]);
                    float density2 = neighbors[t].density;
                    accel += sphComputeAccel(pos1, pos2, vel1, vel2, pressure1, density2);
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
    GridData uniform grid[],
    uniform int32 xi, uniform int32 yi, uniform int32 zi )
{
    uniform const GridData &gd = grid[mpWorldDiv.x*zi + xi];
    uniform const int32 particle_num = gd.end - gd.begin;
    soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
    soa<8> ParticleIMData * uniform imd = &g_imds[gd.soai*8];

    foreach(i=0 ... particle_num) {
        vec3 vel = get_vel(particles[i]);
        vec3 accel = get_accel(imd[i]);
        vec3 pos = get_pos(particles[i]);

        vel += accel * mpTimestep;
        vel *= mpDecelerate;
        vel *= mpCoordScale;

        pos += vel * mpTimestep;
        pos *= mpCoordScale;

        set_pos(particles[i], pos);
        set_vel(particles[i], vel);
        particles[i].speed = length3(vel);
    }
}



static inline void impComputeParticleInteraction(
    vec3 pos1,
    vec3 pos2,
    vec3 &accel )
{
    const uniform float mpRcpParticleSize2 = 1.0f/(mpParticleSize*2.0f);
    vec3 diff = pos2 - pos1;    
    float d = length3(diff); // float d = length3est(diff); // 何故か est の方が遅くなる
    if(d > 0.0f) { // d==0: 衝突元＆衝突相手が同一
        vec3 dir = diff * mpRcpParticleSize2; // vec3 dir = diff / d;
        accel += dir * (min(0.0f, d-(mpParticleSize*2.0f)) * mpPressureStiffness);
    }
}

export void impZeroAccel(
    soa<8> Particle all_particles[],
    GridData uniform grid[],
    uniform int32 xi, uniform int32 yi, uniform int32 zi )
{
    uniform const GridData &gd = grid[mpWorldDiv.x*zi + xi];
    uniform const int32 particle_num = gd.end - gd.begin;
    soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
    soa<8> ParticleIMData * uniform imd = &g_imds[gd.soai*8];
    foreach(i=0 ... particle_num) {
        imd[i].ax = 0.0f;
        imd[i].ay = 0.0f;
        imd[i].az = 0.0f;
    }
}

export void impUpdateVelocity(
    soa<8> Particle all_particles[],
    GridData uniform grid[],
    uniform int32 xi, uniform int32 yi, uniform int32 zi )
{
    uniform const GridData &gd = grid[mpWorldDiv.x*zi + xi];
    uniform const int32 particle_num = gd.end - gd.begin;
    soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
    soa<8> ParticleIMData * uniform imd = &g_imds[gd.soai*8];

    uniform const int32 nx_beg = max(xi-1, 0);
    uniform const int32 nx_end = min(xi+1, mpWorldDiv.x-1);
    uniform const int32 ny_beg = max(xi-1, 0);
    uniform const int32 ny_end = min(xi+1, mpWorldDiv.y-1);
    uniform const int32 nz_beg = max(zi-1, 0);
    uniform const int32 nz_end = min(zi+1, mpWorldDiv.z-1);

    for(uniform int32 i=0; i<particle_num; ++i) {
        uniform vec3 pos1 = get_pos(particles[i]);
        vec3 accel = {0.0f, 0.0f, 0.0f};
        for(uniform int32 nzi=nz_beg; nzi<=nz_end; ++nzi) {
            for(uniform int32 nxi=nx_beg; nxi<=nx_end; ++nxi) {
                uniform const GridData &ngd = grid[mpWorldDiv.x*nzi + nxi];
                soa<8> Particle * uniform neighbors = &all_particles[ngd.soai*8];
                uniform const int32 neighbor_num = ngd.end - ngd.begin;
                foreach(t=0 ... neighbor_num) {
                    vec3 pos2 = get_pos(neighbors[t]);
                    impComputeParticleInteraction(pos1, pos2, accel);
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
    GridData uniform grid[],
    uniform int32 xi, uniform int32 yi, uniform int32 zi )
{
    uniform const GridData &gd = grid[mpWorldDiv.x*mpWorldDiv.z*yi + mpWorldDiv.x*zi + xi];
    uniform const int32 particle_num = gd.end - gd.begin;
    soa<8> Particle * uniform particles = &all_particles[gd.soai*8];
    soa<8> ParticleIMData * uniform imd = &g_imds[gd.soai*8];

    foreach(i=0 ... particle_num) {
        vec3 pos = get_pos(particles[i]);
        vec3 accel = get_accel(imd[i]);
        vec3 vel = get_vel(particles[i]);

        vel += accel * mpTimestep;
        // 自然減速
        vel *= mpDecelerate;
        vel *= mpCoordScale;

        pos += vel * mpTimestep;
        pos *= mpCoordScale;

        set_pos(particles[i], pos);
        set_vel(particles[i], vel);
        particles[i].speed = length3(vel);
    }
}
