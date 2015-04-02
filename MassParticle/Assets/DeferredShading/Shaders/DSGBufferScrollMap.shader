Shader "DeferredShading/GBufferDefault" {

Properties {
	_BaseColor ("BaseColor", Vector) = (0.15, 0.15, 0.2, 1.0)
	_GlowColor ("GlowColor", Vector) = (0.0, 0.0, 0.0, 0.0)
	_ScrollMap ("Normal", 2D) = "white" {}
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
		#pragma fragment frag
		#pragma target 3.0
		#ifdef SHADER_API_OPENGL 
			#pragma glsl
		#endif

ps_out frag(vs_out i)
{
	ps_out o;
	o.normal = float4(i.normal.xyz, _Gloss);
	o.position = float4(i.position.xyz, i.screen_pos.z);
	o.color = _BaseColor;
	o.glow = _GlowColor;
	return o;
}

		ENDCG
	}
}
Fallback Off
}
