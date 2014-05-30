#include <intrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <windows.h>
#include <xnamath.h>

#define SSE_SHUFFLE(x,y,z,w) _MM_SHUFFLE(w,z,y,x)
#define istForceInline __forceinline

//#define __ist_enable_soavec8__



namespace ist {

typedef __m128  simdvec4;
typedef __m128i simdvec4i;
typedef __m256  simdvec8;
typedef __m256i simdvec8i;

#ifndef __XNAMATH_H__
istForceInline simdvec4 operator+(simdvec4 a, simdvec4 b) { return _mm_add_ps(a, b); }
istForceInline simdvec4 operator-(simdvec4 a, simdvec4 b) { return _mm_sub_ps(a, b); }
istForceInline simdvec4 operator*(simdvec4 a, simdvec4 b) { return _mm_mul_ps(a, b); }
istForceInline simdvec4 operator/(simdvec4 a, simdvec4 b) { return _mm_div_ps(a, b); }
#endif // __XNAMATH_H__
istForceInline simdvec4i operator+(simdvec4i a, simdvec4i b) { return _mm_add_epi32(a, b); }
istForceInline simdvec4i operator-(simdvec4i a, simdvec4i b) { return _mm_sub_epi32(a, b); }
istForceInline simdvec4i operator*(simdvec4i a, simdvec4i b) { return _mm_mul_epi32(a, b); }

istForceInline simdvec4& operator+=(simdvec4 &a, simdvec4 b) { return (a = a + b); }
istForceInline simdvec4& operator-=(simdvec4 &a, simdvec4 b) { return (a = a - b); }
istForceInline simdvec4& operator*=(simdvec4 &a, simdvec4 b) { return (a = a * b); }
istForceInline simdvec4& operator/=(simdvec4 &a, simdvec4 b) { return (a = a / b); }
istForceInline simdvec4i& operator+=(simdvec4i &a, simdvec4i b) { return (a = a + b); }
istForceInline simdvec4i& operator-=(simdvec4i &a, simdvec4i b) { return (a = a - b); }
istForceInline simdvec4i& operator*=(simdvec4i &a, simdvec4i b) { return (a = a * b); }

istForceInline simdvec8 operator+(simdvec8 a, simdvec8 b) { return _mm256_add_ps(a, b); }
istForceInline simdvec8 operator-(simdvec8 a, simdvec8 b) { return _mm256_sub_ps(a, b); }
istForceInline simdvec8 operator*(simdvec8 a, simdvec8 b) { return _mm256_mul_ps(a, b); }
istForceInline simdvec8 operator/(simdvec8 a, simdvec8 b) { return _mm256_div_ps(a, b); }
istForceInline simdvec8& operator+=(simdvec8 &a, simdvec8 b) { return (a = a + b); }
istForceInline simdvec8& operator-=(simdvec8 &a, simdvec8 b) { return (a = a - b); }
istForceInline simdvec8& operator*=(simdvec8 &a, simdvec8 b) { return (a = a * b); }
istForceInline simdvec8& operator/=(simdvec8 &a, simdvec8 b) { return (a = a / b); }

istForceInline simdvec4 simdvec4_set(float x, float y, float z, float w) { return _mm_set_ps(w, z, y, x); }
istForceInline simdvec4i simdvec4_seti(int x, int y, int z, int w) { return _mm_set_epi32(w, z, y, x); }

istForceInline simdvec4 reduce_add4(simdvec4 v1)
{
    simdvec4 t1 = _mm_shuffle_ps(v1,v1,SSE_SHUFFLE(1,0,3,0));
    t1 = _mm_add_ps(v1,t1);
    v1 = _mm_shuffle_ps(t1,t1,SSE_SHUFFLE(2,0,0,0));
    t1 = _mm_add_ps(v1,t1);
    return _mm_shuffle_ps(t1,t1,SSE_SHUFFLE(0,0,0,0));
}

istForceInline simdvec4 rsqrt4(simdvec4 v1)
{
    return _mm_rsqrt_ps(v1);
}
istForceInline simdvec4 sqrt4(simdvec4 v1)
{
    return _mm_sqrt_ps(v1);
}

istForceInline simdvec4 dot4(simdvec4 v1, simdvec4 v2)
{
    return XMVector4Dot(v1, v2);
}

istForceInline simdvec4 length_sq4(simdvec4 v1)
{
    return XMVector4LengthSq(v1);
}

istForceInline simdvec4 length4(simdvec4 v1)
{
    return XMVector4LengthEst(v1);
}


template<class simdvec> struct simdvec_traits;

template<>
struct simdvec_traits<simdvec4>
{
    enum{ simd_elements = 4 };
};
template<>
struct simdvec_traits<simdvec8>
{
    enum{ simd_elements = 8 };
};


template<class soavec>
struct soavec_traits
{
    enum{ soa_elements  = soavec::soa_elements };
    enum{ simd_elements = soavec::simd_elements };
};



template<class simdvec>
struct tsoavec2x
{
    typedef simdvec simdvec_t;
    enum{
        soa_elements  = 2,
        simd_elements = simdvec_traits<simdvec_t>::simd_elements,
    };

    simdvec sv[2];

    istForceInline tsoavec2x() {}
    istForceInline tsoavec2x(const simdvec *_v) { sv[0]=_sv[0]; sv[1]=_sv[1]; }
    istForceInline tsoavec2x(const simdvec &_x, const simdvec &_y) { sv[0]=_x; sv[1]=_y; }
    istForceInline simdvec x() const { return sv[0]; }
    istForceInline simdvec y() const { return sv[1]; }
    istForceInline void x(const simdvec &_v) { sv[0]=_v; }
    istForceInline void y(const simdvec &_v) { sv[1]=_v; }
    istForceInline simdvec& operator[](int i) { return sv[i]; }
    istForceInline const simdvec& operator[](int i) const { return sv[i]; }
};

template<class simdvec>
struct tsoavec3x
{
    typedef simdvec simdvec_t;
    enum{
        soa_elements  = 3,
        simd_elements = simdvec_traits<simdvec_t>::simd_elements,
    };

    simdvec sv[3];

    istForceInline tsoavec3x() {}
    istForceInline tsoavec3x(const simdvec *_v) { sv[0]=_sv[0]; sv[1]=_sv[1]; sv[2]=_sv[2]; }
    istForceInline tsoavec3x(const simdvec &_x, const simdvec &_y, const simdvec &_z) { sv[0]=_x; sv[1]=_y; sv[2]=_z; }
    istForceInline simdvec x() const { return sv[0]; }
    istForceInline simdvec y() const { return sv[1]; }
    istForceInline simdvec z() const { return sv[2]; }
    istForceInline void x(const simdvec &_v) { sv[0]=_v; }
    istForceInline void y(const simdvec &_v) { sv[1]=_v; }
    istForceInline void z(const simdvec &_v) { sv[2]=_v; }
    istForceInline simdvec& operator[](int i) { return sv[i]; }
    istForceInline const simdvec& operator[](int i) const { return sv[i]; }
};

template<class simdvec>
struct tsoavec4x
{
    typedef simdvec simdvec_t;
    typedef tsoavec3x<simdvec_t> soavec3x;
    enum{
        soa_elements  = 4,
        simd_elements = simdvec_traits<simdvec_t>::simd_elements,
    };

    simdvec sv[4];

    istForceInline tsoavec4x() {}
    istForceInline tsoavec4x(const simdvec *_v) { sv[0]=_sv[0]; sv[1]=_sv[1]; sv[2]=_sv[2]; sv[3]=_sv[3]; }
    istForceInline tsoavec4x(const simdvec &_x, const simdvec &_y, const simdvec &_z, const simdvec &_w) { sv[0]=_x; sv[1]=_y; sv[2]=_z; sv[3]=_w; }
    istForceInline tsoavec4x(const soavec3x &_xyz, const simdvec &_w) { sv[0]=_xyz.sv[0]; sv[1]=_xyz.sv[1]; sv[2]=_xyz.sv[2]; sv[3]=_w; }
    istForceInline simdvec x() const { return sv[0]; }
    istForceInline simdvec y() const { return sv[1]; }
    istForceInline simdvec z() const { return sv[2]; }
    istForceInline simdvec w() const { return sv[3]; }
    istForceInline soavec3x xyz() const { return soavec3x(sv[0], sv[1], sv[2]); }
    istForceInline void x(const simdvec &_v) { sv[0]=_v; }
    istForceInline void y(const simdvec &_v) { sv[1]=_v; }
    istForceInline void z(const simdvec &_v) { sv[2]=_v; }
    istForceInline void w(const simdvec &_v) { sv[3]=_v; }
    istForceInline void xyz(const soavec3x &_v) { sv[0]=_v.sv[0]; sv[1]=_v.sv[1]; sv[2]=_v.sv[2]; }
    istForceInline simdvec& operator[](int i) { return sv[i]; }
    istForceInline const simdvec& operator[](int i) const { return sv[i]; }
};

typedef __declspec(align(16)) tsoavec2x<simdvec4> soavec24;
typedef __declspec(align(16)) tsoavec3x<simdvec4> soavec34;
typedef __declspec(align(16)) tsoavec4x<simdvec4> soavec44;
typedef __declspec(align(32)) tsoavec2x<simdvec8> soavec28;
typedef __declspec(align(32)) tsoavec3x<simdvec8> soavec38;
typedef __declspec(align(32)) tsoavec4x<simdvec8> soavec48;


namespace detail {

    template<size_t S> struct soa_types;

    template<>
    struct soa_types<4>
    {
        typedef simdvec4 simdvec;
        typedef soavec24 soavec2x;
        typedef soavec34 soavec3x;
        typedef soavec44 soavec4x;
    };

    template<>
    struct soa_types<8>
    {
        typedef simdvec8 simdvec;
        typedef soavec28 soavec2x;
        typedef soavec38 soavec3x;
        typedef soavec48 soavec4x;
    };

} // namespace detail




///////////////////////////////////////////////////////////////
//  AoS <-> SoA
///////////////////////////////////////////////////////////////

// 2 要素 SoA ベクトルへ

// in:  {x,y,z,w}[1]
// out: 
//      x[0], 0.0f, 0.0f, 0.0f
//      y[0], 0.0f, 0.0f, 0.0f
inline soavec24 soa_transpose24(const simdvec4 &v0)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, zero);
    __m128 r2 = zero;
    return soavec24(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)) );
}

// in:  {x,y,z,w}[2]
// out: 
//      x[0], x[1], 0.0f, 0.0f
//      y[0], y[1], 0.0f, 0.0f
inline soavec24 soa_transpose24(const simdvec4 &v0, const simdvec4 &v1)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = zero;
    return soavec24(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)) );
}

// in:  {x,y,z,w}[3]
// out: 
//      x[0], x[1], x[2], 0.0f
//      y[0], y[1], y[2], 0.0f
inline soavec24 soa_transpose24(const simdvec4 &v0, const simdvec4 &v1, const simdvec4 &v2)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = _mm_unpacklo_ps(v2, zero);
    return soavec24(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)) );
}

// in:  {x,y,z,w}[4]
// out: 
//      x[0], x[1], x[2], x[3]
//      y[0], y[1], y[2], y[3]
inline soavec24 soa_transpose24(const simdvec4 &v0, const simdvec4 &v1, const simdvec4 &v2, const simdvec4 &v3)
{
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = _mm_unpacklo_ps(v2, v3);
    return soavec24(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)) );
}


// 3 要素 SoA ベクトルへ

// in:  {x,y,z,w}[1]
// out: 
//      x[0], 0.0f, 0.0f, 0.0f
//      y[0], 0.0f, 0.0f, 0.0f
//      z[0], 0.0f, 0.0f, 0.0f
inline soavec34 soa_transpose34(const simdvec4 &v0)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, zero);
    __m128 r2 = zero;
    __m128 r3 = _mm_unpackhi_ps(v0, zero);
    __m128 r4 = zero;
    return soavec34(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(0,1,0,1)) );
}

// in:  {x,y,z,w}[2]
// out: 
//      x[0], x[1], 0.0f, 0.0f
//      y[0], y[1], 0.0f, 0.0f
//      z[0], z[1], 0.0f, 0.0f
inline soavec34 soa_transpose34(const simdvec4 &v0, const simdvec4 &v1)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = zero;
    __m128 r3 = _mm_unpackhi_ps(v0, v1);
    __m128 r4 = zero;
    return soavec34(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(0,1,0,1)) );
}

// in:  {x,y,z,w}[3]
// out: 
//      x[0], x[1], x[2], 0.0f
//      y[0], y[1], y[2], 0.0f
//      z[0], z[1], z[2], 0.0f
inline soavec34 soa_transpose34(const simdvec4 &v0, const simdvec4 &v1, const simdvec4 &v2)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = _mm_unpacklo_ps(v2, zero);
    __m128 r3 = _mm_unpackhi_ps(v0, v1);
    __m128 r4 = _mm_unpackhi_ps(v2, zero);
    return soavec34(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(0,1,0,1)) );
}

// in:  {x,y,z,w}[4]
// out: 
//      x[0], x[1], x[2], x[3]
//      y[0], y[1], y[2], y[3]
//      z[0], z[1], z[2], z[3]
inline soavec34 soa_transpose34(const simdvec4 &v0, const simdvec4 &v1, const simdvec4 &v2, const simdvec4 &v3)
{
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = _mm_unpacklo_ps(v2, v3);
    __m128 r3 = _mm_unpackhi_ps(v0, v1);
    __m128 r4 = _mm_unpackhi_ps(v2, v3);
    return soavec34(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(0,1,0,1)) );
}


// 4 要素 SoA ベクトルへ

// in:  {x,y,z,w}[1]
// out: 
//      x[0], 0.0f, 0.0f, 0.0f
//      y[0], 0.0f, 0.0f, 0.0f
//      z[0], 0.0f, 0.0f, 0.0f
//      w[0], 0.0f, 0.0f, 0.0f
inline soavec44 soa_transpose44(const simdvec4 &v0)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, zero);
    __m128 r2 = zero;
    __m128 r3 = _mm_unpackhi_ps(v0, zero);
    __m128 r4 = zero;
    return soavec44(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(2,3,2,3)) );
}

// in:  {x,y,z,w}[2]
// out: 
//      x[0], x[1], 0.0f, 0.0f
//      y[0], y[1], 0.0f, 0.0f
//      z[0], z[1], 0.0f, 0.0f
//      w[0], w[1], 0.0f, 0.0f
inline soavec44 soa_transpose44(const simdvec4 &v0, const simdvec4 &v1)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = zero;
    __m128 r3 = _mm_unpackhi_ps(v0, v1);
    __m128 r4 = zero;
    return soavec44(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(2,3,2,3)) );
}

// in:  {x,y,z,w}[3]
// out: 
//      x[0], x[1], x[2], 0.0f
//      y[0], y[1], y[2], 0.0f
//      z[0], z[1], z[2], 0.0f
//      w[0], w[1], w[2], 0.0f
inline soavec44 soa_transpose44(const simdvec4 &v0, const simdvec4 &v1, const simdvec4 &v2)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = _mm_unpacklo_ps(v2, zero);
    __m128 r3 = _mm_unpackhi_ps(v0, v1);
    __m128 r4 = _mm_unpackhi_ps(v2, zero);
    return soavec44(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(2,3,2,3)) );
}

// in:  {x,y,z,w}[4]
// out: 
//      x[0], x[1], x[2], x[3]
//      y[0], y[1], y[2], y[3]
//      z[0], z[1], z[2], z[3]
//      w[0], w[1], w[2], w[3]
inline soavec44 soa_transpose44(const simdvec4 &v0, const simdvec4 &v1, const simdvec4 &v2, const simdvec4 &v3)
{
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = _mm_unpacklo_ps(v2, v3);
    __m128 r3 = _mm_unpackhi_ps(v0, v1);
    __m128 r4 = _mm_unpackhi_ps(v2, v3);
    return soavec44(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(2,3,2,3)) );
}



istForceInline soavec24 operator+(const soavec24 &a, const soavec24 &b) { return soavec24(a[0]+b[0], a[1]+b[1]); }
istForceInline soavec34 operator+(const soavec34 &a, const soavec34 &b) { return soavec34(a[0]+b[0], a[1]+b[1], a[2]+b[2]); }
istForceInline soavec44 operator+(const soavec44 &a, const soavec44 &b) { return soavec44(a[0]+b[0], a[1]+b[1], a[2]+b[2], a[3]+b[3]); }

istForceInline soavec24 operator-(const soavec24 &a, const soavec24 &b) { return soavec24(a[0]-b[0], a[1]-b[1]); }
istForceInline soavec34 operator-(const soavec34 &a, const soavec34 &b) { return soavec34(a[0]-b[0], a[1]-b[1], a[2]-b[2]); }
istForceInline soavec44 operator-(const soavec44 &a, const soavec44 &b) { return soavec44(a[0]-b[0], a[1]-b[1], a[2]-b[2], a[3]-b[3]); }

istForceInline soavec24 operator*(const soavec24 &a, const soavec24 &b) { return soavec24(a[0]*b[0], a[1]*b[1]); }
istForceInline soavec34 operator*(const soavec34 &a, const soavec34 &b) { return soavec34(a[0]*b[0], a[1]*b[1], a[2]*b[2]); }
istForceInline soavec44 operator*(const soavec44 &a, const soavec44 &b) { return soavec44(a[0]*b[0], a[1]*b[1], a[2]*b[2], a[3]*b[3]); }
istForceInline soavec24 operator*(const soavec24 &a, const simdvec4 &b) { return soavec24(a[0]*b, a[1]*b); }
istForceInline soavec34 operator*(const soavec34 &a, const simdvec4 &b) { return soavec34(a[0]*b, a[1]*b, a[2]*b); }
istForceInline soavec44 operator*(const soavec44 &a, const simdvec4 &b) { return soavec44(a[0]*b, a[1]*b, a[2]*b, a[3]*b); }

istForceInline soavec24 operator/(const soavec24 &a, const soavec24 &b) { return soavec24(a[0]/b[0], a[1]/b[1]); }
istForceInline soavec34 operator/(const soavec34 &a, const soavec34 &b) { return soavec34(a[0]/b[0], a[1]/b[1], a[2]/b[2]); }
istForceInline soavec44 operator/(const soavec44 &a, const soavec44 &b) { return soavec44(a[0]/b[0], a[1]/b[1], a[2]/b[2], a[3]/b[3]); }
istForceInline soavec24 operator/(const soavec24 &a, const simdvec4 &b) { return soavec24(a[0]/b, a[1]/b); }
istForceInline soavec34 operator/(const soavec34 &a, const simdvec4 &b) { return soavec34(a[0]/b, a[1]/b, a[2]/b); }
istForceInline soavec44 operator/(const soavec44 &a, const simdvec4 &b) { return soavec44(a[0]/b, a[1]/b, a[2]/b, a[3]/b); }

istForceInline simdvec4 reduce_add2(const soavec24 &v) { return v[0] + v[1]; }
istForceInline simdvec4 reduce_add2(const soavec34 &v) { return v[0] + v[1]; }
istForceInline simdvec4 reduce_add2(const soavec44 &v) { return v[0] + v[1]; }
istForceInline simdvec4 reduce_add3(const soavec34 &v) { return v[0] + v[1] + v[2]; }
istForceInline simdvec4 reduce_add3(const soavec44 &v) { return v[0] + v[1] + v[2]; }
istForceInline simdvec4 reduce_add4(const soavec44 &v) { return v[0] + v[1] + v[2] + v[3]; }

istForceInline simdvec4 dot2(const soavec24 &v1, const soavec24 &v2) { return v1[0]*v2[0] + v1[1]*v2[1]; }
istForceInline simdvec4 dot2(const soavec34 &v1, const soavec34 &v2) { return v1[0]*v2[0] + v1[1]*v2[1]; }
istForceInline simdvec4 dot2(const soavec44 &v1, const soavec44 &v2) { return v1[0]*v2[0] + v1[1]*v2[1]; }
istForceInline simdvec4 dot3(const soavec34 &v1, const soavec34 &v2) { return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]; }
istForceInline simdvec4 dot3(const soavec44 &v1, const soavec44 &v2) { return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]; }
istForceInline simdvec4 dot4(const soavec44 &v1, const soavec44 &v2) { return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2] + v1[3]*v2[3]; }

istForceInline simdvec4 length_sq2(const soavec24 &v1) { return dot2(v1, v1); }
istForceInline simdvec4 length_sq2(const soavec34 &v1) { return dot2(v1, v1); }
istForceInline simdvec4 length_sq2(const soavec44 &v1) { return dot2(v1, v1); }
istForceInline simdvec4 length_sq3(const soavec34 &v1) { return dot3(v1, v1); }
istForceInline simdvec4 length_sq3(const soavec44 &v1) { return dot3(v1, v1); }
istForceInline simdvec4 length_sq4(const soavec44 &v1) { return dot4(v1, v1); }

istForceInline simdvec4 length2(const soavec24 &v1) { return sqrt4(dot2(v1, v1)); }
istForceInline simdvec4 length2(const soavec34 &v1) { return sqrt4(dot2(v1, v1)); }
istForceInline simdvec4 length2(const soavec44 &v1) { return sqrt4(dot2(v1, v1)); }
istForceInline simdvec4 length3(const soavec34 &v1) { return sqrt4(dot3(v1, v1)); }
istForceInline simdvec4 length3(const soavec44 &v1) { return sqrt4(dot3(v1, v1)); }
istForceInline simdvec4 length4(const soavec44 &v1) { return sqrt4(dot4(v1, v1)); }


} // namespace ist
