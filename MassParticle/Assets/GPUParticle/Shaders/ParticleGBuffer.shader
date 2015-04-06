Shader "GPUParticle/ParticleGBuffer" {

Properties {
    _BaseColor ("BaseColor", Vector) = (0.15, 0.15, 0.2, 5.0)
    _GlowColor ("GlowColor", Vector) = (0.0, 0.0, 0.0, 0.0)
    _HeatColor ("HeatColor", Vector) = (0.25, 0.05, 0.025, 0.0)
    _HeatThreshold ("HeatThreshold", Float) = 2.5
    _HeatIntensity ("HeatIntensity", Float) = 1.0
    _Scale ("Scale", Float) = 1.0
    _FadeTime ("FadeTime", Float) = 0.1
    _Spin ("Spin", Float) = 1.0
}
SubShader {
    Tags { "RenderType"="Opaque" }

CGINCLUDE
#include "UnityCG.cginc"
#include "MPGPFoundation.cginc"

float4 _BaseColor;
float4 _GlowColor;
float4 _HeatColor;
float _HeatThreshold;
float _HeatIntensity;
float _Scale;
float _FadeTime;
float _Spin;

struct Vertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 texcoord;
};
StructuredBuffer<Particle> particles;
StructuredBuffer<Vertex> vertices;

struct ia_out {
    uint vertexID : SV_VertexID;
    uint instanceID : SV_InstanceID;
};

struct vs_out {
    float4 vertex : SV_POSITION;
    float4 screen_pos : TEXCOORD0;
    float4 position : TEXCOORD1;
    float4 normal : TEXCOORD2;
    float4 emission : TEXCOORD3;
};

struct ps_out
{
    float4 normal : COLOR0;
    float4 position : COLOR1;
    float4 color : COLOR2;
    float4 glow : COLOR3;
};


float4x4 rotation_matrix(float3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return float4x4(
        oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
        oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
        oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
        0.0,                                0.0,                                0.0,                                1.0);
}

vs_out vert(ia_out io)
{
    float pid = (float)particles[io.instanceID].id;
    float lifetime = particles[io.instanceID].lifetime;
    float scale = _Scale * min(lifetime/_FadeTime, 1.0);
    float speed = particles[io.instanceID].speed;

    float3 position = particles[io.instanceID].position;
    float4 v = float4(vertices[io.vertexID].position, 1.0);
    float4 n = float4(vertices[io.vertexID].normal, 0.0);
    if(lifetime<=0.0) { v=0.0; }
    if(_Spin != 0.0) {
        float ang = _Spin * lifetime;
        float4x4 rot = rotation_matrix(normalize(iq_rand(float3(pid, pid*123.45, pid*543.21))), ang);
        v = mul(rot, v);
        n = mul(rot, n);
    }
    v.xyz *= scale;
    v.xyz += position;
    float4 vp = mul(UNITY_MATRIX_MVP, v);

    vs_out o;
    o.vertex = vp;
    o.screen_pos = vp;
    o.position = v;
    o.normal.xyz = normalize(n.xyz);
    o.normal.w = 1.0;

    float heat = max(speed-_HeatThreshold, 0.0) * _HeatIntensity;
    o.emission = _GlowColor + _HeatColor * heat;
    o.emission.w = lifetime;
    return o;
}

ps_out frag(vs_out vo)
{
    ps_out o;
    o.normal = float4(vo.normal.xyz, 1.0);
    o.position = float4(vo.position.xyz, vo.screen_pos.z);
    o.color = _BaseColor;
    o.glow = vo.emission;
    return o;
}
ENDCG

    Pass {
        Name "GBuffer"
        Cull Back
        ZWrite On
        ZTest LEqual

        CGPROGRAM
        #pragma target 5.0
        #pragma vertex vert
        #pragma fragment frag 
        ENDCG
    }
    Pass {
        Name "DepthPrePass"
        Cull Back
        ZWrite On
        ZTest Less
        ColorMask 0

        CGPROGRAM
        #pragma vertex vert
        #pragma fragment frag
        #pragma target 5.0
        #ifdef SHADER_API_OPENGL 
            #pragma glsl
        #endif
        ENDCG
    }
    Pass {
        Name "GBuffer"
        Cull Back
        ZWrite Off
        ZTest Equal

        CGPROGRAM
        #pragma vertex vert
        #pragma fragment frag
        #pragma target 5.0
        #ifdef SHADER_API_OPENGL 
            #pragma glsl
        #endif
        ENDCG
    }
}
Fallback Off

}
