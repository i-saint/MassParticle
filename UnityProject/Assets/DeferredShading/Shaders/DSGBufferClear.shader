Shader "DeferredShading/GBufferClear" {
	Properties {
	}
	SubShader {
		Tags { "RenderType"="Opaque" }
		ZTest Always
		ZWrite On
		Cull Back

		CGINCLUDE


		struct vs_in
		{
			float4 vertex : POSITION;
			float4 normal : NORMAL;
		};

		struct ps_in {
			float4 vertex : SV_POSITION;
			float4 screen_pos : TEXCOORD0;
		};

		struct ps_out
		{
			float4 normal : COLOR0;
			float4 position : COLOR1;
			float4 color : COLOR2;
			float4 glow : COLOR3;
			float depth : DEPTH;
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
			ps_out o;
			o.normal = 0.0;
			o.position = 0.0;
			o.color = 0.0;
			o.glow = 0.0;
			o.depth = 1.0;
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
	} 
}
