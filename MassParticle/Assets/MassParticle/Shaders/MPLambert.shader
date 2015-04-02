Shader "MassParticle/Lambert" {

Properties {
    _MainTex ("Base (RGB)", 2D) = "white" {}
    _Color ("Color", Color) = (0.8, 0.8, 0.8, 1.0)
    g_size ("Particle Size", Float) = 0.2
    g_fade_time ("Fade Time", Float) = 0.3
    g_spin ("Spin", Float) = 0.0
}
SubShader {
    Tags { "RenderType"="Opaque" }

CGPROGRAM
#pragma surface surf Lambert vertex:vert
#include "UnityCG.cginc"
#include "MPFoundation.cginc" 

sampler2D _MainTex;
float4 _Color;

struct Input {
    float2 uv_MainTex;
    float4 velocity;
};

void vert (inout appdata_full v, out Input data)
{
    UNITY_INITIALIZE_OUTPUT(Input,data);

    float4 pos;
    float4 vel;
    float4 params;
    ParticleTransform(v, pos, vel, params);

    data.velocity = vel;
}

void surf (Input data, inout SurfaceOutput o)
{
    o.Albedo = (_Color * tex2D(_MainTex, data.uv_MainTex)).xyz;
    o.Alpha = 1.0f;

    float speed = data.velocity.w;
    float ei = max(speed-2.0, 0.0) * 1.0;
    o.Emission = float3(0.25, 0.05, 0.025)*ei;
}
ENDCG


Pass {
    Name "ShadowCaster"
    Tags { "LightMode" = "ShadowCaster" }
        
    Fog {Mode Off}
    ZWrite On ZTest LEqual Cull Off
    Offset 1, 1

CGPROGRAM
#pragma vertex vert
#pragma fragment frag
#pragma multi_compile_shadowcaster
#include "UnityCG.cginc"
#include "MPFoundation.cginc" 

sampler2D _MainTex;
float4 _Color;

struct v2f { 
    V2F_SHADOW_CASTER;
};

v2f vert( appdata_full v )
{
    float4 pos;
    float4 vel;
    float4 params;
    ParticleTransform(v, pos, vel, params);

    v2f o;
    TRANSFER_SHADOW_CASTER(o)
    return o;
}

float4 frag( v2f i ) : SV_Target
{
    SHADOW_CASTER_FRAGMENT(i)
}
ENDCG
    }

    Pass {
        Name "ShadowCollector"
        Tags { "LightMode" = "ShadowCollector" }
        
        Fog {Mode Off}
        ZWrite On ZTest LEqual

CGPROGRAM
#pragma target 4.0
#pragma vertex vert
#pragma fragment frag
#pragma multi_compile_shadowcollector

#define SHADOW_COLLECTOR_PASS
#include "UnityCG.cginc"
#include "MPFoundation.cginc" 

sampler2D _MainTex;
float4 _Color;

struct v2f { 
    V2F_SHADOW_COLLECTOR;
};

v2f vert( appdata_full v )
{
    float4 pos;
    float4 vel;
    float4 params;
    ParticleTransform(v, pos, vel, params);

    v2f o;
    TRANSFER_SHADOW_COLLECTOR(o)
    return o;
}
        
fixed4 frag (v2f i) : SV_Target
{
    SHADOW_COLLECTOR_FRAGMENT(i)
}
ENDCG
    }
}
FallBack Off

}
