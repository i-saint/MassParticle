Shader "Custom/PostEffect_NormalPattern" {
Properties {
	_GridPattern ("GridPattern", Int) = 0
	_SpreadPattern ("SpreaddPattern", Int) = 0
	_Intensity ("Intensity", Float) = 1.0
	_GridSize ("GridSize", Vector) = (0.526, 0.526, 0.526, 0.0)
	_GridScale ("GridScale", Float) = 1.25

}
SubShader {
	Tags { "RenderType"="Opaque" }
	ZTest Always
	ZWrite Off
	Cull Back

	CGINCLUDE
	#include "Compat.cginc"

	sampler2D _PositionBuffer;
	sampler2D _NormalBuffer;
	int _GridPattern;
	int _SpreadPattern;
	float _Intensity;
	float _GridScale;

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
	};


	vs_out vert(ia_out v)
	{
		vs_out o;
		o.vertex = v.vertex;
		o.screen_pos = v.vertex;
		return o;
	}


	float hex( float2 p, float2 h )
	{
		float2 q = abs(p);
		return max(q.x-h.y,max(q.x+q.y*0.57735,q.y*1.1547)-h.x);
	}
	float hex_pattern(float3 p, float3 n, float scale)
	{
		float2 grid = float2(0.692, 0.4) * scale;
		float radius = 0.22 * scale;

		float2 p2d;
		float t = 0.7;
		if(abs(n.x)>t) {
			p2d = p.yz;
		}
		else if(abs(n.z)>t) {
			p2d = p.xy;
		}
		else {
			p2d = p.xz;
		}

		float2 p1 = modc(p2d, grid) - grid*0.5;
		float c1 = hex(p1, radius);

		float2 p2 = modc(p2d+grid*0.5, grid) - grid*0.5;
		float c2 = hex(p2, radius);
	
		float hexd = min(c1, c2);
		return hexd;
	}
	float3 guess_normal(float3 p, float3 n, float s)
	{
		const float d = 0.01;
		return normalize( float3(
			hex_pattern(p+float3(  d,0.0,0.0), n, s)-hex_pattern(p+float3( -d,0.0,0.0), n, s),
			hex_pattern(p+float3(0.0,  d,0.0), n, s)-hex_pattern(p+float3(0.0, -d,0.0), n, s),
			hex_pattern(p+float3(0.0,0.0,  d), n, s)-hex_pattern(p+float3(0.0,0.0, -d), n, s) ));
	}

	ps_out frag(vs_out i)
	{
		float2 coord = (i.screen_pos.xy / i.screen_pos.w + 1.0) * 0.5;
		#if UNITY_UV_STARTS_AT_TOP
			coord.y = 1.0-coord.y;
		#endif

		float t = _Time.x;
		float4 p = tex2D(_PositionBuffer, coord);
		if(p.w==0.0) { discard; }
		float4 n = tex2D(_NormalBuffer, coord);

		float d = hex_pattern(p.xyz, n.xyz, _GridScale);
		float3 gn = guess_normal(p.xyz, n.xyz, _GridScale);
		float g = saturate((d+0.02)*50.0);

		ps_out r;
		if(g==0.0 || g==1.0) {
			r.normal = n;
		}
		else {
			r.normal = float4(gn, n.w);
		}
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
