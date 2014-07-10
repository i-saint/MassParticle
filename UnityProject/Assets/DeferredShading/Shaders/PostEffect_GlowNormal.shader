Shader "Custom/PostEffect_GlowNormal" {
	Properties {
	}
	SubShader {
		Tags { "RenderType"="Opaque" }
		Blend One One
		ZTest Always
		ZWrite Off
		Cull Back

		CGINCLUDE

		sampler2D _PositionBuffer;
		sampler2D _NormalBuffer;

		float  modc(float  a, float  b) { return a - b * floor(a/b); }
		float2 modc(float2 a, float2 b) { return a - b * floor(a/b); }
		float3 modc(float3 a, float3 b) { return a - b * floor(a/b); }
		float4 modc(float4 a, float4 b) { return a - b * floor(a/b); }



		struct vs_in
		{
			float4 vertex : POSITION;
		};

		struct ps_in {
			float4 vertex : SV_POSITION;
			float4 screen_pos : TEXCOORD0;
		};

		struct ps_out
		{
			float4 color : COLOR0;
		};


		ps_in vert (vs_in v)
		{
			ps_in o;
			o.vertex = v.vertex;
			o.screen_pos = v.vertex;
			return o;
		}

		ps_out frag (ps_in i)
		{
			float2 coord = (i.screen_pos.xy / i.screen_pos.w + 1.0) * 0.5;
			// see: http://docs.unity3d.com/Manual/SL-PlatformDifferences.html
			#if UNITY_UV_STARTS_AT_TOP
				coord.y = 1.0-coord.y;
			#endif

			float t = _Time.x;
			float4 p = tex2D(_PositionBuffer, coord);
			if(p.w==0.0) { discard; }

			float3 camDir = normalize(p.xyz - _WorldSpaceCameraPos);
			float tw = _ScreenParams.z - 1.0;
			float th = _ScreenParams.w - 1.0;
			float3 n1 = tex2D(_NormalBuffer, coord).xyz;
			float3 n2 = tex2D(_NormalBuffer, coord+float2(tw, 0.0)).xyz;
			float3 n3 = tex2D(_NormalBuffer, coord+float2(0.0, th)).xyz;
			float glow = max(1.0-abs(dot(camDir, n1)-0.5), 0.0)*1.5;
			if(dot(n1, n2)<0.8 || dot(n1, n3)<0.8) {
				glow += 0.2;
			}

			ps_out r = {p};
			r.color.xyz = float3(0.75, 0.75, 1.25) * glow;
			return r;
		}
		ENDCG

	Pass {
		CGPROGRAM
		#pragma vertex vert
		#pragma fragment frag
		#pragma target 3.0
		#pragma glsl
		ENDCG
	}
	} 
}
