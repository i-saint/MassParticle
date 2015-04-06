Shader "GPUParticle/Point AlphaBlended" {

Properties {
    _BaseColor ("BaseColor", Vector) = (0.15, 0.15, 0.2, 5.0)
    _FadeTime ("FadeTime", Float) = 0.1
}
Category {
    Tags { "Queue"="Transparent" "IgnoreProjector"="True" "RenderType"="Transparent" }
    Blend SrcAlpha One
    AlphaTest Greater .01
    ColorMask RGB
    Cull Off Lighting Off ZWrite Off Fog { Color (0,0,0,0) }

    SubShader {
        Pass {
            Cull Back
            ZWrite On
            ZTest LEqual

CGPROGRAM
#pragma target 5.0
#pragma vertex vert
#pragma fragment frag 
#include "Point.cginc"
ENDCG
    }
}
Fallback Off
}
}
