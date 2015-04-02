Shader "DeferredShading/GBufferClear" {

Properties {
}
SubShader {
	Tags { "RenderType"="Opaque" }

	CGINCLUDE
	#include "Compat.cginc"

	struct ia_out
	{
		float4 vertex : POSITION;
		float4 normal : NORMAL;
	};

	struct vs_out
	{
		float4 vertex : SV_POSITION;
	};

	struct ps_out
	{
		float4 normal : COLOR0;
		float4 position : COLOR1;
		float4 color : COLOR2;
		float4 glow : COLOR3;
		float depth : DEPTH;
	};


	vs_out vert (ia_out v)
	{
		vs_out o;
		o.vertex = v.vertex;
		return o;
	}

	ps_out frag (vs_out i)
	{
		ps_out o;
		o.normal = 0.0;
		o.position = 0.0;
		o.color = 0.0;
		o.glow = 0.0;
		o.depth = 1.0;
		return o;
	}
	ENDCG

	Pass {
		Stencil {
			Ref 0
			Comp Always
			Pass Replace
		}
		ZTest Always
		ZWrite On
		Cull Back

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
