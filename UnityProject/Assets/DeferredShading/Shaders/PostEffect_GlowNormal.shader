Shader "Custom/PostEffect_GlowNormal" {
Properties {
	_BaseColor ("BaseColor", Vector) = (0.75, 0.75, 1.25, 0.0)
	_Intensity ("Intensity", Float) = 1.0
	_Threshold ("Threshold", Float) = 0.5
	_Edge ("Edge", Float) = 0.2
}
SubShader {
	Tags { "RenderType"="Opaque" }
	Blend One One
	ZTest Always
	ZWrite Off
	Cull Back

	CGINCLUDE
	#include "Compat.cginc"

	sampler2D _PositionBuffer;
	sampler2D _NormalBuffer;
	float4 _BaseColor;
	float _Intensity;
	float _Threshold;
	float _Edge;


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


	vs_out vert (ia_out io)
	{
		vs_out o;
		o.vertex = io.vertex;
		o.screen_pos = io.vertex;
		return o;
	}

	ps_out frag (vs_out i)
	{
		float2 coord = (i.screen_pos.xy / i.screen_pos.w + 1.0) * 0.5;
		// see: http://docs.unity3d.com/Manual/SL-PlatformDifferences.html
		#if UNITY_UV_STARTS_AT_TOP
			coord.y = 1.0-coord.y;
		#endif

		float t = _Time.x;
		float4 p = tex2D(_PositionBuffer, coord);
		if(p.w==0.0) { discard; }

		float glow = 0.0;
		float3 camDir = normalize(p.xyz - _WorldSpaceCameraPos);
		float tw = _ScreenParams.z - 1.0;
		float th = _ScreenParams.w - 1.0;
		float4 n = tex2D(_NormalBuffer, coord);
		float3 n1 = n.xyz;
		float3 n2 = tex2D(_NormalBuffer, coord+float2(tw, 0.0)).xyz;
		float3 n3 = tex2D(_NormalBuffer, coord+float2(0.0, th)).xyz;
		glow = max(1.0-abs(dot(camDir, n1)-_Threshold), 0.0)*_Intensity;
		if(dot(n1, n2)<0.8 || dot(n1, n3)<0.8) {
			glow += _Edge;
		}

		ps_out r;
		r.color = _BaseColor * glow * n.w;
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
