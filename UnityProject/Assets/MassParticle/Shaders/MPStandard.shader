Shader "MassParticle/MPStandard" {

Properties {
    _MainTex ("Base (RGB)", 2D) = "white" {}
    _Color ("Color", Color) = (0.8, 0.8, 0.8, 1.0)
    _Glossiness ("Smoothness", Range(0,1)) = 0.5
    _Metallic ("Metallic", Range(0,1)) = 0.0
    g_size ("Particle Size", Float) = 0.08
    g_fade_time ("Fade Time", Float) = 0.3
}

CGINCLUDE
int g_batch_begin;
sampler2D g_instance_data;
float g_size;
float g_fade_time;
float4 g_instance_data_size;

int GetInstanceID(float2 i)
{
    return i.x + g_batch_begin;
}

float4 InstanceTexcoord(int i)
{
    i *= 3;
    return float4(
        g_instance_data_size.xy * float2(fmod(i, g_instance_data_size.z) + 0.5, floor(i/g_instance_data_size.z) + 0.5),
        0.0, 0.0);
}

ENDCG

SubShader {
    Tags { "RenderType"="Opaque" }

CGPROGRAM
#pragma surface surf Standard fullforwardshadows vertex:vert
#pragma target 3.0

sampler2D _MainTex;
fixed4 _Color;
half _Glossiness;
half _Metallic;


struct Input {
    float2 uv_MainTex;
    float4 velocity;
};

void vert(inout appdata_full v, out Input data)
{
    UNITY_INITIALIZE_OUTPUT(Input,data);

    int iid = GetInstanceID(v.texcoord1.xy);
    float4 itc = InstanceTexcoord(iid);
    float4 pitch = float4(g_instance_data_size.x, 0.0, 0.0, 0.0);
    float4 position = tex2Dlod(g_instance_data, itc + pitch*0.0);
    float4 velocity = tex2Dlod(g_instance_data, itc + pitch*1.0);
    float4 params   = tex2Dlod(g_instance_data, itc + pitch*2.0);
    float lifetime = params.y;

    v.vertex.xyz *= g_size;
    v.vertex.xyz *= min(1.0, lifetime/g_fade_time);
    if(lifetime<=0.0) {
        v.vertex.xyz = 0.0;
    }
    v.vertex.xyz += position.xyz;
    data.velocity = velocity;
}

void surf(Input IN, inout SurfaceOutputStandard o)
{
    float speed = IN.velocity.w;

    fixed4 c = tex2D(_MainTex, IN.uv_MainTex) * _Color;
    o.Albedo = c.rgb;
    o.Metallic = _Metallic;
    o.Smoothness = _Glossiness;
    o.Alpha = c.a;

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
    #pragma glsl
    #include "UnityCG.cginc"

    sampler2D _MainTex;

    struct v2f { 
        V2F_SHADOW_CASTER;
    };

    v2f vert( appdata_full v )
    {
        int iid = GetInstanceID(v.texcoord1.xy);
        float4 itc = InstanceTexcoord(iid);
        float4 pitch = float4(g_instance_data_size.x, 0.0, 0.0, 0.0);
        float4 position = tex2Dlod(g_instance_data, itc + pitch*0.0);
        float4 velocity = tex2Dlod(g_instance_data, itc + pitch*1.0);
        float4 params   = tex2Dlod(g_instance_data, itc + pitch*2.0);
        float lifetime = params.y;

        v.vertex.xyz *= g_size;
        v.vertex.xyz *= min(1.0, lifetime/g_fade_time);
        if(lifetime<=0.0) {
            v.vertex.xyz = 0.0;
        }
        v.vertex.xyz += position.xyz;

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

}
FallBack Off

}
