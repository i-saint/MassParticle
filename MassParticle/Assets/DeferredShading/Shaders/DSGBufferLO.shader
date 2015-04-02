Shader "DeferredShading/GBufferLO" {

Properties {
	_MainTex ("Base (RGB)", 2D) = "white" {}
	_BaseColor ("BaseColor", Vector) = (0.15, 0.15, 0.2, 1.0)
	_GlowColor ("GlowColor", Vector) = (0.0, 0.0, 0.0, 0.0)
}
SubShader {
	Tags { "RenderType"="Opaque" }

	CGINCLUDE
	#include "Compat.cginc"
	#include "DS.cginc"

	sampler2D _MainTex;
	float4 _BaseColor;
	float4 _GlowColor;
	int _EnableLogicOp;
	sampler2D _RDepthBuffer;
	sampler2D _AndRDepthBuffer;
	sampler2D _AndNormalBuffer;
	sampler2D _AndPositionBuffer;
	sampler2D _AndColorBuffer;
	sampler2D _AndGlowBuffer;


	struct ia_out
	{
		float4 vertex : POSITION;
		float4 normal : NORMAL;
	};

	struct vs_out
	{
		float4 vertex : SV_POSITION;
		float4 screen_pos : TEXCOORD0;
		float4 position : TEXCOORD1;
		float4 normal : TEXCOORD2;
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
		float4 vmvp = mul(UNITY_MATRIX_MVP, v.vertex);
		o.vertex = vmvp;
		o.screen_pos = vmvp;
		o.position = mul(_Object2World, v.vertex);
		o.normal = normalize(mul(_Object2World, float4(v.normal.xyz,0.0)));
		return o;
	}

	ps_out frag (vs_out i)
	{
		float2 coord = (i.screen_pos.xy / i.screen_pos.w + 1.0) * 0.5;
		// see: http://docs.unity3d.com/Manual/SL-PlatformDifferences.html
		#if UNITY_UV_STARTS_AT_TOP
			coord.y = 1.0-coord.y;
		#endif

		if(_EnableLogicOp==0) {
			ps_out o;
			o.normal = i.normal;
			o.position = float4(i.position.xyz, i.screen_pos.z);
			o.color = _BaseColor;
			o.glow = _GlowColor;
			o.depth = ComputeDepth(i.screen_pos);
			return o;
		}
		else {
			float4 pos1 = float4(i.position.xyz, i.screen_pos.z);
			float4 pos2 = tex2D(_AndPositionBuffer, coord);
			float rdepth1 = tex2D(_RDepthBuffer, coord).x;
			float rdepth2 = tex2D(_AndRDepthBuffer, coord).x;
			ps_out r;
			if(pos2.w==0.0 || pos1.w>rdepth2 || pos2.w>rdepth1) {
				discard;
			}
			else if(pos1.w<pos2.w) {
				r.position	= pos2;
				r.normal	= tex2D(_AndNormalBuffer, coord);
				r.color		= tex2D(_AndColorBuffer, coord);
				r.glow		= tex2D(_AndGlowBuffer, coord);
				//r.glow		= 1.0;
				r.depth = ComputeDepth(mul(UNITY_MATRIX_VP, float4(pos2.xyz, 1.0)));
			}
			else {
				r.position	= pos1;
				r.normal	= i.normal;
				r.color		= _BaseColor;
				r.glow		= _GlowColor;
				r.depth = ComputeDepth(i.screen_pos);
			}
			return r;
		}
	}
	ENDCG

	Pass {
		ZTest Less
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
FallBack Off
}
