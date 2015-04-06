#ifndef Trail_h
#define Trail_h

#include "UnityCG.cginc"
#include "MPGPFoundation.cginc"

float4 _BaseColor;
float _FadeTime;
StructuredBuffer<Particle> particles;
StructuredBuffer<TrailParams> params;
StructuredBuffer<TrailVertex> vertices;

struct ia_out {
    uint vertexID : SV_VertexID;
    uint instanceID : SV_InstanceID;
};

struct vs_out {
    float4 vertex : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float4 color : TEXCOORD01;
};

struct ps_out
{
    float4 color : COLOR0;
};

vs_out vert(ia_out io)
{
    float lifetime = particles[io.instanceID].lifetime;
    float fade = min(lifetime/_FadeTime, 1.0);

    uint ii = (particles[io.instanceID].id % params[0].max_entities) * params[0].max_history * 2;
    const uint ls[6] = {0,3,1, 0,2,3};
    uint iv = (io.vertexID/6)*2 + ls[io.vertexID%6];
    TrailVertex tv = vertices[ii + iv];

    float4 v = float4(tv.position, 1.0);
    if(lifetime<=0.0) { v=0.0; }
    float4 vp = mul(UNITY_MATRIX_VP, v);

    vs_out o;
    o.vertex = vp;
    o.texcoord = tv.texcoord;
    o.color = _BaseColor * fade;

    return o;
}

ps_out frag(vs_out vo)
{
    ps_out o;
    float ua = pow( 1.0 - abs(vo.texcoord.x*2.0f-1.0f)+0.0001, 0.5 );
    float va = pow((vo.texcoord.y+0.0001), 0.5);
    o.color = vo.color * ua * va;
    return o;
}
#endif // Trail_h
