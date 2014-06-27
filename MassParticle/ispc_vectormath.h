#ifndef _ispc_vectormath_h_
#define _ispc_vectormath_h_

typedef float<2> vec2;
typedef float<3> vec3;
typedef float<4> vec4;
typedef int<2> ivec2;
typedef int<3> ivec3;
typedef int<4> ivec4;


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

// somehow length_est3() is slower than length3()...
static inline float length_est3(vec3 v)
{
	float t = length_sq3(v);
	return rsqrt(t)*t;
}
static inline uniform float length_est3(uniform vec3 v)
{
	uniform float t = length_sq3(v);
	return rsqrt(t)*t;
}

static inline float rcp_length3(vec3 v)
{
	return rsqrt(length_sq3(v));
}
static inline uniform float rcp_length3(uniform vec3 v)
{
	return rsqrt(length_sq3(v));
}

static inline vec3 normalize3(vec3 v)
{
	return v * rsqrt(dot3(v, v));
}
static inline uniform vec3 normalize3(uniform vec3 v)
{
	return v * rsqrt(dot3(v, v));
}

static inline float lerp(float a, float b, float t) {
	return (1.0f-t)*a + t*b;
}
static inline uniform float lerp(uniform float a, uniform float b, uniform float t) {
	return (1.0f-t)*a + t*b;
}

static inline vec3 lerp3(vec3 a, vec3 b, float t) {
	return (1.0f-t)*a + t*b;
}
static inline uniform vec3 lerp3(uniform vec3 a, uniform vec3 b, uniform float t) {
	return (1.0f-t)*a + t*b;
}

static inline float clamp_and_normalize(float v, float low, float high, float rcp_range) {
	float r = (v - low)*rcp_range;
	return clamp(r, 0.0f, 1.0f);
}


#endif // _ispc_vectormath_h_
