Shader "DeferredShading/GBufferRM" {

Properties {
	_BaseColor ("BaseColor", Vector) = (0.15, 0.15, 0.2, 1.0)
	_GlowColor ("GlowColor", Vector) = (0.0, 0.0, 0.0, 0.0)
	_Fovy ("Fovy", Float) = 1.8
	_FixedRadius ("MinRadius", Float) = 1.0
	_MinRadius ("FixedRadius", Float) = 0.5
}
SubShader {
	Tags { "RenderType"="Opaque" }
	Cull Back

	CGINCLUDE
	#include "Compat.cginc"

	sampler2D _MainTex;
	float4 _BaseColor;
	float4 _GlowColor;
	float _Fovy;
	float _FixedRadius;
	float _MinRadius;

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
		float4 normal : COLOR0;
		float4 position : COLOR1;
		float4 color : COLOR2;
		float4 glow : COLOR3;
	};


	// ref:
	// http://blog.hvidtfeldts.net/index.php/2011/11/distance-estimated-3d-fractals-vi-the-mandelbox/

	void sphereFold(inout float3 z, inout float dz)
	{
		float fixedRadius2 = _FixedRadius;
		float minRadius2 = _MinRadius;
		float r2 = dot(z,z);
		if (r2<minRadius2) { 
			// linear inner scaling
			float temp = (fixedRadius2/minRadius2);
			z *= temp;
			dz*= temp;
		} else if (r2<fixedRadius2) { 
			// this is the actual sphere inversion
			float temp =(fixedRadius2/r2);
			z *= temp;
			dz*= temp;
		}
	}
	void boxFold(inout float3 z, inout float dz)
	{
		const float foldingLimit = 2.0;
		z = clamp(z, -foldingLimit, foldingLimit) * 2.0 - z;
	}

	float map(float3 z)
	{
		z.y = z.y + 0.1;
		z = modc(z, 8.5) -4.25;

		float Scale = -2.7;
		float3 offset = z;
		float dr = 1.0;
		for(int n = 0; n<16; n++) {
			boxFold(z,dr);
			sphereFold(z,dr);
			z = Scale*z + offset;
			dr = dr*abs(Scale)+1.0;
		}
		float r = length(z);
		return r/abs(dr);
	}

	float3 genNormal(float3 p)
	{
		const float d = 0.01;
		return normalize( float3(
			map(p+float3(  d,0.0,0.0))-map(p+float3( -d,0.0,0.0)),
			map(p+float3(0.0,  d,0.0))-map(p+float3(0.0, -d,0.0)),
			map(p+float3(0.0,0.0,  d))-map(p+float3(0.0,0.0, -d)) ));
	}


	vs_out vert (ia_out v)
	{
		vs_out o;
		o.vertex = v.vertex;
		o.screen_pos = v.vertex;
		return o;
	}

	ps_out frag (vs_out i)
	{
		float time = _Time.x * 30.0;
		float2 pos = i.screen_pos.xy*2.0;
		// see: http://docs.unity3d.com/Manual/SL-PlatformDifferences.html
		#if UNITY_UV_STARTS_AT_TOP
			pos.y = -pos.y;
		#endif
			
		float aspect = _ScreenParams.x / _ScreenParams.y;
		pos.y /= aspect;


		float3 camPos = float3(0.0, 0.05, 0.0 - time*0.25);
		float3 camDir = normalize(float3(0.5, -0.1, -0.5));
		camDir = normalize(float3(-0.5, -0.2, -1.0));
		float3 camUp  = normalize(float3(-0.5, 1.0, 0.0));
		float3 camSide = cross(camDir, camUp);
		float focus = _Fovy;

		float3 rayDir = normalize(camSide*pos.x + camUp*pos.y + camDir*focus);
		float3 ray = camPos;
		int march = 0;
		float d = 0.0;

		float total_d = 0.0;
		const int MAX_MARCH = 64;
		const float MAX_DIST = _ProjectionParams.z;
		int mi = 0;
		for(; mi<MAX_MARCH; ++mi) {
			d = map(ray);
			march=mi;
			total_d += d;
			ray += rayDir * d;
			if(d<0.001) {break; }
		}
		if(mi==MAX_MARCH || total_d>MAX_DIST) {
			discard;
		}

		float4 rpos = mul(UNITY_MATRIX_MVP, float4(ray, 1.0));
		float3 n = genNormal(ray);

		ps_out o;
		o.normal = float4(n, 0.0);
		o.position = float4(ray, rpos.z);
		o.color = _BaseColor;
		o.glow = _GlowColor;
		return o;
	}
	ENDCG

	Pass {
		ZWrite Off
		ZTest Always

		CGPROGRAM
		#pragma vertex vert
		#pragma fragment frag
		#pragma target 3.0
		#pragma glsl
		ENDCG
	}
	Pass {
		Name "DepthPrePass"
		Tags { "DepthPrePass" = "DepthPrePass" }
		ColorMask 0
		ZWrite On
		ZTest LEqual
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
