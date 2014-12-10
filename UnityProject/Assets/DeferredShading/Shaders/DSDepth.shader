Shader "DeferredShading/Depth" {

Properties {
}
SubShader {
	Tags { "RenderType"="Opaque" "Queue"="Geometry-1" }

	CGINCLUDE
	#include "Compat.cginc"
	#include "DS.cginc"
	#include "DSGBuffer.cginc"
	ENDCG

	Pass {
		Name "DepthPrePass"
		Cull Back
		ColorMask 0
		ZWrite On
		ZTest Less

		CGPROGRAM
		#pragma vertex vert
		#pragma fragment frag_no_texture
		#pragma target 3.0
		#ifdef SHADER_API_OPENGL 
			#pragma glsl
		#endif
		ENDCG
	}
}
Fallback Off
}
