Shader "DeferredShading/GBufferSubtractStencil" {

Properties {
	_MainTex ("Base (RGB)", 2D) = "white" {}
	_BaseColor ("BaseColor", Vector) = (0.15, 0.15, 0.2, 1.0)
	_GlowColor ("GlowColor", Vector) = (0.75, 0.75, 1.0, 1.0)
}
SubShader {
	CGINCLUDE

	struct vs_in
	{
		float4 vertex : POSITION;
	};

	struct ps_in {
		float4 vertex : SV_POSITION;
	};

	struct ps_out
	{
		float4 color : COLOR0;
	};


	ps_in vert (vs_in v)
	{
		ps_in o;
		o.vertex = mul(UNITY_MATRIX_MVP, v.vertex);
		return o;
	}

	ps_out frag (ps_in i)
	{
		ps_out o;
		o.color = 0.0;
		return o;
	}
	ENDCG

	Pass {
		Tags { "RenderType"="Opaque" "Queue"="Geometry+1" }
		Stencil {
			Ref 1
			Comp always
			Pass replace
		}
		ColorMask 0
		ZWrite Off
		ZTest Less
		Cull Back

		CGPROGRAM
		#pragma vertex vert
		#pragma fragment frag
		#pragma target 3.0
		#pragma glsl
		ENDCG
	}
} 

}
