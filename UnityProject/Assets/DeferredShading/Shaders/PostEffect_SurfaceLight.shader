Shader "Custom/PostEffect_SurfaceLight" {
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
sampler2D _PrevResult;
float _Intensity;
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

float3 lighting(float3 EyePos, float3 EyeDir, float3 LightPos, float3 LightColor, float3 FragPos, float3 Normal, float3 Albedo, float Shininess, float Gloss)
{
	float3 LightDiff	= LightPos - FragPos;
	float LightDistSq	= dot(LightDiff, LightDiff);
	float LightDist		= sqrt(LightDistSq);
	float3  LightDir	= LightDiff / LightDist;

	float3 h		= normalize(EyeDir + LightDir);
	float nh		= max(dot(Normal, h), 0.0);
	float Specular	= pow(nh, Shininess);
	float Intensity	= max(dot(Normal, LightDir), 0.0);

	float3 Result	= 0.0;
	Result += LightColor * (Albedo * Intensity);
	Result.rgb += LightColor * Specular * Gloss;
	return Result;
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
	float4 as = tex2D(_ColorBuffer, coord);
	float3 EyeDir	= normalize(_WorldSpaceCameraPos.xyz - p.xyz);

	const int NumRays = 8;
	ps_out r;
	r.color = 0.0;
	for(int j=0; j<NumRays; ++j) {
		float3 raypos = p + ((n+iq_rand(p.xyz*_Time.y+j)) * _RayAdvance * 0.5);
		float4 tpos = mul(UNITY_MATRIX_MVP, float4(raypos, 1.0) );
		float2 tcoord = (tpos.xy / tpos.w + 1.0) * 0.5;
		#if UNITY_UV_STARTS_AT_TOP
			tcoord.y = 1.0-tcoord.y;
		#endif

		float3 raydir = tex2D(_NormalBuffer, tcoord).xyz;
		float3 raycolor = tex2D(_GlowBuffer, tcoord).xyz*_Intensity;
		float4 rayfrom = tex2D(_PositionBuffer, tcoord);
		if(dot(raycolor,raycolor)>0.0) {
			r.color.rgb += lighting(_WorldSpaceCameraPos.xyz, EyeDir, rayfrom.xyz, raycolor, p.xyz, n, as.xyz, 10.0, as.a);
		}
	}
	r.color = max(r.color, tex2D(_PrevResult, coord));
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
