#ifndef mpCollision_h
#define mpCollision_h

#include "ispc_vectormath.h"
typedef int id_type;


struct Plane
{
	float nx, ny, nz;
	float distance;
};

struct BoundingBox
{
	float bl_x, bl_y, bl_z;
	float ur_x, ur_y, ur_z;
};

struct Sphere
{
	float x, y, z;
	float radius;
};

struct Box
{
	Plane planes[6];
	float x, y, z;
};

struct SphereCollider
{
	id_type id;
	BoundingBox bb;
	float x, y, z;
	float radius;
};

struct CapsuleCollider
{
	id_type id;
	BoundingBox bb;
	float x1, y1, z1;
	float x2, y2, z2;
	float radius;
};

struct PlaneCollider
{
	id_type id;
	BoundingBox bb;
	float nx, ny, nz;
	float distance;
};

struct BoxCollider
{
	id_type id;
	BoundingBox bb;
	float x, y, z;
	Plane planes[6];
};


struct PointForce
{
	float x, y, z;
	float strength;
};

struct DirectionalForce
{
	float nx, ny, nz;
	float strength;
};

struct BoxForce
{
	float nx, ny, nz;
	float strength;
	Box box;
};


enum ForceShape
{
	FS_AffectAll,
	FS_Sphere,
	FS_Box,
};

enum ForceDirection
{
	FD_Directional,
	FD_Radial,
	FD_Vortex,      // todo:
	FD_Spline,      // 
	FD_VectorField, //
};

struct ForceParams
{
	float x, y, z; // direction for plane, position for other shapes
	float strength;
	float strength_random_diffuse;
};

struct Force
{
	int shape_type; // ForceShape
	int dir_type; // ForceDirection

	BoundingBox bb;
	// union doesn't exists in ISPC :(
	Sphere              shape_sphere;
	Box                 shape_box;

	ForceParams params;
};


struct Particle
{
	float   x, y, z;
	float   vx, vy, vz;
	float   speed;
	float   density;
	int     hit;
};

struct ParticleIMData
{
	float ax, ay, az;
	float affection;
	float distance_sq;
};

struct GridData
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
	float WorldSize_x;
	float WorldSize_y;
	float WorldSize_z;
	float WorldSize_pad;
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

// struct ÇÃéQè∆ìnÇµÇ™Ç≈Ç´Ç»Ç¢Ç¡Ç€Ç¢ÇÃÇ≈ macro Ç≈...
// ï°êîÇÃå^Ç…ëŒâûÇ≈Ç´ÇÈÇÃÇ≈ÇﬁÇµÇÎÇ±Ç¡ÇøÇÃÇ™Ç¢Ç¢ÇÃÇ©Ç‡
#define get_pos(p)      {p.x, p.y, p.z}
#define get_vel(p)      {p.vx, p.vy, p.vz}
#define get_normal(p)   {p.nx, p.ny, p.nz}
#define get_accel(p)    {p.ax, p.ay, p.az}
#define set_pos(p, v)   p.x=v.x; p.y=v.y; p.z=v.z;
#define set_vel(p, v)   p.vx=v.x; p.vy=v.y; p.vz=v.z;
#define set_accel(p, v) p.ax=v.x; p.ay=v.y; p.az=v.z;
#define get_bl(p)       {p.bl_x, p.bl_y, p.bl_z}
#define get_ur(p)       {p.ur_x, p.ur_y, p.ur_z}


#endif // mpCollision_h
