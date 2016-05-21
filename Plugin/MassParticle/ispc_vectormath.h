#ifndef _ispc_vectormath_h_
#define _ispc_vectormath_h_

//typedef float<3> vec3f;
//typedef int<3> vec3i;

struct vec2f { float x, y; };
struct vec2i { int   x, y; };
struct vec3f { float x, y, z; };
struct vec3i { int   x, y, z; };

vec2f v2f() { vec2f r = { 0.0, 0.0 }; return r; }
vec2f v2f(float a) { vec2f r = { a, a }; return r; }
vec2f v2f(float a, float b) { vec2f r = {a, b }; return r; }

vec3f v3f() { vec3f r = { 0.0, 0.0, 0.0 }; return r; }
vec3f v3f(float a) { vec3f r = { a, a, a }; return r; }
vec3f v3f(float a, float b, float c) { vec3f r = { a, b, c }; return r; }
vec3f v3f(vec2f a, float b) { vec3f r = { a.x, a.y, b }; return r; }
vec3f v3f(float a, vec2f b) { vec3f r = { a, b.x, b.y }; return r; }

static inline vec2f operator+(vec2f a, vec2f b) { vec2f r = { a.x + b.x, a.y + b.y }; return r; }
static inline vec2f operator-(vec2f a, vec2f b) { vec2f r = { a.x - b.x, a.y - b.y }; return r; }
static inline vec2f operator*(vec2f a, vec2f b) { vec2f r = { a.x * b.x, a.y * b.y }; return r; }
static inline vec2f operator/(vec2f a, vec2f b) { vec2f r = { a.x / b.x, a.y / b.y }; return r; }
static inline vec2f operator+(vec2f a, float b) { vec2f r = { a.x + b, a.y + b }; return r; }
static inline vec2f operator-(vec2f a, float b) { vec2f r = { a.x - b, a.y - b }; return r; }
static inline vec2f operator*(vec2f a, float b) { vec2f r = { a.x * b, a.y * b }; return r; }
static inline vec2f operator/(vec2f a, float b) { vec2f r = { a.x / b, a.y / b }; return r; }
static inline vec2f operator+(float a, vec2f b) { vec2f r = { a + b.x, a + b.y }; return r; }
static inline vec2f operator-(float a, vec2f b) { vec2f r = { a - b.x, a - b.y }; return r; }
static inline vec2f operator*(float a, vec2f b) { vec2f r = { a * b.x, a * b.y }; return r; }
static inline vec2f operator/(float a, vec2f b) { vec2f r = { a / b.x, a / b.y }; return r; }
static inline vec2f operator+(vec2f a, vec3i b) { vec2f r = { a.x + b.x, a.y + b.y }; return r; }
static inline vec2f operator-(vec2f a, vec3i b) { vec2f r = { a.x - b.x, a.y - b.y }; return r; }
static inline vec2f operator*(vec2f a, vec3i b) { vec2f r = { a.x * b.x, a.y * b.y }; return r; }
static inline vec2f operator/(vec2f a, vec3i b) { vec2f r = { a.x / b.x, a.y / b.y }; return r; }

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

static inline uniform vec2f operator+(uniform vec2f a, uniform vec2f b) { uniform vec2f r = { a.x + b.x, a.y + b.y }; return r; }
static inline uniform vec2f operator-(uniform vec2f a, uniform vec2f b) { uniform vec2f r = { a.x - b.x, a.y - b.y }; return r; }
static inline uniform vec2f operator*(uniform vec2f a, uniform vec2f b) { uniform vec2f r = { a.x * b.x, a.y * b.y }; return r; }
static inline uniform vec2f operator/(uniform vec2f a, uniform vec2f b) { uniform vec2f r = { a.x / b.x, a.y / b.y }; return r; }
static inline uniform vec2f operator+(uniform vec2f a, uniform float b) { uniform vec2f r = { a.x + b, a.y + b }; return r; }
static inline uniform vec2f operator-(uniform vec2f a, uniform float b) { uniform vec2f r = { a.x - b, a.y - b }; return r; }
static inline uniform vec2f operator*(uniform vec2f a, uniform float b) { uniform vec2f r = { a.x * b, a.y * b }; return r; }
static inline uniform vec2f operator/(uniform vec2f a, uniform float b) { uniform vec2f r = { a.x / b, a.y / b }; return r; }
static inline uniform vec2f operator+(uniform float a, uniform vec2f b) { uniform vec2f r = { a + b.x, a + b.y }; return r; }
static inline uniform vec2f operator-(uniform float a, uniform vec2f b) { uniform vec2f r = { a - b.x, a - b.y }; return r; }
static inline uniform vec2f operator*(uniform float a, uniform vec2f b) { uniform vec2f r = { a * b.x, a * b.y }; return r; }
static inline uniform vec2f operator/(uniform float a, uniform vec2f b) { uniform vec2f r = { a / b.x, a / b.y }; return r; }
static inline uniform vec2f operator+(uniform vec2f a, uniform vec3i b) { uniform vec2f r = { a.x + b.x, a.y + b.y }; return r; }
static inline uniform vec2f operator-(uniform vec2f a, uniform vec3i b) { uniform vec2f r = { a.x - b.x, a.y - b.y }; return r; }
static inline uniform vec2f operator*(uniform vec2f a, uniform vec3i b) { uniform vec2f r = { a.x * b.x, a.y * b.y }; return r; }
static inline uniform vec2f operator/(uniform vec2f a, uniform vec3i b) { uniform vec2f r = { a.x / b.x, a.y / b.y }; return r; }

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

static inline uniform vec2f reduce_add(vec2f v)
{
    uniform vec2f r = { reduce_add(v.x), reduce_add(v.y) };
    return r;
}
static inline uniform vec3f reduce_add(vec3f v)
{
    uniform vec3f r = {reduce_add(v.x), reduce_add(v.y), reduce_add(v.z)};
    return r;
}

static inline float mod(float a, float b)
{
    return a - b * floor(a / b);
}
static inline uniform float mod(uniform float a, uniform float b)
{
    return a - b * floor(a / b);
}

static inline float frac(float a)
{
    return mod(a, 1.0);
}
static inline uniform float frac(uniform float a)
{
    return mod(a, 1.0);
}


#define define_vmath1(f)\
    static inline vec2f f(vec2f a)\
    {\
        vec2f r = { f(a.x), f(a.y) };\
        return r;\
    }\
    static inline uniform vec2f f(uniform vec2f a)\
    {\
        uniform vec2f r = { f(a.x), f(a.y) };\
        return r;\
    }\
    static inline vec3f f(vec3f a)\
    {\
        vec3f r = { f(a.x), f(a.y), f(a.z) };\
        return r;\
    }\
    static inline uniform vec3f f(uniform vec3f a)\
    {\
        uniform vec3f r = { f(a.x), f(a.y), f(a.z) };\
        return r;\
    }

#define define_vmath2(f)\
    static inline vec2f f(vec2f a, vec2f b)\
    {\
        vec2f r = { f(a.x, b.x), f(a.y, b.y) };\
        return r;\
    }\
    static inline uniform vec2f f(uniform vec2f a, uniform vec2f b)\
    {\
        uniform vec2f r = { f(a.x, b.x), f(a.y, b.y) };\
        return r;\
    }\
    static inline vec3f f(vec3f a, vec3f b)\
    {\
        vec3f r = { f(a.x, b.x), f(a.y, b.y), f(a.z, b.z) };\
        return r;\
    }\
    static inline uniform vec3f f(uniform vec3f a, uniform vec3f b)\
    {\
        uniform vec3f r = { f(a.x, b.x), f(a.y, b.y), f(a.z, b.z) };\
        return r;\
    }

define_vmath1(abs)
define_vmath1(round)
define_vmath1(floor)
define_vmath1(ceil)
define_vmath2(min)
define_vmath2(max)
define_vmath1(rcp)
define_vmath1(sqrt)
define_vmath1(rsqrt)
define_vmath1(sin)
define_vmath1(cos)
define_vmath1(tan)
define_vmath1(asin)
define_vmath1(acos)
define_vmath1(atan)
define_vmath2(atan2)
define_vmath1(exp)
define_vmath1(log)
define_vmath2(pow)
define_vmath2(mod)
define_vmath1(frac)



static inline float dot(vec2f a, vec2f b)
{
    return a.x*b.x + a.y*b.y;
}
static inline uniform float dot(uniform vec2f a, uniform vec2f b)
{
    return a.x*b.x + a.y*b.y;
}
static inline float dot(vec3f a, vec3f b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
static inline uniform float dot(uniform vec3f a, uniform vec3f b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}


static inline vec3f cross(vec3f v0, vec3f v1)
{
    vec3f ret;
    ret.x = v0.y*v1.z - v0.z*v1.y;
    ret.y = v0.z*v1.x - v0.x*v1.z;
    ret.z = v0.x*v1.y - v0.y*v1.x;
    return ret;
}
static inline uniform vec3f cross(uniform vec3f v0, uniform vec3f v1)
{
    uniform vec3f ret;
    ret.x = v0.y*v1.z - v0.z*v1.y;
    ret.y = v0.z*v1.x - v0.x*v1.z;
    ret.z = v0.x*v1.y - v0.y*v1.x;
    return ret;
}



static inline float length_sq(vec2f v)
{
    return dot(v, v);
}
static inline uniform float length_sq(uniform vec2f v)
{
    return dot(v, v);
}
static inline float length_sq(vec3f v)
{
    return dot(v, v);
}
static inline uniform float length_sq(uniform vec3f v)
{
    return dot(v, v);
}

static inline float length(vec2f v)
{
    return sqrt(length_sq(v));
}
static inline uniform float length(uniform vec2f v)
{
    return sqrt(length_sq(v));
}
static inline float length(vec3f v)
{
    return sqrt(length_sq(v));
}
static inline uniform float length(uniform vec3f v)
{
    return sqrt(length_sq(v));
}

// somehow length_est() is slower than length()...
static inline float length_est(vec3f v)
{
    float t = length_sq(v);
    return rsqrt(t)*t;
}
static inline uniform float length_est(uniform vec3f v)
{
    uniform float t = length_sq(v);
    return rsqrt(t)*t;
}

static inline float rcp_length(vec2f v)
{
    return rsqrt(length_sq(v));
}
static inline uniform float rcp_length(uniform vec2f v)
{
    return rsqrt(length_sq(v));
}
static inline float rcp_length(vec3f v)
{
    return rsqrt(length_sq(v));
}
static inline uniform float rcp_length(uniform vec3f v)
{
    return rsqrt(length_sq(v));
}

static inline vec2f normalize(vec2f v)
{
    return v * rsqrt(dot(v, v));
}
static inline uniform vec2f normalize(uniform vec2f v)
{
    return v * rsqrt(dot(v, v));
}
static inline vec3f normalize(vec3f v)
{
    return v * rsqrt(dot(v, v));
}
static inline uniform vec3f normalize(uniform vec3f v)
{
    return v * rsqrt(dot(v, v));
}

static inline float lerp(float a, float b, float t) {
    return (1.0f-t)*a + t*b;
}
static inline uniform float lerp(uniform float a, uniform float b, uniform float t) {
    return (1.0f-t)*a + t*b;
}

static inline vec3f lerp(vec3f a, vec3f b, float t) {
    return (1.0f-t)*a + t*b;
}
static inline uniform vec3f lerp(uniform vec3f a, uniform vec3f b, uniform float t) {
    return (1.0f-t)*a + t*b;
}

static inline float clamp_and_normalize(float v, float low, float high, float rcp_range) {
    float r = (v - low)*rcp_range;
    return clamp(r, 0.0f, 1.0f);
}



static inline float iq_rand(float  p)
{
    return frac(sin(p)*43758.5453);
}
static inline vec2f iq_rand(vec2f p)
{
    p = v2f( dot(p, v2f(127.1, 311.7)), dot(p, v2f(269.5, 183.3)) );
    return frac(sin(p)*43758.5453);
}
static inline vec3f iq_rand(vec3f p)
{
    p = v3f(dot(p, v3f(127.1, 311.7, 311.7)), dot(p, v3f(269.5, 183.3, 183.3)), dot(p, v3f(269.5, 183.3, 183.3)));
    return frac(sin(p)*43758.5453);
}

#endif // _ispc_vectormath_h_
