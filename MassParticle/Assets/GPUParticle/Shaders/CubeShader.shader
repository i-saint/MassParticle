Shader "GPUParticle/CubeShader" {

Properties {
    _BaseColor ("BaseColor", Vector) = (0.15, 0.15, 0.2, 5.0)
    _Scale ("Scale", Float) = 1.0
}
SubShader {
    Tags { "RenderType"="Opaque" }

CGINCLUDE
#include "UnityCG.cginc"
#include "MPGPFoundation.cginc"

struct Vertex
{
    float3 position;
    float3 normal;
};

float4 _BaseColor;
float _Scale;
int _FlipY;
StructuredBuffer<Particle> particles;
StructuredBuffer<Vertex> vertices;

struct ia_out {
    uint vertexID : SV_VertexID;
    uint instanceID : SV_InstanceID;
};

struct vs_out {
    float4 vertex : SV_POSITION;
    float4 color : TEXCOORD0;
};

struct ps_out
{
    float4 color : COLOR0;
};

vs_out vert(ia_out io)
{
    float3 ipos = particles[io.instanceID].position;
    float4 v = float4(vertices[io.vertexID].position*_Scale+ipos, 1.0);
    float4 n = float4(vertices[io.vertexID].normal, 0.0);
    float4 vp = mul(UNITY_MATRIX_VP, v);
    if(_FlipY) {
        vp.y *= -1.0;
    }

    float density = particles[io.instanceID].density;
    vs_out o;
    o.vertex = vp;
    o.color = _BaseColor;
    o.color.x += density * 0.0003;
    o.color.y += density * 0.0002;
    o.color.z += density * 0.0005;
    o.color.w = particles[io.vertexID].lifetime==0.0 ? 0.0 : 1.0;

    return o;
}

ps_out frag(vs_out vo)
{
    if(vo.color.w==0.0) { discard; }
    ps_out o;
    o.color = vo.color;
    return o;
}

ENDCG

    Pass {
        Cull Back
        ZWrite On
        ZTest LEqual

        CGPROGRAM
        #pragma target 5.0
        #pragma vertex vert
        #pragma fragment frag 
        ENDCG
    }
}
Fallback Off
}
