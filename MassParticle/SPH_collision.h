#ifndef __SPH_collision_h__
#define __SPH_collision_h__

#include "ispc_vectormath.h"
#include "SPH_const.h"
typedef unsigned int32 id_type;


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

struct Plane
{
    float nx, ny, nz;
    float distance;
};

struct Box
{
    Plane planes[6];
};

struct SphereCollider
{
    id_type id;
    BoundingBox bb;
    float x, y, z;
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


struct Particle
{
    float   x, y, z;
    float   vx, vy, vz;
    float   density;
    int32   hit;
};

struct Force
{
    float ax, ay, az;
};

struct GridData
{
    int32 begin, end;
    int32 soai;
    float density;
};


// struct ‚ÌQÆ“n‚µ‚ª‚Å‚«‚È‚¢‚Á‚Û‚¢‚Ì‚Å macro ‚Å...
// •¡”‚ÌŒ^‚É‘Î‰‚Å‚«‚é‚Ì‚Å‚Ş‚µ‚ë‚±‚Á‚¿‚Ì‚ª‚¢‚¢‚Ì‚©‚à
#define get_pos(p)      {p.x, p.y, p.z}
#define get_vel(p)      {p.vx, p.vy, p.vz}
#define get_normal(p)   {p.nx, p.ny, p.nz}
#define get_accel(p)    {p.ax, p.ay, p.az}
#define set_pos(p, v)   p.x=v.x; p.y=v.y; p.z=v.z;
#define set_vel(p, v)   p.vx=v.x; p.vy=v.y; p.vz=v.z;
#define set_accel(p, v) p.ax=v.x; p.ay=v.y; p.az=v.z;


#endif // __SPH_collision_h__
