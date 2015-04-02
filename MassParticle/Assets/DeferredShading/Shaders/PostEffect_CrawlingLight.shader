Shader "Custom/PostEffect_CrawlingLight" {
Properties {
	_Intensity ("Intensity", Float) = 1.0
	_RayAdvance ("RayAdvance", Float) = 1.0
}
SubShader {
	Tags { "RenderType"="Opaque" }

	CGINCLUDE
#include "Compat.cginc"

struct Inputs
{
	float4x4 vp;
	float intensity;
	float rayadvance;
};

sampler2D _NormalBuffer;
sampler2D _PositionBuffer;
sampler2D _ColorBuffer;
sampler2D _GlowBuffer;
sampler2D _GlowBufferB;
sampler2D _PrevResult;
float _RayAdvance;

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

float3 iq_rand( float3 p )
{
		p = float3( dot(p,float3(127.1,311.7,311.7)), dot(p,float3(269.5,183.3,183.3)), dot(p,float3(269.5,183.3,183.3)) );
		return frac(sin(p)*43758.5453)*2.0-1.0;
}

ps_out frag(vs_out i)
{
	float2 coord = (i.screen_pos.xy / i.screen_pos.w + 1.0) * 0.5;
	// see: http://docs.unity3d.com/Manual/SL-PlatformDifferences.html
	#if UNITY_UV_STARTS_AT_TOP
		coord.y = 1.0-coord.y;
	#endif

	float4 p = tex2D(_PositionBuffer, coord);
	if(p.w==0.0) { discard; }

	float3 n = tex2D(_NormalBuffer, coord).xyz;
	float4 glow = tex2D(_GlowBuffer, coord) * 2.0;
	float4 prev = tex2D(_PrevResult, coord);

	const int NumRays = 16;
	ps_out r;
	r.color = glow;
	if(prev.a<0.1) {
		for(int j=0; j<NumRays; ++j) {
			float3 raypos = p + ((iq_rand(p.xyz*_Time.y+j)) * _RayAdvance);
			float4 tpos = mul(UNITY_MATRIX_MVP, float4(raypos, 1.0) );
			float2 tcoord = (tpos.xy / tpos.w + 1.0) * 0.5;
			#if UNITY_UV_STARTS_AT_TOP
				tcoord.y = 1.0-tcoord.y;
			#endif

			float attenuation = 1.0;
			float4 raycolor = tex2D(_GlowBufferB, tcoord) * attenuation;
			float4 rayfrom = tex2D(_PositionBuffer, tcoord);
			float d = length(p.xyz-rayfrom.xyz);
			if(length(p.xyz-rayfrom.xyz)<_RayAdvance) {
				r.color.rgb = max(r.color.rgb, raycolor.rgb);
			}
		}
		r.color.rgb = max(prev.rgb, r.color.rgb);
	}
	else {
		r.color.rgb = max(prev.rgb*0.99-0.0001, glow.rgb);
	}

	if(dot(r.color.rgb, r.color.rgb)>0.0) {
		r.color.a = prev.a+0.0166666666;
		if(r.color.a>3.0) {
			r.color.a = 0.0;
		}
	}
	return r;
}
ENDCG

	Pass {
		Blend Off
		ZTest Always
		ZWrite Off
		Cull Back

		CGPROGRAM
		#pragma vertex vert
		#pragma fragment frag
		#pragma target 5.0
		#ifdef SHADER_API_OPENGL 
			#pragma glsl
		#endif
		ENDCG
	}
}
}
