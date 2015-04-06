Shader "GPUParticle/Lambert" {

Properties {
    _MainTex ("Base (RGB)", 2D) = "white" {}
    _Color ("Color", Color) = (0.8, 0.8, 0.8, 1.0)
    _Emission ("Emission", Color) = (0.0, 0.0, 0.0, 0.0)
    g_size ("Particle Size", Float) = 0.2
    g_fade_time ("Fade Time", Float) = 0.3
    g_spin ("Spin", Float) = 0.0
}
SubShader {
    Tags { "RenderType"="Opaque" }

CGPROGRAM
#pragma surface surf Lambert vertex:vert

#define MPGP_SURFACE
#include "MPGPSurface.cginc" 
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

#define MPGP_SHADOW_CASTER
#include "MPGPSurface.cginc" 
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

#define MPGP_SHADOW_COLLECTOR
#include "MPGPSurface.cginc" 
ENDCG
    }
}
FallBack Off

}
