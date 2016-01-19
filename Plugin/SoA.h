#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>

#define SSE_SHUFFLE(x,y,z,w) _MM_SHUFFLE(w,z,y,x)
#ifdef _MSC_VER
#   define istForceInline __forceinline
#else  // _MSC_VER
#   define istForceInline inline
#endif // _MSC_VER

//#define __ist_enable_soavec8__



namespace ist {

typedef __m128  simdvec4;
typedef __m128i simdvec4i;
//typedef __m256  simdvec8;
//typedef __m256i simdvec8i;


istForceInline simdvec4 add(simdvec4 a, simdvec4 b) { return _mm_add_ps(a, b); }
istForceInline simdvec4 sub(simdvec4 a, simdvec4 b) { return _mm_sub_ps(a, b); }
istForceInline simdvec4 mul(simdvec4 a, simdvec4 b) { return _mm_mul_ps(a, b); }
istForceInline simdvec4 div(simdvec4 a, simdvec4 b) { return _mm_div_ps(a, b); }

//istForceInline simdvec4i addeq(simdvec4i a, simdvec4i b) { return _mm_add_epi32(a, b); }
//istForceInline simdvec4i subeq(simdvec4i a, simdvec4i b) { return _mm_sub_epi32(a, b); }
//istForceInline simdvec4i muleq(simdvec4i a, simdvec4i b) { return _mm_mul_epi32(a, b); }

istForceInline simdvec4& addeq(simdvec4 &a, simdvec4 b) { return (a = add(a,b)); }
istForceInline simdvec4& subeq(simdvec4 &a, simdvec4 b) { return (a = sub(a,b)); }
istForceInline simdvec4& muleq(simdvec4 &a, simdvec4 b) { return (a = mul(a,b)); }
istForceInline simdvec4& diveq(simdvec4 &a, simdvec4 b) { return (a = div(a,b)); }
//istForceInline simdvec4i& addeq(simdvec4i &a, simdvec4i b) { return (a = add(a,b)); }
//istForceInline simdvec4i& subeq(simdvec4i &a, simdvec4i b) { return (a = sub(a,b)); }
//istForceInline simdvec4i& muleq(simdvec4i &a, simdvec4i b) { return (a = mul(a,b)); }

//istForceInline simdvec8 addeq(simdvec8 a, simdvec8 b) { return _mm256_add_ps(a, b); }
//istForceInline simdvec8 subeq(simdvec8 a, simdvec8 b) { return _mm256_sub_ps(a, b); }
//istForceInline simdvec8 muleq(simdvec8 a, simdvec8 b) { return _mm256_mul_ps(a, b); }
//istForceInline simdvec8 diveq(simdvec8 a, simdvec8 b) { return _mm256_div_ps(a, b); }
//istForceInline simdvec8& addeq(simdvec8 &a, simdvec8 b) { return (a = a + b); }
//istForceInline simdvec8& subeq(simdvec8 &a, simdvec8 b) { return (a = a - b); }
//istForceInline simdvec8& muleq(simdvec8 &a, simdvec8 b) { return (a = a * b); }
//istForceInline simdvec8& diveq(simdvec8 &a, simdvec8 b) { return (a = a / b); }

istForceInline simdvec4 simdvec4_set(float v) { return _mm_set1_ps(v); }
istForceInline simdvec4 simdvec4_set(float x, float y, float z, float w) { return _mm_set_ps(w, z, y, x); }
istForceInline simdvec4i simdvec4_seti(int v) { return _mm_set1_epi32(v); }
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
    simdvec4 t2 = v2;
    simdvec4 t1 = _mm_mul_ps(v1, t2);
    t2 = _mm_shuffle_ps(t2, t1, _MM_SHUFFLE(1, 0, 0, 0));
    t2 = _mm_add_ps(t2, t1);
    t1 = _mm_shuffle_ps(t1, t2, _MM_SHUFFLE(0, 3, 0, 0));
    t1 = _mm_add_ps(t1, t2);
    return _mm_shuffle_ps(t1, t1, _MM_SHUFFLE(2, 2, 2, 2));
}

istForceInline simdvec4 length_sq4(simdvec4 v1)
{
    return dot4(v1, v1);
}

istForceInline simdvec4 length4(simdvec4 v1)
{
    return _mm_sqrt_ps(dot4(v1, v1));
}


template<class simdvec> struct simdvec_traits;

template<>
struct simdvec_traits<simdvec4>
{
    enum{ simd_elements = 4 };
};
//template<>
//struct simdvec_traits<simdvec8>
//{
//	enum{ simd_elements = 8 };
//};


template<class soavec>
struct soavec_traits
{
    enum{ soa_elements  = soavec::soa_elements };
    enum{ simd_elements = soavec::simd_elements };
};



template<class simdvec>
struct tvecsoa2
{
    typedef simdvec simdvec_t;
    enum{
        soa_elements  = 2,
        simd_elements = simdvec_traits<simdvec_t>::simd_elements,
    };

    simdvec sv[2];

    istForceInline tvecsoa2() {}
    istForceInline tvecsoa2(const simdvec *_v) { sv[0]=_v[0]; sv[1]=_v[1]; }
    istForceInline tvecsoa2(const simdvec &_x, const simdvec &_y) { sv[0]=_x; sv[1]=_y; }
    istForceInline simdvec x() const { return sv[0]; }
    istForceInline simdvec y() const { return sv[1]; }
    istForceInline void x(const simdvec &_v) { sv[0]=_v; }
    istForceInline void y(const simdvec &_v) { sv[1]=_v; }
    istForceInline simdvec& operator[](int i) { return sv[i]; }
    istForceInline const simdvec& operator[](int i) const { return sv[i]; }
};

template<class simdvec>
struct tvecsoa3
{
    typedef simdvec simdvec_t;
    enum{
        soa_elements  = 3,
        simd_elements = simdvec_traits<simdvec_t>::simd_elements,
    };

    simdvec sv[3];

    istForceInline tvecsoa3() {}
    istForceInline tvecsoa3(const simdvec *_v) { sv[0]=_v[0]; sv[1]=_v[1]; sv[2]=_v[2]; }
    istForceInline tvecsoa3(const simdvec &_x, const simdvec &_y, const simdvec &_z) { sv[0]=_x; sv[1]=_y; sv[2]=_z; }
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
struct tvecsoa4
{
    typedef simdvec simdvec_t;
    typedef tvecsoa3<simdvec_t> tvecsoa3;
    enum{
        soa_elements  = 4,
        simd_elements = simdvec_traits<simdvec_t>::simd_elements,
    };

    simdvec sv[4];

    istForceInline tvecsoa4() {}
    istForceInline tvecsoa4(const simdvec *_v) { sv[0]=_v[0]; sv[1]=_v[1]; sv[2]=_v[2]; sv[3]=_v[3]; }
    istForceInline tvecsoa4(const simdvec &_x, const simdvec &_y, const simdvec &_z, const simdvec &_w) { sv[0]=_x; sv[1]=_y; sv[2]=_z; sv[3]=_w; }
    istForceInline tvecsoa4(const tvecsoa3 &_xyz, const simdvec &_w) { sv[0]=_xyz.sv[0]; sv[1]=_xyz.sv[1]; sv[2]=_xyz.sv[2]; sv[3]=_w; }
    istForceInline simdvec x() const { return sv[0]; }
    istForceInline simdvec y() const { return sv[1]; }
    istForceInline simdvec z() const { return sv[2]; }
    istForceInline simdvec w() const { return sv[3]; }
    istForceInline tvecsoa3 xyz() const { return tvecsoa3(sv[0], sv[1], sv[2]); }
    istForceInline void x(const simdvec &_v) { sv[0]=_v; }
    istForceInline void y(const simdvec &_v) { sv[1]=_v; }
    istForceInline void z(const simdvec &_v) { sv[2]=_v; }
    istForceInline void w(const simdvec &_v) { sv[3]=_v; }
    istForceInline void xyz(const tvecsoa3 &_v) { sv[0]=_v.sv[0]; sv[1]=_v.sv[1]; sv[2]=_v.sv[2]; }
    istForceInline simdvec& operator[](int i) { return sv[i]; }
    istForceInline const simdvec& operator[](int i) const { return sv[i]; }
};

typedef __declspec(align(16)) tvecsoa2<simdvec4> vec4soa2;
typedef __declspec(align(16)) tvecsoa3<simdvec4> vec4soa3;
typedef __declspec(align(16)) tvecsoa4<simdvec4> vec4soa4;
//typedef __declspec(align(32)) tvecsoa2<simdvec8> vec8soa2;
//typedef __declspec(align(32)) tvecsoa3<simdvec8> vec8soa3;
//typedef __declspec(align(32)) tvecsoa4<simdvec8> vec8soa4;


namespace detail {

    template<size_t S> struct soa_types;

    template<>
    struct soa_types<4>
    {
        typedef simdvec4 simdvec;
        typedef vec4soa2 vecxsoa2;
        typedef vec4soa3 vecxsoa3;
        typedef vec4soa4 vecxsoa4;
    };

    //template<>
    //struct soa_types<8>
    //{
    //	typedef simdvec8 simdvec;
    //	typedef vec8soa2 vecxsoa2;
    //	typedef vec8soa3 vecxsoa3;
    //	typedef vec8soa4 vecxsoa4;
    //};

} // namespace detail




///////////////////////////////////////////////////////////////
//  AoS <-> SoA
///////////////////////////////////////////////////////////////

// 2 要素 SoA ベクトルへ

// in:  {x,y,z,w}[1]
// out: 
//      x[0], 0.0f, 0.0f, 0.0f
//      y[0], 0.0f, 0.0f, 0.0f
inline vec4soa2 soa_transpose24(const simdvec4 &v0)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, zero);
    __m128 r2 = zero;
    return vec4soa2(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)) );
}

// in:  {x,y,z,w}[2]
// out: 
//      x[0], x[1], 0.0f, 0.0f
//      y[0], y[1], 0.0f, 0.0f
inline vec4soa2 soa_transpose24(const simdvec4 &v0, const simdvec4 &v1)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = zero;
    return vec4soa2(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)) );
}

// in:  {x,y,z,w}[3]
// out: 
//      x[0], x[1], x[2], 0.0f
//      y[0], y[1], y[2], 0.0f
inline vec4soa2 soa_transpose24(const simdvec4 &v0, const simdvec4 &v1, const simdvec4 &v2)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = _mm_unpacklo_ps(v2, zero);
    return vec4soa2(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)) );
}

// in:  {x,y,z,w}[4]
// out: 
//      x[0], x[1], x[2], x[3]
//      y[0], y[1], y[2], y[3]
inline vec4soa2 soa_transpose24(const simdvec4 &v0, const simdvec4 &v1, const simdvec4 &v2, const simdvec4 &v3)
{
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = _mm_unpacklo_ps(v2, v3);
    return vec4soa2(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)) );
}


// 3 要素 SoA ベクトルへ

// in:  {x,y,z,w}[1]
// out: 
//      x[0], 0.0f, 0.0f, 0.0f
//      y[0], 0.0f, 0.0f, 0.0f
//      z[0], 0.0f, 0.0f, 0.0f
inline vec4soa3 soa_transpose34(const simdvec4 &v0)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, zero);
    __m128 r2 = zero;
    __m128 r3 = _mm_unpackhi_ps(v0, zero);
    __m128 r4 = zero;
    return vec4soa3(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(0,1,0,1)) );
}

// in:  {x,y,z,w}[2]
// out: 
//      x[0], x[1], 0.0f, 0.0f
//      y[0], y[1], 0.0f, 0.0f
//      z[0], z[1], 0.0f, 0.0f
inline vec4soa3 soa_transpose34(const simdvec4 &v0, const simdvec4 &v1)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = zero;
    __m128 r3 = _mm_unpackhi_ps(v0, v1);
    __m128 r4 = zero;
    return vec4soa3(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(0,1,0,1)) );
}

// in:  {x,y,z,w}[3]
// out: 
//      x[0], x[1], x[2], 0.0f
//      y[0], y[1], y[2], 0.0f
//      z[0], z[1], z[2], 0.0f
inline vec4soa3 soa_transpose34(const simdvec4 &v0, const simdvec4 &v1, const simdvec4 &v2)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = _mm_unpacklo_ps(v2, zero);
    __m128 r3 = _mm_unpackhi_ps(v0, v1);
    __m128 r4 = _mm_unpackhi_ps(v2, zero);
    return vec4soa3(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(0,1,0,1)) );
}

// in:  {x,y,z,w}[4]
// out: 
//      x[0], x[1], x[2], x[3]
//      y[0], y[1], y[2], y[3]
//      z[0], z[1], z[2], z[3]
inline vec4soa3 soa_transpose34(const simdvec4 &v0, const simdvec4 &v1, const simdvec4 &v2, const simdvec4 &v3)
{
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = _mm_unpacklo_ps(v2, v3);
    __m128 r3 = _mm_unpackhi_ps(v0, v1);
    __m128 r4 = _mm_unpackhi_ps(v2, v3);
    return vec4soa3(
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
inline vec4soa4 soa_transpose44(const simdvec4 &v0)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, zero);
    __m128 r2 = zero;
    __m128 r3 = _mm_unpackhi_ps(v0, zero);
    __m128 r4 = zero;
    return vec4soa4(
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
inline vec4soa4 soa_transpose44(const simdvec4 &v0, const simdvec4 &v1)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = zero;
    __m128 r3 = _mm_unpackhi_ps(v0, v1);
    __m128 r4 = zero;
    return vec4soa4(
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
inline vec4soa4 soa_transpose44(const simdvec4 &v0, const simdvec4 &v1, const simdvec4 &v2)
{
    __m128 zero = _mm_set_ps1(0.0f);
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = _mm_unpacklo_ps(v2, zero);
    __m128 r3 = _mm_unpackhi_ps(v0, v1);
    __m128 r4 = _mm_unpackhi_ps(v2, zero);
    return vec4soa4(
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
inline vec4soa4 soa_transpose44(const simdvec4 &v0, const simdvec4 &v1, const simdvec4 &v2, const simdvec4 &v3)
{
    __m128 r1 = _mm_unpacklo_ps(v0, v1);
    __m128 r2 = _mm_unpacklo_ps(v2, v3);
    __m128 r3 = _mm_unpackhi_ps(v0, v1);
    __m128 r4 = _mm_unpackhi_ps(v2, v3);
    return vec4soa4(
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r1, r2, SSE_SHUFFLE(2,3,2,3)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(0,1,0,1)),
        _mm_shuffle_ps(r3, r4, SSE_SHUFFLE(2,3,2,3)) );
}



istForceInline vec4soa2 operator+(const vec4soa2 &a, const vec4soa2 &b)
    { return vec4soa2(add(a[0],b[0]), add(a[1],b[1])); }
istForceInline vec4soa3 operator+(const vec4soa3 &a, const vec4soa3 &b)
    { return vec4soa3(add(a[0],b[0]), add(a[1],b[1]), add(a[2],b[2])); }
istForceInline vec4soa4 operator+(const vec4soa4 &a, const vec4soa4 &b)
    { return vec4soa4(add(a[0],b[0]), add(a[1],b[1]), add(a[2],b[2]), add(a[3],b[3])); }

istForceInline vec4soa2 operator-(const vec4soa2 &a, const vec4soa2 &b)
    { return vec4soa2(sub(a[0],b[0]), sub(a[1],b[1])); }
istForceInline vec4soa3 operator-(const vec4soa3 &a, const vec4soa3 &b)
    { return vec4soa3(sub(a[0],b[0]), sub(a[1],b[1]), sub(a[2],b[2])); }
istForceInline vec4soa4 operator-(const vec4soa4 &a, const vec4soa4 &b)
    { return vec4soa4(sub(a[0],b[0]), sub(a[1],b[1]), sub(a[2],b[2]), sub(a[3],b[3])); }

istForceInline vec4soa2 operator*(const vec4soa2 &a, const vec4soa2 &b)
    { return vec4soa2(mul(a[0],b[0]), mul(a[1],b[1])); }
istForceInline vec4soa3 operator*(const vec4soa3 &a, const vec4soa3 &b)
    { return vec4soa3(mul(a[0],b[0]), mul(a[1],b[1]), mul(a[2],b[2])); }
istForceInline vec4soa4 operator*(const vec4soa4 &a, const vec4soa4 &b)
    { return vec4soa4(mul(a[0],b[0]), mul(a[1],b[1]), mul(a[2],b[2]), mul(a[3],b[3])); }
istForceInline vec4soa2 operator*(const vec4soa2 &a, const simdvec4 &b)
    { return vec4soa2(mul(a[0],b), mul(a[1],b)); }
istForceInline vec4soa3 operator*(const vec4soa3 &a, const simdvec4 &b)
    { return vec4soa3(mul(a[0],b), mul(a[1],b), mul(a[2],b)); }
istForceInline vec4soa4 operator*(const vec4soa4 &a, const simdvec4 &b)
    { return vec4soa4(mul(a[0],b), mul(a[1],b), mul(a[2],b), mul(a[3],b)); }

istForceInline vec4soa2 operator/(const vec4soa2 &a, const vec4soa2 &b)
    { return vec4soa2(div(a[0],b[0]), div(a[1],b[1])); }
istForceInline vec4soa3 operator/(const vec4soa3 &a, const vec4soa3 &b)
    { return vec4soa3(div(a[0],b[0]), div(a[1],b[1]), div(a[2],b[2])); }
istForceInline vec4soa4 operator/(const vec4soa4 &a, const vec4soa4 &b)
    { return vec4soa4(div(a[0],b[0]), div(a[1],b[1]), div(a[2],b[2]), div(a[3],b[3])); }
istForceInline vec4soa2 operator/(const vec4soa2 &a, const simdvec4 &b)
    { return vec4soa2(div(a[0],b), div(a[1],b)); }
istForceInline vec4soa3 operator/(const vec4soa3 &a, const simdvec4 &b)
    { return vec4soa3(div(a[0],b), div(a[1],b), div(a[2],b)); }
istForceInline vec4soa4 operator/(const vec4soa4 &a, const simdvec4 &b)
    { return vec4soa4(div(a[0],b), div(a[1],b), div(a[2],b), div(a[3],b)); }

istForceInline simdvec4 reduce_add2(const vec4soa2 &v) { return add(v[0],v[1]); }
istForceInline simdvec4 reduce_add2(const vec4soa3 &v) { return add(v[0],v[1]); }
istForceInline simdvec4 reduce_add2(const vec4soa4 &v) { return add(v[0],v[1]); }
istForceInline simdvec4 reduce_add3(const vec4soa3 &v) { return add(add(v[0],v[1]),v[2]); }
istForceInline simdvec4 reduce_add3(const vec4soa4 &v) { return add(add(v[0],v[1]),v[2]); }
istForceInline simdvec4 reduce_add4(const vec4soa4 &v) { return add(add(v[0],v[1]),add(v[2],v[3])); }

istForceInline simdvec4 dot2(const vec4soa2 &v1, const vec4soa2 &v2) { return add(mul(v1[0],v2[0]), mul(v1[1], v2[1])); }
istForceInline simdvec4 dot2(const vec4soa3 &v1, const vec4soa3 &v2) { return add(mul(v1[0],v2[0]), mul(v1[1], v2[1])); }
istForceInline simdvec4 dot2(const vec4soa4 &v1, const vec4soa4 &v2) { return add(mul(v1[0],v2[0]), mul(v1[1], v2[1])); }
istForceInline simdvec4 dot3(const vec4soa3 &v1, const vec4soa3 &v2) { return add(add(mul(v1[0],v2[0]), mul(v1[1], v2[1])), mul(v1[2], v2[2])); }
istForceInline simdvec4 dot3(const vec4soa4 &v1, const vec4soa4 &v2) { return add(add(mul(v1[0],v2[0]), mul(v1[1], v2[1])), mul(v1[2], v2[2])); }
istForceInline simdvec4 dot4(const vec4soa4 &v1, const vec4soa4 &v2) { return add(add(mul(v1[0],v2[0]), mul(v1[1], v2[1])), add(mul(v1[2], v2[2]), mul(v1[3], v2[3]))); }

istForceInline simdvec4 length_sq2(const vec4soa2 &v1) { return dot2(v1, v1); }
istForceInline simdvec4 length_sq2(const vec4soa3 &v1) { return dot2(v1, v1); }
istForceInline simdvec4 length_sq2(const vec4soa4 &v1) { return dot2(v1, v1); }
istForceInline simdvec4 length_sq3(const vec4soa3 &v1) { return dot3(v1, v1); }
istForceInline simdvec4 length_sq3(const vec4soa4 &v1) { return dot3(v1, v1); }
istForceInline simdvec4 length_sq4(const vec4soa4 &v1) { return dot4(v1, v1); }

istForceInline simdvec4 length2(const vec4soa2 &v1) { return sqrt4(dot2(v1, v1)); }
istForceInline simdvec4 length2(const vec4soa3 &v1) { return sqrt4(dot2(v1, v1)); }
istForceInline simdvec4 length2(const vec4soa4 &v1) { return sqrt4(dot2(v1, v1)); }
istForceInline simdvec4 length3(const vec4soa3 &v1) { return sqrt4(dot3(v1, v1)); }
istForceInline simdvec4 length3(const vec4soa4 &v1) { return sqrt4(dot3(v1, v1)); }
istForceInline simdvec4 length4(const vec4soa4 &v1) { return sqrt4(dot4(v1, v1)); }


} // namespace ist
