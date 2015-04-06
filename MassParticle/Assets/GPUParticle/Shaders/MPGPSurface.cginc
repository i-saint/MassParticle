#ifndef MPGPSurface_h
#define MPGPSurface_h

#if (defined(SHADER_API_D3D11) || defined(SHADER_API_PSSL))
    #define MPGP_WITH_STRUCTURED_BUFFER
#endif

#ifdef MPGP_SHADOW_COLLECTOR
#   define SHADOW_COLLECTOR_PASS
#endif // MPGP_SHADOW_COLLECTOR

#include "UnityCG.cginc"
#include "MPGPFoundation.cginc"

#ifdef MPGP_WITH_STRUCTURED_BUFFER
StructuredBuffer<Particle> particles;
#endif // MPGP_WITH_STRUCTURED_BUFFER
int         g_batch_begin;
float       g_size;
float       g_fade_time;
float       g_spin;


int ParticleTransform(inout appdata_full v)
{
    int iid = v.texcoord1.x + g_batch_begin;
#ifdef MPGP_WITH_STRUCTURED_BUFFER
    Particle p = particles[iid];

    v.vertex.xyz *= g_size;
    v.vertex.xyz *= min(1.0, p.lifetime/g_fade_time);
    if(p.lifetime<=0.0) {
        v.vertex.xyz = 0.0;
    }
    if(g_spin != 0.0) {
        float ang = (dot(p.position.xyz, 1.0) * min(1.0, p.speed*0.02)) * g_spin;
        float3x3 rot = rotation_matrix33(normalize(iq_rand(p.id)), ang);
        v.vertex.xyz = mul(rot, v.vertex.xyz);
        v.normal.xyz = mul(rot, v.normal.xyz);
    }
    v.vertex.xyz += p.position.xyz;
#endif // MPGP_WITH_STRUCTURED_BUFFER
    return iid;
}



#if defined(MPGP_STANDARD) || defined(MPGP_SURFACE)
    sampler2D _MainTex;
    fixed4 _Color;
    fixed4 _Emission;

    struct Input {
        float2 uv_MainTex;
    #ifdef ENABLE_HEAT_EMISSION
        float4 velocity;
    #endif // ENABLE_HEAT_EMISSION
    };

    void vert(inout appdata_full v, out Input data)
    {
        UNITY_INITIALIZE_OUTPUT(Input,data);

        int iid = ParticleTransform(v);
    #ifdef MPGP_WITH_STRUCTURED_BUFFER
    #ifdef ENABLE_HEAT_EMISSION
        data.velocity = particles[iid].velocity;
    #endif // ENABLE_HEAT_EMISSION
    #endif // MPGP_WITH_STRUCTURED_BUFFER
    }
#endif // defined(MPGP_STANDARD) || defined(MPGP_SURFACE)



// legacy surface shader
#ifdef MPGP_SURFACE
    void surf(Input data, inout SurfaceOutput o)
    {
        o.Albedo = _Color * tex2D(_MainTex, data.uv_MainTex);
        o.Emission += _Emission;

    #ifdef ENABLE_HEAT_EMISSION
        float speed = data.velocity.w;
        float ei = max(speed-2.0, 0.0) * 1.0;
        o.Emission += float3(0.25, 0.05, 0.025)*ei;
    #endif // ENABLE_HEAT_EMISSION
    }
#endif // MPGP_SURFACE



// standard shader
#ifdef MPGP_STANDARD
    half _Glossiness;
    half _Metallic;

    void surf(Input IN, inout SurfaceOutputStandard o)
    {
        fixed4 c = tex2D(_MainTex, IN.uv_MainTex) * _Color;
        o.Albedo = c.rgb;
        o.Metallic = _Metallic;
        o.Smoothness = _Glossiness;
        o.Alpha = c.a;
        o.Emission += _Emission;

    #ifdef ENABLE_HEAT_EMISSION
        float speed = IN.velocity.w;
        float ei = max(speed-2.0, 0.0) * 1.0;
        o.Emission += float3(0.25, 0.05, 0.025)*ei;
    #endif // ENABLE_HEAT_EMISSION
    }
#endif // MPGP_STANDARD



// shadow caster
#ifdef MPGP_SHADOW_CASTER
    struct v2f
    { 
        V2F_SHADOW_CASTER;
    };

    v2f vert( appdata_full v )
    {
        int iid = ParticleTransform(v);

        v2f o;
        TRANSFER_SHADOW_CASTER(o)
        return o;
    }

    float4 frag( v2f i ) : SV_Target
    {
        SHADOW_CASTER_FRAGMENT(i)
    }
#endif // MPGP_SHADOW_CASTER



// legacy shadow collector
#ifdef MPGP_SHADOW_COLLECTOR
    struct v2f { 
        V2F_SHADOW_COLLECTOR;
    };

    v2f vert( appdata_full v )
    {
        int iid = ParticleTransform(v);

        v2f o;
        TRANSFER_SHADOW_COLLECTOR(o)
        return o;
    }
        
    fixed4 frag (v2f i) : SV_Target
    {
        SHADOW_COLLECTOR_FRAGMENT(i)
    }
#endif // MPGP_SHADOW_COLLECTOR



#endif // MPGPSurface_h
