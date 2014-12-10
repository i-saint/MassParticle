Shader "DeferredShading/ReverseDepth" {

Properties {
}
SubShader {
	CGINCLUDE
	#include "Compat.cginc"
	#include "DS.cginc"

	struct ia_out
	{
		float4 vertex : POSITION;
	};

	struct vs_out
	{
		float4 vertex : SV_POSITION;
		float4 screen_pos : TEXCOORD0;
	};

	struct ps_out
	{
		float4 color : COLOR0;
	};


	vs_out vert(ia_out v)
	{
		vs_out o;
		float4 t = mul(UNITY_MATRIX_MVP, v.vertex);
		o.vertex = t;
		o.screen_pos = t;
		return o;
	}

	ps_out frag (vs_out i)
	{
		ps_out r;
		r.color = i.screen_pos.z;
		return r;
	}
	ENDCG

	Pass {
		Tags { "RenderType"="Opaque" "Queue"="Geometry" }
		ZTest Greater
		ZWrite On
		Cull Front

		CGPROGRAM
		#pragma vertex vert
		#pragma fragment frag
		#pragma target 3.0
		#ifdef SHADER_API_OPENGL 
			#pragma glsl
		#endif
		ENDCG
	}
} 

}
