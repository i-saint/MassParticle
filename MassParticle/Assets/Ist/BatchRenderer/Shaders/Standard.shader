Shader "BatchRenderer/Standard" {
Properties {
    _Color ("Color", Color) = (1,1,1,1)
    _MainTex ("Albedo (RGB)", 2D) = "white" {}
    _Glossiness ("Smoothness", Range(0,1)) = 0.5
    _Metallic ("Metallic", Range(0,1)) = 0.0
}
SubShader {
    Tags { "RenderType"="Opaque" "Queue"="Geometry+1" }

CGPROGRAM
#if defined(SHADER_API_D3D9)
    #pragma target 3.0
#else
    #pragma target 4.0
#endif
#define ENABLE_INSTANCE_BUFFER
#define ENABLE_INSTANCE_SCALE
#define ENABLE_INSTANCE_ROTATION
#define ENABLE_INSTANCE_UVOFFSET
#define ENABLE_INSTANCE_EMISSION
#if SHADER_TARGET > 40
    // this will exceed max interpolator counts on shader model 3.0
    #define ENABLE_INSTANCE_COLOR
#endif


#pragma surface surf Standard fullforwardshadows vertex:vert addshadow

#define BR_STANDARD
#include "Surface.cginc"
ENDCG
} 
FallBack Off
}
