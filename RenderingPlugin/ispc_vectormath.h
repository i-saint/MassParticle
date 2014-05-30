#ifndef _ispc_vectormath_h_
#define _ispc_vectormath_h_

typedef float<2> vec2;
typedef float<3> vec3;
typedef float<4> vec4;


static inline float dot3(vec3 a, vec3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline uniform float dot3(uniform vec3 a, uniform vec3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}


static inline vec3 cross3(vec3 v0, vec3 v1)
{
    vec3 ret;
    ret.x = v0.y*v1.z - v0.z*v1.y;
    ret.y = v0.z*v1.x - v0.x*v1.z;
    ret.z = v0.x*v1.y - v0.y*v1.x;
    return ret;
}

static inline uniform vec3 cross3(uniform vec3 v0, uniform vec3 v1)
{
    uniform vec3 ret;
    ret.x = v0.y*v1.z - v0.z*v1.y;
    ret.y = v0.z*v1.x - v0.x*v1.z;
    ret.z = v0.x*v1.y - v0.y*v1.x;
    return ret;
}


static inline float length_sq3(vec3 v)
{
    return dot3(v, v);
}

static inline uniform float length_sq3(uniform vec3 v)
{
    return dot3(v, v);
}


static inline float length3(vec3 v)
{
    return sqrt(length_sq3(v));
}

static inline uniform float length3(uniform vec3 v)
{
    return sqrt(length_sq3(v));
}

static inline float length3est(vec3 v)
{
    float t = length_sq3(v);
    return rsqrt(t)*t;
}

static inline uniform float length3est(uniform vec3 v)
{
    uniform float t = length_sq3(v);
    return rsqrt(t)*t;
}


static inline vec3 normalize3(vec3 v)
{
    float len2 = dot3(v, v);
    float invlen = rsqrt(len2);
    return v * invlen;
}

static inline uniform vec3 normalize3(uniform vec3 v)
{
    uniform float len2 = dot3(v, v);
    uniform float invlen = rsqrt(len2);
    return v * invlen;
}

#endif // _ispc_vectormath_h_
