Shader "Custom/SimpleColor" {

Properties {
	_BaseColor ("BaseColor", Vector) = (0.5, 0.5, 0.5, 1.0)
}
SubShader {
	Tags { "RenderType"="Opaque" }

	CGINCLUDE
	#include "UnityCG.cginc"

	float4 _BaseColor;

	struct ia_out
	{
		float4 vertex : POSITION;
	};

	struct vs_out
	{
		float4 vertex : SV_POSITION;
	};

	struct ps_out
	{
		float4 color : COLOR0;
	};

	vs_out vert(ia_out io)
	{
		vs_out o;
		o.vertex = mul(UNITY_MATRIX_MVP, io.vertex);
		return o;
	}

	ps_out frag(vs_out vo)
	{
		ps_out o;
		o.color = _BaseColor;
		return o;
	}

	ENDCG

	Pass {
		Cull Off
		ZWrite Off
		ZTest Always
		Blend SrcAlpha OneMinusSrcAlpha
		//Blend One One

		CGPROGRAM
		#pragma target 5.0
		#pragma vertex vert
		#pragma fragment frag 
		ENDCG
	}
}
Fallback Off

}
