Shader "Custom/PostEffect_BloomBlur" {

Properties {
}
SubShader {
	Tags { "RenderType"="Opaque" }
	Blend Off
	ZTest Always
	ZWrite Off
	Cull Back

	CGINCLUDE
	#include "Compat.cginc"

	sampler2D _FrameBuffer;
	sampler2D _GlowBuffer;
	float4 _Screen;

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
		o.vertex = v.vertex;
		o.screen_pos = v.vertex;
		return o;
	}

	ps_out hblur(vs_out i)
	{
		float2 coord = (i.screen_pos.xy / i.screen_pos.w + 1.0) * 0.5;
		#if UNITY_UV_STARTS_AT_TOP
			coord.y = 1.0-coord.y;
		#endif

		float Weight[5] = {0.05, 0.09, 0.12, 0.16, 0.16};
		float2 s = float2((_Screen.z)*1.39, 0.0);
		float4 c = 0.0;
		c += tex2D(_GlowBuffer, coord - s*4.0) * Weight[0];
		c += tex2D(_GlowBuffer, coord - s*3.0) * Weight[1];
		c += tex2D(_GlowBuffer, coord - s*2.0) * Weight[2];
		c += tex2D(_GlowBuffer, coord - s*1.0) * Weight[3];
		c += tex2D(_GlowBuffer, coord        ) * Weight[4];
		c += tex2D(_GlowBuffer, coord + s*1.0) * Weight[3];
		c += tex2D(_GlowBuffer, coord + s*2.0) * Weight[2];
		c += tex2D(_GlowBuffer, coord + s*3.0) * Weight[1];
		c += tex2D(_GlowBuffer, coord + s*4.0) * Weight[0];
		ps_out r = {c};
		return r;
	}
	
	ps_out vblur(vs_out i)
	{
		float2 coord = (i.screen_pos.xy / i.screen_pos.w + 1.0) * 0.5;
		#if UNITY_UV_STARTS_AT_TOP
			coord.y = 1.0-coord.y;
		#endif

		float Weight[5] = {0.05, 0.09, 0.12, 0.16, 0.16};
		float2 s = float2(0.0, (_Screen.w)*1.0);
		float4 c = 0.0;
		c += tex2D(_GlowBuffer, coord - s*4.0) * Weight[0];
		c += tex2D(_GlowBuffer, coord - s*3.0) * Weight[1];
		c += tex2D(_GlowBuffer, coord - s*2.0) * Weight[2];
		c += tex2D(_GlowBuffer, coord - s*1.0) * Weight[3];
		c += tex2D(_GlowBuffer, coord        ) * Weight[4];
		c += tex2D(_GlowBuffer, coord + s*1.0) * Weight[3];
		c += tex2D(_GlowBuffer, coord + s*2.0) * Weight[2];
		c += tex2D(_GlowBuffer, coord + s*3.0) * Weight[1];
		c += tex2D(_GlowBuffer, coord + s*4.0) * Weight[0];
		ps_out r = {c};
		return r;
	}
	ENDCG

	Pass {
		CGPROGRAM
		#pragma vertex vert
		#pragma fragment hblur
		#pragma target 3.0
		#ifdef SHADER_API_OPENGL 
			#pragma glsl
		#endif
		ENDCG
	}
	Pass {
		CGPROGRAM
		#pragma vertex vert
		#pragma fragment vblur
		#pragma target 3.0
		#ifdef SHADER_API_OPENGL 
			#pragma glsl
		#endif
		ENDCG
	}
}
}
