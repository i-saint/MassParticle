#include "ispc_vectormath.h"
#include "mpCollision.h"

struct Context
{
   KernelParams *kparams;
   Cell         *grid;

   float *pos_x;
   float *pos_y;
   float *pos_z;
   float *vel_x;
   float *vel_y;
   float *vel_z;
   float *acl_x;
   float *acl_y;
   float *acl_z;
   float *speed;
   float *density;
   float *affection;
   int   *hit;

   PlaneCollider    *planes;
   SphereCollider   *spheres;
   CapsuleCollider  *capsules;
   BoxCollider      *boxes;
   Force            *forces;
   int              num_planes;
   int              num_spheres;
   int              num_capsules;
   int              num_boxes;
   int              num_forces;
};

#define expand_particle_params()\
    uniform float *uniform pos_x = &ctx.pos_x[gd.soai*8];\
    uniform float *uniform pos_y = &ctx.pos_y[gd.soai*8];\
    uniform float *uniform pos_z = &ctx.pos_z[gd.soai*8];\
    uniform float *uniform vel_x = &ctx.vel_x[gd.soai*8];\
    uniform float *uniform vel_y = &ctx.vel_y[gd.soai*8];\
    uniform float *uniform vel_z = &ctx.vel_z[gd.soai*8];\
    uniform float *uniform acl_x = &ctx.acl_x[gd.soai*8];\
    uniform float *uniform acl_y = &ctx.acl_y[gd.soai*8];\
    uniform float *uniform acl_z = &ctx.acl_z[gd.soai*8];\
    uniform float *uniform speed = &ctx.speed[gd.soai*8];\
    uniform float *uniform density = &ctx.density[gd.soai*8];\
    uniform float *uniform affection = &ctx.affection[gd.soai*8];\
    uniform int   *uniform hit = &ctx.hit[gd.soai*8];

#define get_particle_position(i) {pos_x[i], pos_y[i], pos_z[i]}
#define get_particle_velocity(i) {vel_x[i], vel_y[i], vel_z[i]}
#define get_particle_accel(i) {acl_x[i], acl_y[i], acl_z[i]}
#define set_particle_position(i, v) pos_x[i]=v.x; pos_y[i]=v.y; pos_z[i]=v.z;
#define set_particle_velocity(i, v) vel_x[i]=v.x; vel_y[i]=v.y; vel_z[i]=v.z;
#define set_particle_accel(i, v) acl_x[i]=v.x; acl_y[i]=v.y; acl_z[i]=v.z;


#define expand_neighbor_params()\
    uniform float *uniform npos_x = &ctx.pos_x[ngd.soai*8];\
    uniform float *uniform npos_y = &ctx.pos_y[ngd.soai*8];\
    uniform float *uniform npos_z = &ctx.pos_z[ngd.soai*8];\
    uniform float *uniform nvel_x = &ctx.vel_x[ngd.soai*8];\
    uniform float *uniform nvel_y = &ctx.vel_y[ngd.soai*8];\
    uniform float *uniform nvel_z = &ctx.vel_z[ngd.soai*8];\
    uniform float *uniform ndensity = &ctx.density[ngd.soai*8];

#define get_neighbor_position(i) {npos_x[i], npos_y[i], npos_z[i]}
#define get_neighbor_velocity(i) {nvel_x[i], nvel_y[i], nvel_z[i]}


export uniform int GetProgramCount() { return programCount; }





bool IsOverlaped(vec3f pos, uniform const AABB &aabb, float r)
{
    vec3f wext = (aabb.ur-aabb.bl) * 0.5 + r;
    vec3f rpos = pos - (aabb.ur+aabb.bl) * 0.5;
    rpos = abs(rpos);
    if(rpos.x>wext.x || rpos.y>wext.y || rpos.z>wext.z)
    {
        return false;
    }
    return true;
}


struct DistanceData
{
    vec3f direction;
    float distance;
};


DistanceData DistancePointSphere(vec3f ppos, uniform const Sphere &shape)
{
    vec3f diff = ppos - shape.center;
    float distance = length(diff)-shape.radius;
    vec3f dir = normalize(diff);

    DistanceData ret = {dir, distance};
    return ret;
}


DistanceData DistancePointCapsule(vec3f ppos, uniform const Capsule &shape)
{
    vec3f pos1 = shape.pos1;
    vec3f pos2 = shape.pos2;
    vec3f d = pos2-pos1;

    float t = clamp(dot(ppos-pos1, pos2-pos1) / dot(d,d), 0.0f, 1.0f);
    vec3f nearest = pos1 + (pos2-pos1)*t;
    vec3f diff = ppos-nearest;
    float distance = length(diff)-shape.radius;
    vec3f dir = normalize(diff);

    DistanceData ret = {dir, distance};
    return ret;
}

DistanceData DistancePointBox(vec3f ppos, uniform const Box &shape)
{
    int inside = 0;
    float closest_distance = -999.0f;
    vec3f closest_normal;
    vec3f rpos = ppos - shape.center;
    for(int p=0; p<6; ++p) {
        vec3f plane_normal = shape.planes[p].normal;
        float plane_distance = shape.planes[p].distance;
        float distance = dot(rpos, plane_normal) + plane_distance;
        if(distance > closest_distance) {
            closest_distance = distance;
            closest_normal = plane_normal;
        }
    }

    DistanceData ret = {closest_normal, closest_distance};
    return ret;
}



vec3f ComputeGridBox(uniform const KernelParams &params, uniform const vec3i idx, uniform vec3f &o_bl, uniform vec3f &o_ur)
{
    uniform vec3f wextent = params.world_extent;
    uniform vec3i wdiv = params.world_div;
    uniform vec3f cell_size = wextent*2.0f / wdiv;
    o_bl.x = params.world_center.x - params.world_extent.x + cell_size.x*idx.x;
    o_bl.y = params.world_center.y - params.world_extent.y + cell_size.y*idx.y;
    o_bl.z = params.world_center.z - params.world_extent.z + cell_size.z*idx.z;
    o_ur.x = params.world_center.x - params.world_extent.x + cell_size.x*(idx.x+1);
    o_ur.y = params.world_center.y - params.world_extent.y + cell_size.y*(idx.y+1);
    o_ur.z = params.world_center.z - params.world_extent.z + cell_size.z*(idx.z+1);
}

#define repulse(n, d, props)\
    {\
        hit[i] = props.owner_id;\
        vec3f a = get_particle_accel(i);\
        a = a + (n * (-d * props.stiffness));\
        set_particle_accel(i,a);\
    }\


bool IsGridOverrapedAABB(uniform const KernelParams &params, uniform const vec3i idx, uniform const BoundingBox &bb)
{
    uniform vec3f grid_bl;
    uniform vec3f grid_ur;
    ComputeGridBox(params, idx, grid_bl, grid_ur);
    uniform vec3f bb_bl = bb.bl;
    uniform vec3f bb_ur = bb.ur;
    if( grid_ur.x < bb_bl.x || grid_bl.x > bb_ur.x ||
        grid_ur.y < bb_bl.y || grid_bl.y > bb_ur.y ||
        grid_ur.z < bb_bl.z || grid_bl.z > bb_ur.z )
    {
        return false;
    }
    return true;
}


export void ProcessColliders(uniform Context &ctx, uniform const vec3i &idx)
{
    uniform const KernelParams kp = *ctx.kparams;
    uniform const Cell &gd = ctx.grid[kp.world_div.x*kp.world_div.z*idx.y + kp.world_div.x*idx.z + idx.x];
    uniform const int particle_num = gd.end - gd.begin;
    expand_particle_params();

    uniform float particle_radius = kp.particle_size;

    // Plane
    uniform const int num_planes = ctx.num_planes;
    uniform PlaneCollider *uniform planes = ctx.planes;
    for(uniform int s=0; s<num_planes; ++s) {
        uniform const PlaneCollider &col = planes[s];
        uniform const Plane &shape = col.shape;
        if(!IsGridOverrapedAABB(kp, idx, col.bounds)) { continue; }

        uniform const vec3f plane_normal = shape.normal;
        uniform const float plane_distance = shape.distance;
        foreach(i=0 ... particle_num) {
            vec3f ppos = get_particle_position(i);
            float distance = dot(ppos, plane_normal) + plane_distance;
            if(distance < 0.0f) {
                repulse(plane_normal, distance, col.props);
            }
        }
    }

    // Sphere
    uniform const int num_spheres = ctx.num_spheres;
    uniform SphereCollider *uniform spheres = ctx.spheres;
    for(uniform int s=0; s<num_spheres; ++s) {
        uniform const SphereCollider &col = spheres[s];
        uniform const Sphere &shape = col.shape;
        if(!IsGridOverrapedAABB(kp, idx, col.bounds)) { continue; }

        uniform const vec3f sphere_pos = shape.center;
        uniform const float sphere_radius = shape.radius;
        foreach(i=0 ... particle_num) {
            vec3f ppos = get_particle_position(i);
            vec3f diff = ppos - sphere_pos;
            float len = length(diff);
            float distance = len - sphere_radius;
            if(distance < 0.0f) {
                vec3f dir = diff / len;
                repulse(dir, distance, col.props);
            }
        }
    }
    
    // Capsules
    uniform const int num_capsules = ctx.num_capsules;
    uniform CapsuleCollider *uniform capsules = ctx.capsules;
    for(uniform int s=0; s<num_capsules; ++s) {
        uniform const CapsuleCollider &col = capsules[s];
        uniform const Capsule &shape = col.shape;
        if(!IsGridOverrapedAABB(kp, idx, col.bounds)) { continue; }

        uniform const vec3f pos1 = shape.pos1;
        uniform const vec3f pos2 = shape.pos2;
        uniform const float radius = shape.radius;
        uniform float rcp_lensq = shape.rcp_lensq;
        foreach(i=0 ... particle_num) {
            vec3f ppos = get_particle_position(i);
            const float t = dot(ppos-pos1, pos2-pos1) * rcp_lensq;
            vec3f diff;
            if(t<=0.0f) {
                diff = ppos-pos1;
            }
            else if(t>=1.0f) {
                diff = ppos-pos2;
            }
            else {
                vec3f nearest = pos1 + (pos2-pos1)*t;
                diff = ppos-nearest;
            }
            float len = length(diff);
            float distance = len - radius;
            if(distance < 0.0f) {
                vec3f dir = diff / len;
                repulse(dir, distance, col.props);
            }
        }
    }

    // Box
    uniform const int num_boxes = ctx.num_boxes;
    uniform BoxCollider *uniform boxes = ctx.boxes;
    for(uniform int s=0; s<num_boxes; ++s) {
        uniform const BoxCollider &col = boxes[s];
        uniform const Box &shape = col.shape;
        if(!IsGridOverrapedAABB(kp, idx, col.bounds)) { continue; }

        uniform vec3f box_pos = shape.center;
        foreach(i=0 ... particle_num) {
            int inside = 0;
            float closest_distance = -9999.0f;
            vec3f closest_normal;
            vec3f ppos = get_particle_position(i);
            ppos = ppos - box_pos;
            for(uniform int p=0; p<6; ++p) {
                uniform const vec3f plane_normal = shape.planes[p].normal;
                uniform const float plane_distance = shape.planes[p].distance;
                float distance = dot(ppos, plane_normal) + plane_distance;
                if(distance < 0.0f) {
                    inside++;
                    if(distance > closest_distance) {
                        closest_distance = distance;
                        closest_normal = plane_normal;
                    }
                }
            }
            if(inside==6) {
                repulse(closest_normal, closest_distance, col.props);
            }
        }
    }
}
#undef repulse



vec3f VectorField(vec3f pos, vec3f rcp_cellsize, float strength, float random_seed, float random_diffuse)
{
    vec3f dir = normalize(iq_rand(floor(pos * rcp_cellsize) * (1.0+random_seed)) * 2.0 - 1.0);
    float rs = iq_rand(dot(pos, pos));
    vec3f accel = dir * (strength + random_diffuse*rs);
    return accel;

}

export void ProcessExternalForce(uniform Context &ctx, uniform const vec3i &idx)
{
    uniform const KernelParams kp = *ctx.kparams;
    uniform const Cell &gd = ctx.grid[kp.world_div.x*kp.world_div.z*idx.y + kp.world_div.x*idx.z + idx.x];
    uniform const int particle_num = gd.end - gd.begin;
    expand_particle_params();

    uniform float particle_radius = kp.particle_size;

    uniform const int num_forces = ctx.num_forces;
    Force *uniform forces = ctx.forces;
    for(uniform int fi=0; fi<num_forces; ++fi) {
        uniform const Force &force = forces[fi];
        uniform const ForceProperties &props = force.props;

        if(props.shape_type==FS_AffectAll) {
            foreach(i=0 ... particle_num) {
                affection[i] = 1.0f;
            }
        }
        else if(props.shape_type==FS_Sphere) {
            if(!IsGridOverrapedAABB(kp, idx, force.bounds)) { continue; }

            uniform const Sphere &sphere = force.sphere;
            float radius_sq = sphere.radius * sphere.radius;
            vec3f center = sphere.center;
            foreach(i=0 ... particle_num) {
                vec3f ppos = get_particle_position(i);
                vec3f diff = ppos - center;
                float distance_sq = length_sq(diff);
                affection[i] = distance_sq<=radius_sq ? 1.0f : 0.0f;
            }
        }
        else if(props.shape_type==FS_Capsule) {
            if(!IsGridOverrapedAABB(kp, idx, force.bounds)) { continue; }

            // todo
        }
        else if(props.shape_type==FS_Box) {
            if(!IsGridOverrapedAABB(kp, idx, force.bounds)) { continue; }

            uniform const Box &box = force.box;
            uniform vec3f box_pos = box.center;
            foreach(i=0 ... particle_num) {
                int inside = 0;
                vec3f ppos = get_particle_position(i);
                ppos = ppos - box_pos;
                for(uniform int p=0; p<6; ++p) {
                    uniform const vec3f plane_normal = box.planes[p].normal;
                    uniform const float plane_distance = box.planes[p].distance;
                    float distance = dot(ppos, plane_normal) + plane_distance;
                    if(distance < 0.0f) {
                        inside++;
                    }
                }
                affection[i] = inside==6 ? 1.0f : 0.0f;
            }
        }

        if(props.dir_type==FD_Directional) {
            foreach(i=0 ... particle_num) {
                vec3f dir = props.direction;
                float af = affection[i];
                vec3f a = get_particle_accel(i);
                a = a + dir * lerp(props.strength_far, props.strength_near, pow(af, props.attenuation_exp));
                set_particle_accel(i,a);
            }
        }
        else if(props.dir_type==FD_Radial) {
            vec3f center = props.center;
            foreach(i=0 ... particle_num) {
                vec3f ppos = get_particle_position(i);
                vec3f diff = ppos - center;
                vec3f dir = normalize(diff);
                float af = affection[i];
                vec3f a = get_particle_accel(i);
                a = a + dir * lerp(props.strength_far, props.strength_near, pow(af, props.attenuation_exp));
                set_particle_accel(i,a);
            }
        }
        else if(props.dir_type==FD_RadialCapsule) {
            // todo
        }
        else if(props.dir_type==FD_VectorField) {
            float rseed = props.random_seed;
            float rdiff = props.random_diffuse;
            vec3f rcp_cell = props.rcp_cellsize;
            foreach(i=0 ... particle_num) {
                vec3f ppos = get_particle_position(i);
                vec3f a = get_particle_accel(i);
                float af = affection[i];
                float s = lerp(props.strength_far, props.strength_near, pow(af, props.attenuation_exp));
                a = a + VectorField(ppos, rcp_cell, s, rseed, rdiff);
                set_particle_accel(i,a);
            }
        }
    }
}


static inline float sphComputeDensity(const uniform KernelParams &params, vec3f pos1, vec3f pos2)
{
    uniform const float h_sq = params.particle_size * params.particle_size;
    vec3f diff = pos2 - pos1;
    float r_sq = dot(diff, diff);
    if(r_sq < h_sq) {
        // Implements this equation:
        // W_poly6(r, h) = 315 / (64 * pi * h^9) * (h^2 - r^2)^3
        // g_fDensityCoef = fParticleMass * 315.0f / (64.0f * PI * fSmoothlen^9)
        return params.SPHDensityCoef * (h_sq - r_sq) * (h_sq - r_sq) * (h_sq - r_sq);
    }
    return 0.0f;
}

export void sphUpdateDensity( uniform Context &ctx, uniform const vec3i &idx )
{
    uniform const KernelParams kp = *ctx.kparams;
    uniform const Cell &gd = ctx.grid[kp.world_div.x*kp.world_div.z*idx.y + kp.world_div.x*idx.z + idx.x];
    uniform const int particle_num = gd.end - gd.begin;
    expand_particle_params();

    uniform const int nx_beg = max(idx.x-1, 0);
    uniform const int nx_end = min(idx.x+1, kp.world_div.x-1);
    uniform const int ny_beg = max(idx.y-1, 0);
    uniform const int ny_end = min(idx.y+1, kp.world_div.y-1);
    uniform const int nz_beg = max(idx.z-1, 0);
    uniform const int nz_end = min(idx.z+1, kp.world_div.z-1);

    for(uniform int i=0; i<particle_num; ++i) {
        uniform vec3f pos1 = get_particle_position(i);
        float dens = 0.0f;
        for(uniform int nyi=ny_beg; nyi<=ny_end; ++nyi) {
            for(uniform int nzi=nz_beg; nzi<=nz_end; ++nzi) {
                for(uniform int nxi=nx_beg; nxi<=nx_end; ++nxi) {
                    uniform const Cell &ngd = ctx.grid[kp.world_div.x*kp.world_div.z*nyi + kp.world_div.x*nzi + nxi];
                    uniform const int neighbor_num = ngd.end - ngd.begin;
                    expand_neighbor_params();
                    foreach(t=0 ... neighbor_num) {
                        vec3f pos2 = get_neighbor_position(t);
                        dens += sphComputeDensity(kp, pos1, pos2);
                    }
                }
            }
        }
        density[i] = reduce_add(dens);
    }
}


export void sphUpdateDensityEst1(uniform Context &ctx, uniform const vec3i &idx)
{
    uniform const KernelParams kp = *ctx.kparams;
    uniform Cell &gd = ctx.grid[kp.world_div.x*kp.world_div.z*idx.y + kp.world_div.x*idx.z + idx.x];
    uniform const int particle_num = gd.end - gd.begin;
    expand_particle_params();

    for(uniform int i=0; i<particle_num; ++i) {
        uniform vec3f pos1 = get_particle_position(i);
        float dens = 0.0f;

        uniform const Cell &ngd = gd;
        uniform const int neighbor_num = ngd.end - ngd.begin;
        expand_neighbor_params();
        foreach(t=0 ... neighbor_num) {
            vec3f pos2 = get_neighbor_position(t);
            dens += sphComputeDensity(kp, pos1, pos2);
        }
        density[i] = reduce_add(dens);
    }
    {
        gd.density = 0.0f;
        float total_density = 0.0f;
        foreach(i=0 ... particle_num) {
            total_density += density[i];
        }
        gd.density = reduce_add(total_density) / particle_num;
    }
}

export void sphUpdateDensityEst2(uniform Context &ctx, uniform const vec3i &idx)
{
    uniform const KernelParams kp = *ctx.kparams;
    uniform const Cell &gd = ctx.grid[kp.world_div.x*kp.world_div.z*idx.y + kp.world_div.x*idx.z + idx.x];
    uniform const int particle_num = gd.end - gd.begin;
    expand_particle_params();

    uniform const int nx_beg = max(idx.x-1, 0);
    uniform const int nx_end = min(idx.x+1, kp.world_div.x-1);
    uniform const int ny_beg = max(idx.y-1, 0);
    uniform const int ny_end = min(idx.y+1, kp.world_div.y-1);
    uniform const int nz_beg = max(idx.z-1, 0);
    uniform const int nz_end = min(idx.z+1, kp.world_div.z-1);

    for(uniform int nyi=ny_beg; nyi<=ny_end; ++nyi) {
        for(uniform int nzi=nz_beg; nzi<=nz_end; ++nzi) {
            for(uniform int nxi=nx_beg; nxi<=nx_end; ++nxi) {
                uniform const Cell &ngd = ctx.grid[kp.world_div.x*kp.world_div.z*nyi + kp.world_div.x*nzi + nxi];
                foreach(i=0 ... particle_num) {
                    density[i] += ngd.density*0.05f;
                }
            }
        }
    }
}


static inline float sphCalculatePressure(uniform const KernelParams &params, float density)
{
    // Implements this equation:
    // Pressure = B * ((rho / rho_0)^y  - 1)
    return params.pressure_stiffness * max(pow(density / params.SPHRestDensity, 3) - 1.0f, 0.0f);
}
static inline uniform float sphCalculatePressure(uniform const KernelParams &params, uniform float density)
{
    return params.pressure_stiffness * max(pow(density / params.SPHRestDensity, 3) - 1.0f, 0.0f);
}


static inline vec3f sphCalculateGradPressure(uniform const KernelParams &params, float r, float P_pressure, float N_pressure, float N_density, vec3f diff)
{
    const float h = params.particle_size;
    float avg_pressure = 0.5f * (N_pressure + P_pressure);
    // Implements this equation:
    // W_spkiey(r, h) = 15 / (pi * h^6) * (h - r)^3
    // GRAD( W_spikey(r, h) ) = -45 / (pi * h^6) * (h - r)^2
    // g_fGradPressureCoef = fParticleMass * -45.0f / (PI * fSmoothlen^6)
    return (params.SPHGradPressureCoef * avg_pressure / N_density * (h - r) * (h - r) / r) * diff;
}

static inline vec3f sphCalculateLapVelocity(uniform const KernelParams &params, float r, vec3f P_velocity, vec3f N_velocity, float N_density)
{
    const float h = params.particle_size;
    vec3f vel_diff = N_velocity - P_velocity;
    // Implements this equation:
    // W_viscosity(r, h) = 15 / (2 * pi * h^3) * (-r^3 / (2 * h^3) + r^2 / h^2 + h / (2 * r) - 1)
    // LAPLACIAN( W_viscosity(r, h) ) = 45 / (pi * h^6) * (h - r)
    // g_fLapViscosityCoef = fParticleMass * fViscosity * 45.0f / (PI * fSmoothlen^6)
    return  (params.SPHLapViscosityCoef / N_density * (h - r)) * vel_diff;
}

static inline float sphCalculateDensity(uniform const KernelParams &params, float r_sq)
{
    const float h_sq = params.particle_size * params.particle_size;
    // Implements this equation:
    // W_poly6(r, h) = 315 / (64 * pi * h^9) * (h^2 - r^2)^3
    // g_fDensityCoef = fParticleMass * 315.0f / (64.0f * PI * fSmoothlen^9)
    return params.SPHDensityCoef * (h_sq - r_sq) * (h_sq - r_sq) * (h_sq - r_sq);
}

static inline vec3f sphComputeAccel(
    uniform const KernelParams &params,
    vec3f pos1,
    vec3f pos2,
    vec3f vel1,
    vec3f vel2,
    float pressure1,
    float density2 )
{
    uniform const float h_sq = params.particle_size * params.particle_size;
    vec3f accel = {0.0f, 0.0f, 0.0f};
    vec3f diff = pos2 - pos1;
    float r_sq = dot(diff, diff);
    if(r_sq < h_sq && r_sq > 0.0f) {
        float pressure2 = sphCalculatePressure(params, density2);
        float r = sqrt(r_sq);

        // Pressure Term
        accel = accel + sphCalculateGradPressure(params, r, pressure1, pressure2, density2, diff);

        // Viscosity Term
        accel = accel + sphCalculateLapVelocity(params, r, vel1, vel2, density2);
    }
    return accel;
}

export void sphUpdateForce(uniform Context &ctx, uniform const vec3i &idx)
{
    uniform const KernelParams kp = *ctx.kparams;
    uniform const Cell &gd = ctx.grid[kp.world_div.x*kp.world_div.z*idx.y + kp.world_div.x*idx.z + idx.x];
    uniform const int particle_num = gd.end - gd.begin;
    expand_particle_params();

    uniform const int nx_beg = max(idx.x-1, 0);
    uniform const int nx_end = min(idx.x+1, kp.world_div.x-1);
    uniform const int ny_beg = max(idx.y-1, 0);
    uniform const int ny_end = min(idx.y+1, kp.world_div.y-1);
    uniform const int nz_beg = max(idx.z-1, 0);
    uniform const int nz_end = min(idx.z+1, kp.world_div.z-1);

    for(uniform int i=0; i<particle_num; ++i) {
        uniform vec3f pos1 = get_particle_position(i);
        uniform vec3f vel1 = get_particle_velocity(i);
        uniform float density1 = density[i];
        uniform float pressure1 = sphCalculatePressure(kp, density1);

        vec3f accel = {0.0f, 0.0f, 0.0f};
        for(uniform int nyi=ny_beg; nyi<=ny_end; ++nyi) {
            for(uniform int nzi=nz_beg; nzi<=nz_end; ++nzi) {
                for(uniform int nxi=nx_beg; nxi<=nx_end; ++nxi) {
                    uniform const Cell &ngd = ctx.grid[kp.world_div.x*kp.world_div.z*nyi + kp.world_div.x*nzi + nxi];
                    uniform const int neighbor_num = ngd.end - ngd.begin;
                    expand_neighbor_params();
                    foreach(t=0 ... neighbor_num) {
                        vec3f pos2 = get_neighbor_position(t);
                        vec3f vel2 = get_neighbor_velocity(t);
                        float density2 = ndensity[t];
                        accel = accel + sphComputeAccel(kp, pos1, pos2, vel1, vel2, pressure1, density2);
                    }
                }
            }
        }

        uniform vec3f a = reduce_add(accel);
        set_particle_accel(i,a);
    }
}


export void impUpdatePressure(uniform Context &ctx, uniform const vec3i &idx)
{
    uniform const KernelParams kp = *ctx.kparams;
    uniform const Cell &gd = ctx.grid[kp.world_div.x*kp.world_div.z*idx.y + kp.world_div.x*idx.z + idx.x];
    uniform const int particle_num = gd.end - gd.begin;
    float advection = kp.advection;
    expand_particle_params();

    uniform const int nx_beg = max(idx.x-1, 0);
    uniform const int nx_end = min(idx.x+1, kp.world_div.x-1);
    uniform const int ny_beg = max(idx.y-1, 0);
    uniform const int ny_end = min(idx.y+1, kp.world_div.y-1);
    uniform const int nz_beg = max(idx.z-1, 0);
    uniform const int nz_end = min(idx.z+1, kp.world_div.z-1);

    for(uniform int i=0; i<particle_num; ++i) {
        uniform vec3f pos1 = get_particle_position(i);
        uniform vec3f vel1 = get_particle_velocity(i);
        vec3f accel = {0.0f, 0.0f, 0.0f};
        for(uniform int nyi=ny_beg; nyi<=ny_end; ++nyi) {
            for(uniform int nzi=nz_beg; nzi<=nz_end; ++nzi) {
                for(uniform int nxi=nx_beg; nxi<=nx_end; ++nxi) {
                    uniform const Cell &ngd = ctx.grid[kp.world_div.x*kp.world_div.z*nyi + kp.world_div.x*nzi + nxi];
                    uniform const int neighbor_num = ngd.end - ngd.begin;
                    expand_neighbor_params();
                    foreach(t=0 ... neighbor_num) {
                        vec3f pos2 = get_neighbor_position(t);
                        vec3f vel2 = get_neighbor_velocity(t);
                        vec3f diff = pos2 - pos1;
                        vec3f dir = diff * kp.RcpParticleSize2; // vec3 dir = diff / d;
                        float d = length(diff);
                        if(d > 0.0f) { // d==0: same particle
                            accel = accel + dir * (min(0.0f, d-(kp.particle_size*2.0f)) * kp.pressure_stiffness);
                            accel = accel + (vel2-vel1) * advection;
                        }
                    }
                }
            }
        }

        uniform vec3f a = reduce_add(accel);
        set_particle_accel(i,a);
    }
}

export void Integrate(uniform Context &ctx, uniform const vec3i &idx)
{
    uniform const KernelParams kp = *ctx.kparams;
    uniform const Cell &gd = ctx.grid[kp.world_div.x*kp.world_div.z*idx.y + kp.world_div.x*idx.z + idx.x];
    uniform const int particle_num = gd.end - gd.begin;
    expand_particle_params();

    vec3f coord_scaler = kp.coord_scaler;
    float timestep = kp.timestep;
    float decel = pow(abs(kp.damping), timestep);

    foreach(i=0 ... particle_num) {
        vec3f pos = get_particle_position(i);
        vec3f vel = get_particle_velocity(i);
        vec3f accel = get_particle_accel(i);

        vel = vel + accel * timestep;
        vel = vel * decel;
        vel = vel * coord_scaler;

        pos = pos + vel * timestep;
        pos = pos * coord_scaler;

        set_particle_position(i,pos);
        set_particle_velocity(i,vel);
        speed[i] = length(vel);

        vec3f a = accel * -timestep;
        set_particle_accel(i,a);
    }
}
