#ifndef _ispc_vectormath_h_
#define _ispc_vectormath_h_

//typedef float<3> vec3f;
//typedef int<3> vec3i;

struct vec3f { float x, y, z; };
struct vec3i { int x, y, z; };
static inline vec3f operator+(vec3f a, vec3f b) { vec3f r = { a.x + b.x, a.y + b.y, a.z + b.z }; return r; }
static inline vec3f operator-(vec3f a, vec3f b) { vec3f r = { a.x - b.x, a.y - b.y, a.z - b.z }; return r; }
static inline vec3f operator*(vec3f a, vec3f b) { vec3f r = { a.x * b.x, a.y * b.y, a.z * b.z }; return r; }
static inline vec3f operator/(vec3f a, vec3f b) { vec3f r = { a.x / b.x, a.y / b.y, a.z / b.z }; return r; }
static inline vec3f operator+(vec3f a, float b) { vec3f r = { a.x + b, a.y + b, a.z + b }; return r; }
static inline vec3f operator-(vec3f a, float b) { vec3f r = { a.x - b, a.y - b, a.z - b }; return r; }
static inline vec3f operator*(vec3f a, float b) { vec3f r = { a.x * b, a.y * b, a.z * b }; return r; }
static inline vec3f operator/(vec3f a, float b) { vec3f r = { a.x / b, a.y / b, a.z / b }; return r; }
static inline vec3f operator+(float a, vec3f b) { vec3f r = { a + b.x, a + b.y, a + b.z }; return r; }
static inline vec3f operator-(float a, vec3f b) { vec3f r = { a - b.x, a - b.y, a - b.z }; return r; }
static inline vec3f operator*(float a, vec3f b) { vec3f r = { a * b.x, a * b.y, a * b.z }; return r; }
static inline vec3f operator/(float a, vec3f b) { vec3f r = { a / b.x, a / b.y, a / b.z }; return r; }
static inline vec3f operator+(vec3f a, vec3i b) { vec3f r = { a.x + b.x, a.y + b.y, a.z + b.z }; return r; }
static inline vec3f operator-(vec3f a, vec3i b) { vec3f r = { a.x - b.x, a.y - b.y, a.z - b.z }; return r; }
static inline vec3f operator*(vec3f a, vec3i b) { vec3f r = { a.x * b.x, a.y * b.y, a.z * b.z }; return r; }
static inline vec3f operator/(vec3f a, vec3i b) { vec3f r = { a.x / b.x, a.y / b.y, a.z / b.z }; return r; }

static inline vec3i operator+(vec3i a, vec3i b) { vec3i r = { a.x + b.x, a.y + b.y, a.z + b.z }; return r; }
static inline vec3i operator-(vec3i a, vec3i b) { vec3i r = { a.x - b.x, a.y - b.y, a.z - b.z }; return r; }
static inline vec3i operator*(vec3i a, vec3i b) { vec3i r = { a.x * b.x, a.y * b.y, a.z * b.z }; return r; }
//static inline vec3i operator/(vec3i a, vec3i b) { vec3i r = { a.x / b.x, a.y / b.y, a.x / b.z }; return r; }

static inline uniform vec3f operator+(uniform vec3f a, uniform vec3f b) { uniform vec3f r = { a.x + b.x, a.y + b.y, a.z + b.z }; return r; }
static inline uniform vec3f operator-(uniform vec3f a, uniform vec3f b) { uniform vec3f r = { a.x - b.x, a.y - b.y, a.z - b.z }; return r; }
static inline uniform vec3f operator*(uniform vec3f a, uniform vec3f b) { uniform vec3f r = { a.x * b.x, a.y * b.y, a.z * b.z }; return r; }
static inline uniform vec3f operator/(uniform vec3f a, uniform vec3f b) { uniform vec3f r = { a.x / b.x, a.y / b.y, a.z / b.z }; return r; }
static inline uniform vec3f operator+(uniform vec3f a, uniform float b) { uniform vec3f r = { a.x + b, a.y + b, a.z + b }; return r; }
static inline uniform vec3f operator-(uniform vec3f a, uniform float b) { uniform vec3f r = { a.x - b, a.y - b, a.z - b }; return r; }
static inline uniform vec3f operator*(uniform vec3f a, uniform float b) { uniform vec3f r = { a.x * b, a.y * b, a.z * b }; return r; }
static inline uniform vec3f operator/(uniform vec3f a, uniform float b) { uniform vec3f r = { a.x / b, a.y / b, a.z / b }; return r; }
static inline uniform vec3f operator+(uniform float a, uniform vec3f b) { uniform vec3f r = { a + b.x, a + b.y, a + b.z }; return r; }
static inline uniform vec3f operator-(uniform float a, uniform vec3f b) { uniform vec3f r = { a - b.x, a - b.y, a - b.z }; return r; }
static inline uniform vec3f operator*(uniform float a, uniform vec3f b) { uniform vec3f r = { a * b.x, a * b.y, a * b.z }; return r; }
static inline uniform vec3f operator/(uniform float a, uniform vec3f b) { uniform vec3f r = { a / b.x, a / b.y, a / b.z }; return r; }
static inline uniform vec3f operator+(uniform vec3f a, uniform vec3i b) { uniform vec3f r = { a.x + b.x, a.y + b.y, a.z + b.z }; return r; }
static inline uniform vec3f operator-(uniform vec3f a, uniform vec3i b) { uniform vec3f r = { a.x - b.x, a.y - b.y, a.z - b.z }; return r; }
static inline uniform vec3f operator*(uniform vec3f a, uniform vec3i b) { uniform vec3f r = { a.x * b.x, a.y * b.y, a.z * b.z }; return r; }
static inline uniform vec3f operator/(uniform vec3f a, uniform vec3i b) { uniform vec3f r = { a.x / b.x, a.y / b.y, a.z / b.z }; return r; }

static inline uniform vec3i operator+(uniform vec3i a, uniform vec3i b) { uniform vec3i r = { a.x + b.x, a.y + b.y, a.z + b.z }; return r; }
static inline uniform vec3i operator-(uniform vec3i a, uniform vec3i b) { uniform vec3i r = { a.x - b.x, a.y - b.y, a.z - b.z }; return r; }
static inline uniform vec3i operator*(uniform vec3i a, uniform vec3i b) { uniform vec3i r = { a.x * b.x, a.y * b.y, a.z * b.z }; return r; }
static inline uniform vec3i operator/(uniform vec3i a, uniform vec3i b) { uniform vec3i r = { a.x / b.x, a.y / b.y, a.z / b.z }; return r; }

static inline uniform vec3f reduce_add(vec3f v)
{
    uniform vec3f r = {reduce_add(v.x), reduce_add (v.y), reduce_add(v.z)};
    return r;
}

static inline vec3f rsqrt3(vec3f a)
{
    vec3f r = { rsqrt(a.x), rsqrt(a.y), rsqrt(a.z)};
    return r;
}

static inline uniform vec3f rsqrt3(uniform vec3f a)
{
    uniform vec3f r = { rsqrt(a.x), rsqrt(a.y), rsqrt(a.z) };
    return r;
}

static inline float dot3(vec3f a, vec3f b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline uniform float dot3(uniform vec3f a, uniform vec3f b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}


static inline vec3f cross3(vec3f v0, vec3f v1)
{
    vec3f ret;
    ret.x = v0.y*v1.z - v0.z*v1.y;
    ret.y = v0.z*v1.x - v0.x*v1.z;
    ret.z = v0.x*v1.y - v0.y*v1.x;
    return ret;
}

static inline uniform vec3f cross3(uniform vec3f v0, uniform vec3f v1)
{
    uniform vec3f ret;
    ret.x = v0.y*v1.z - v0.z*v1.y;
    ret.y = v0.z*v1.x - v0.x*v1.z;
    ret.z = v0.x*v1.y - v0.y*v1.x;
    return ret;
}


static inline float length_sq3(vec3f v)
{
    return dot3(v, v);
}
static inline uniform float length_sq3(uniform vec3f v)
{
    return dot3(v, v);
}

static inline float length3(vec3f v)
{
    return sqrt(length_sq3(v));
}
static inline uniform float length3(uniform vec3f v)
{
    return sqrt(length_sq3(v));
}

// somehow length_est3() is slower than length3()...
static inline float length_est3(vec3f v)
{
    float t = length_sq3(v);
    return rsqrt(t)*t;
}
static inline uniform float length_est3(uniform vec3f v)
{
    uniform float t = length_sq3(v);
    return rsqrt(t)*t;
}

static inline float rcp_length3(vec3f v)
{
    return rsqrt(length_sq3(v));
}
static inline uniform float rcp_length3(uniform vec3f v)
{
    return rsqrt(length_sq3(v));
}

static inline vec3f normalize3(vec3f v)
{
    return v * rsqrt(dot3(v, v));
}
static inline uniform vec3f normalize3(uniform vec3f v)
{
    return v * rsqrt(dot3(v, v));
}

static inline float lerp(float a, float b, float t) {
    return (1.0f-t)*a + t*b;
}
static inline uniform float lerp(uniform float a, uniform float b, uniform float t) {
    return (1.0f-t)*a + t*b;
}

static inline vec3f lerp3(vec3f a, vec3f b, float t) {
    return (1.0f-t)*a + t*b;
}
static inline uniform vec3f lerp3(uniform vec3f a, uniform vec3f b, uniform float t) {
    return (1.0f-t)*a + t*b;
}

static inline float clamp_and_normalize(float v, float low, float high, float rcp_range) {
    float r = (v - low)*rcp_range;
    return clamp(r, 0.0f, 1.0f);
}


#endif // _ispc_vectormath_h_
