#ifndef Collision
#define Collision
#include "MPGPFoundation.cginc"

bool IsOverlaped(float3 pos, AABB aabb, float r)
{
    float3 wext = aabb.extents + r;
    float3 rpos = pos - aabb.center;
    rpos = abs(rpos);
    if(rpos.x>wext.x || rpos.y>wext.y || rpos.z>wext.z)
    {
        return false;
    }
    return true;
}


struct DistanceData
{
    float3 direction;
    float distance;
};


DistanceData DistancePointSphere(float3 ppos, Sphere shape)
{
    float3 diff = ppos - shape.center;
    float distance = length(diff)-shape.radius;
    float3 dir = normalize(diff);

    DistanceData ret = {dir, distance};
    return ret;
}


DistanceData DistancePointCapsule(float3 ppos, Capsule shape)
{
    float3 pos1 = shape.pos1;
    float3 pos2 = shape.pos2;
    float3 d = pos2-pos1;

    float t = dot(ppos-pos1, pos2-pos1) / dot(d,d);
    float3 diff;
    if(t<=0.0f) {
        diff = ppos-pos1;
    }
    else if(t>=1.0f) {
        diff = ppos-pos2;
    }
    else {
        float3 nearest = pos1 + (pos2-pos1)*t;
        diff = ppos-nearest;
    }
    float distance = length(diff)-shape.radius;
    float3 dir = normalize(diff);

    DistanceData ret = {dir, distance};
    return ret;
}


DistanceData DistancePointBox(float3 ppos, Box shape)
{
    int inside = 0;
    float closest_distance = -999.0f;
    float3 closest_normal;
    float3 rpos = ppos - shape.center;
    for(int p=0; p<6; ++p) {
        float3 plane_normal = shape.planes[p].normal;
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

#endif // Collision
