Shader "DeferredShading/GBufferBeam" {

Properties {
    _BaseColor ("BaseColor", Vector) = (0.3, 0.3, 0.3, 10.0)
    _GlowColor ("GlowColor", Vector) = (0.0, 0.0, 0.0, 0.0)
}
SubShader {
    Tags { "RenderType"="Opaque" "Queue"="Geometry" }

CGINCLUDE
#include "Compat.cginc"
#include "DS.cginc"
#include "DSGBuffer.cginc"

float4 beam_direction; // xyz: direction w: length
float3 base_position;
    
vs_out vert_beam(ia_out v)
{
    vs_out o;

    float4 pos1 = mul(_Object2World, v.vertex);
    float4 pos2 = pos1;
    pos2.xyz += beam_direction.xyz * beam_direction.w;

    float3 vel = pos1.xyz - pos2.xyz;
    float3 vel_dir = normalize(vel);
    float4 pos = dot(-beam_direction.xyz, v.normal.xyz)>0.0 ? pos1 : pos2;

    float4 vmvp = mul(UNITY_MATRIX_VP, pos);
    o.vertex = vmvp;
    o.screen_pos = vmvp;
    o.position = pos;
    o.normal = normalize(mul(_Object2World, float4(v.normal.xyz,0.0)).xyz);
    return o;
}



float pattern(float3 p)
{
    float3 grid = 0.25;
    float3 b = grid*0.5;

    p = modc(p, 0.25);
    float3 d = abs(p) - b;
    return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0)) * 10.0;
}


ps_out frag_beam(vs_out i)
{
    ps_out o;
    o.normal = float4(i.normal.xyz, _Gloss);
    o.position = float4(i.position.xyz, i.screen_pos.z);
    o.color = float4(_BaseColor.rgb, 0.0);

    float3 camDir = normalize(i.position.xyz - _WorldSpaceCameraPos);
    float d = min(max(abs(dot(camDir, i.normal.xyz))*1.0, 0.0), 1.0);
    float p = lerp(abs(pattern(i.position.xyz-beam_direction.xyz*beam_direction.w)), 1.0, d);
    o.glow = _GlowColor * p;
    //o.glow = _GlowColor;
    return o;
}
ENDCG

    Pass {
        Name "DepthPrePass"
        ColorMask 0
        ZWrite On
        ZTest Less

        CGPROGRAM
        #pragma vertex vert_beam
        #pragma fragment frag_beam
        #pragma target 3.0
        #ifdef SHADER_API_OPENGL 
            #pragma glsl
        #endif
        ENDCG
    }

    Pass {
        Name "Shading"
        Cull Back
        ZWrite On
        ZTest LEqual

        CGPROGRAM
        #pragma vertex vert_beam
        #pragma fragment frag_beam
        #pragma target 3.0
        #ifdef SHADER_API_OPENGL 
            #pragma glsl
        #endif
        ENDCG
    }
}
Fallback Off
}
