Shader "Custom/PostEffect_Bloom" {

Properties {
	_Intensity ("Intensity", Float) = 0.3
}
SubShader {
	Tags { "RenderType"="Opaque" }
	Blend One One
	ZTest Always
	ZWrite Off
	Cull Back

	CGINCLUDE
	#include "Compat.cginc"

	sampler2D _GlowBuffer;
	sampler2D _HalfGlowBuffer;
	sampler2D _QuarterGlowBuffer;
	float _Intensity;


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


	vs_out vert (ia_out v)
	{
		vs_out o;
		o.vertex = v.vertex;
		o.screen_pos = v.vertex;
		return o;
	}

	ps_out frag (vs_out i)
	{
		float2 coord = (i.screen_pos.xy / i.screen_pos.w + 1.0) * 0.5;
		// see: http://docs.unity3d.com/Manual/SL-PlatformDifferences.html
		#if UNITY_UV_STARTS_AT_TOP
			coord.y = 1.0-coord.y;
		#endif

		float4 c = 0;
		c += tex2D(_GlowBuffer, coord) * _Intensity;
		c += tex2D(_HalfGlowBuffer, coord) * _Intensity;
		c += tex2D(_QuarterGlowBuffer, coord) * _Intensity;
		c.w = 0.0;

		ps_out r = {c};
		return r;
	}
	ENDCG

	Pass {
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
