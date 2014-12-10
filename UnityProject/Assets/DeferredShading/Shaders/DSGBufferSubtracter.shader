Shader "DeferredShading/GBufferSubtract" {

Properties {
	_MainTex ("Base (RGB)", 2D) = "white" {}
	_BaseColor ("BaseColor", Vector) = (0.15, 0.15, 0.2, 1.0)
	_GlowColor ("GlowColor", Vector) = (0.0, 0.0, 0.0, 0.0)
	_ClearColor ("ClearColor", Vector) = (0.0, 0.0, 0.0, 0.0)
	_Depth ("Depth", 2D) = "white" {}
}
SubShader {
	Tags { "RenderType"="Opaque" "Queue"="Background+2" }

	CGINCLUDE
	#include "Compat.cginc"
	#include "DS.cginc"
	sampler2D _MainTex;
	sampler2D _Depth;
	float4 _BaseColor;
	float4 _GlowColor;
	float4 _ClearColor;


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
		o.normal = -normalize(mul(_Object2World, float4(v.normal.xyz,0.0)));
		return o;
	}

	ps_out frag (vs_out i)
	{
		float2 coord = (i.screen_pos.xy / i.screen_pos.w + 1.0) * 0.5;
		// see: http://docs.unity3d.com/Manual/SL-PlatformDifferences.html
		#if UNITY_UV_STARTS_AT_TOP
			coord.y = 1.0-coord.y;
		#endif
		float z = tex2D(_Depth, coord).x;
		if(z==0.0) { discard; }

		ps_out o;
		if(i.screen_pos.z>z) {
			o.normal = 0.0;
			o.position = 0.0;
			o.color = _ClearColor;
			o.glow = 0.0;
			o.depth = 1.0;
		}
		else {
			o.normal = i.normal;
			o.position = float4(i.position.xyz, i.screen_pos.z);
			o.color = _BaseColor;
			o.glow = _GlowColor;
			o.depth = ComputeDepth(i.screen_pos);
		}
		return o;
	}
	ENDCG

	Pass {
		Stencil {
			Ref 1
			Comp Equal
		}
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
