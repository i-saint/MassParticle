#ifndef Point_h
#define Point_h

#include "UnityCG.cginc"
#include "MPGPFoundation.cginc"

float4 _BaseColor;
StructuredBuffer<Particle> particles;

struct ia_out {
    uint vertexID : SV_VertexID;
    uint instanceID : SV_InstanceID;
};

struct vs_out {
    float4 vertex : SV_POSITION;
    float4 color : TEXCOORD0;
    float size : PSIZE0;
};

struct ps_out
{
    float4 color : COLOR0;
};

vs_out vert(ia_out io)
{
    float4 v = float4(particles[io.instanceID].position, 1.0);
    float4 vp = mul(UNITY_MATRIX_VP, v);

    float density = particles[io.instanceID].density;
    vs_out o;
    o.vertex = vp;
    o.color = _BaseColor;
    o.color.x += density * 0.0003;
    o.color.y += density * 0.0002;
    o.color.z += density * 0.0005;
    o.size = 50.0;

    return o;
}

ps_out frag(vs_out vo)
{
    ps_out o;
    o.color = vo.color;
    return o;
}

#endif // Point_h
