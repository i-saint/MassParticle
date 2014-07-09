Shader "DeferredShading/GBufferDefault" {
	Properties {
		_MainTex ("Base (RGB)", 2D) = "white" {}
		_BaseColor ("BaseColor", Vector) = (0.15, 0.15, 0.2, 1.0)
		_GlowColor ("GlowColor", Vector) = (0.75, 0.75, 1.0, 1.0)
	}
	SubShader {
		Tags { "RenderType"="Opaque" }
		Cull Back

		CGINCLUDE

		sampler2D _MainTex;
		float4 _BaseColor;
		float4 _GlowColor;


		struct vs_in
		{
			float4 vertex : POSITION;
			float4 normal : NORMAL;
		};

		struct ps_in {
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
		};


		ps_in vert (vs_in v)
		{
			ps_in o;
			float4 vmvp = mul(UNITY_MATRIX_MVP, v.vertex);
			o.vertex = vmvp;
			o.screen_pos = vmvp;
			o.position = mul(_Object2World, v.vertex);
			o.normal = normalize(mul(_Object2World, float4(v.normal.xyz,0.0)));
			return o;
		}

		ps_out frag (ps_in i)
		{
			ps_out o;
			o.normal = i.normal;
			o.position = float4(i.position.xyz, i.screen_pos.z);
			o.color = _BaseColor;
			o.glow = _GlowColor;
			return o;
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
		#pragma glsl
		ENDCG
	}
	} 
}
