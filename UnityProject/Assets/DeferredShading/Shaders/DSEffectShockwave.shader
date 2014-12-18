Shader "DeferredShading/Shockwave" {

Properties {
    _BaseColor ("BaseColor", Vector) = (0.3, 0.3, 0.3, 10.0)
    _GlowColor ("GlowColor", Vector) = (0.0, 0.0, 0.0, 0.0)
}
SubShader {
    Tags { "RenderType"="Opaque" "Queue"="Geometry" }

CGINCLUDE
#include "Compat.cginc"

sampler2D frame_buffer;
float4 shockwave_params; 
float3 base_position;

struct ia_out
{
    float4 vertex : POSITION;
    float4 normal : NORMAL;
};

struct vs_out
{
    float4 vertex : SV_POSITION;
    float4 refpos : TEXCOORD0;
    float4 spos : TEXCOORD1;
    float4 position : TEXCOORD2;
    float4 normal : TEXCOORD3;
};

struct ps_out
{
    float4 color : COLOR0;
};

vs_out vert(ia_out v)
{
    vs_out o;

    float4 p = float4(v.vertex.xyz, 1.0);
    o.vertex = o.spos = mul(UNITY_MATRIX_MVP, p);
    o.position = mul(UNITY_MATRIX_VP, p);
    o.normal = normalize(mul(_Object2World, float4(v.normal.xyz,0.0)));

    float4 wp = mul(_Object2World, p);
    o.refpos = mul(UNITY_MATRIX_VP, wp+float4(v.normal.xyz*shockwave_params.x*shockwave_params.y, 0.0));
    return o;
}


ps_out frag(vs_out i)
{
    float2 coord1 = screen_to_texcoord(i.refpos);
    float2 coord2 = screen_to_texcoord(i.spos);
#if UNITY_UV_STARTS_AT_TOP
    coord1.y = 1.0 - coord1.y;
    coord2.y = 1.0 - coord2.y;
#endif

    ps_out o;
    float3 eyedir = normalize(i.position.xyz - _WorldSpaceCameraPos);
    float d = dot(eyedir, i.normal.xyz);
    d = d*d;
    o.color = tex2D(frame_buffer, lerp(coord2, coord1, d));
    o.color.a = 1.0;
    return o;
}
ENDCG

    Pass {
        Cull Back
        ZWrite Off
        ZTest LEqual

        CGPROGRAM
        #pragma vertex vert
        #pragma fragment frag
        #pragma target 3.0
        #ifdef SHADER_API_OPENGL 
            #pragma glsl
        #endif
        ENDCG
    }
}
Fallback Off
}
