Shader "DeferredShading/GBufferDefault" {

Properties {
	_BaseColor ("BaseColor", Vector) = (0.15, 0.15, 0.2, 1.0)
	_GlowColor ("GlowColor", Vector) = (0.0, 0.0, 0.0, 0.0)
	_Gloss ("Gloss", Float) = 1.0
}
SubShader {
	Tags { "RenderType"="Opaque" "Queue"="Geometry" }

	CGINCLUDE
	#include "Compat.cginc"
	#include "DS.cginc"
	#include "DSGBuffer.cginc"
	ENDCG

	Pass {
		Name "Shading"
		Cull Back
		ZWrite On
		ZTest LEqual

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
