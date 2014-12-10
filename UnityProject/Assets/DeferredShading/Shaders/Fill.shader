Shader "DeferredShading/Fill" {

Properties {
}
SubShader {
	Tags { "RenderType"="Transparent" }
	ZTest Always
	ZWrite Off
	Cull Back
	Blend SrcAlpha OneMinusSrcAlpha

	CGINCLUDE
	#include "Compat.cginc"

	sampler2D _PositionBuffer1;
	sampler2D _PositionBuffer2;
	float4 _Color;

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


	vs_out vert (ia_out v)
	{
		vs_out o;
		o.vertex = v.vertex;
		o.screen_pos = v.vertex;
		return o;
	}

	ps_out frag(vs_out i)
	{
		ps_out o;
		o.color = _Color;
		return o;
	}

	ps_out frag2(vs_out i)
	{
		float2 coord = (i.screen_pos.xy / i.screen_pos.w + 1.0) * 0.5;
		// see: http://docs.unity3d.com/Manual/SL-PlatformDifferences.html
		#if UNITY_UV_STARTS_AT_TOP
			coord.y = 1.0-coord.y;
		#endif

		float4 pos1 = tex2D(_PositionBuffer1, coord);
		float4 pos2 = tex2D(_PositionBuffer2, coord);
		float diff = length(pos1.xyz-pos2.xyz);
		float s = diff * 4.0;
		if(pos1.w==0.0 || pos2.w==0.0) { s=1.0; }

		ps_out o;
		o.color = _Color;
		o.color.a = min(s+0.025, 1.0);
		return o;
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

	Pass {
		CGPROGRAM
		#pragma vertex vert
		#pragma fragment frag2
		#pragma target 3.0
		#ifdef SHADER_API_OPENGL 
			#pragma glsl
		#endif
		ENDCG
	}
}
}
