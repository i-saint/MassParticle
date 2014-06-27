#ifndef mpCollision_h
#define mpCollision_h

#include "ispc_vectormath.h"
typedef int id_type;
typedef unsigned int uint;

struct vec3f
{
	float x, y, z;
};
struct vec3i
{
	int x, y, z;
};

struct Plane
{
	vec3f normal;
	float distance;
};

struct BoundingBox
{
	vec3f bl, ur;
};

struct Sphere
{
	vec3f center;
	float radius;
};

struct Capsule
{
	vec3f center, pos1, pos2;
	float radius;
	float rcp_lensq; // for optimization
};

struct Box
{
	vec3f center;
	Plane planes[6];
};


struct ColliderProperties
{
	uint group_mask;
	int owner_id;
	float stiffness;
	float rebound;
	float lifetime_on_hit;
};

struct PlaneCollider
{
	ColliderProperties props;
	BoundingBox bounds;
	Plane shape;
};

struct SphereCollider
{
	ColliderProperties props;
	BoundingBox bounds;
	Sphere shape;
};

struct CapsuleCollider
{
	ColliderProperties props;
	BoundingBox bounds;
	Capsule shape;
};

struct BoxCollider
{
	ColliderProperties props;
	BoundingBox bounds;
	Box shape;
};


enum ForceShape
{
	FS_AffectAll,
	FS_Sphere,
	FS_Capsule,
	FS_Box,
};

enum ForceDirection
{
	FD_Directional,
	FD_Radial,
	FD_RadialCapsule,
	FD_Vortex,      // todo:
	FD_Spline,      // 
	FD_VectorField, //
};

struct ForceProperties
{
	uint	group_mask;
	int		shape_type; // ForceShape
	int		dir_type; // ForceDirection
	float	strength_near;
	float	strength_far;
	float	range_inner;
	float	range_outer;
	float	attenuation_exp;

	vec3f	directional_pos;
	vec3f	directional_dir;
	vec3f	radial_center;
	vec3f	vortex_pos;
	vec3f	vortex_axis;
	float	vortex_pull;

	float	directional_plane_distance;
	float rcp_range;
};

struct Force
{
	ForceProperties	props;
	BoundingBox		bounds;
	Sphere			sphere;
	Capsule			capsule;
	Box				box;
};


struct Particle
{
	float	x, y, z;
	float	vx, vy, vz;
	float	speed;
	float	density;
	int		hit;
};

struct ParticleIMData
{
	float ax, ay, az;
	float inside_force;
};

struct Cell
{
	int begin, end;
	int soai;
	float density;
};

struct KernelParams
{
	float WorldCenter_x;
	float WorldCenter_y;
	float WorldCenter_z;
	float WorldCenter_pad;
	float WorldExtent_x;
	float WorldExtent_y;
	float WorldExtent_z;
	float WorldExtent_pad;
	int WorldDiv_x;
	int WorldDiv_y;
	int WorldDiv_z;
	int WorldDiv_pad;
	float Scale_x;
	float Scale_y;
	float Scale_z;
	float Scale_pad;

	int SolverType;
	float LifeTime;
	float Timestep;
	float Decelerate;
	float PressureStiffness;
	float WallStiffness;

	int MaxParticles;
	float ParticleSize;

	float SPHRestDensity;
	float SPHParticleMass;
	float SPHViscosity;
};

#define to_vec3(v) {v.x, v.y,v.z}
#define gather3(v, ex, ey, ez) {v.ex, v.ey,v.ez}
#define store3(t, ex, ey, ez, v) t.ex=v.x; t.ey=v.y; t.ez=v.z;

#endif // mpCollision_h
