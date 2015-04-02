Shader "DeferredShading/StencilClear" {

Properties {
}
SubShader {
	Tags { "RenderType"="Opaque" "Queue"="Background+1" }

	CGINCLUDE
	#include "Compat.cginc"

	struct ia_out
	{
		float4 vertex : POSITION;
	};

	struct vs_out
	{
		float4 vertex : SV_POSITION;
	};

	struct ps_out
	{
		float4 color : COLOR0;
	};


	vs_out vert(ia_out v)
	{
		vs_out o;
		o.vertex = mul(UNITY_MATRIX_MVP, v.vertex);
		return o;
	}

	ps_out frag(vs_out i)
	{
		ps_out o;
		o.color = 0.0;
		return o;
	}
	ENDCG

	Pass {
		Stencil {
			Ref 0
			Comp Always
			Pass Replace
		}
		ColorMask 0
		ZWrite Off
		ZTest Always
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
