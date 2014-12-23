Shader "Custom/GSBillboard" {
	Properties {
		_MainTex ("Base (RGB)", 2D) = "white" {}
		_Color ("Color", Color) = (0.8, 0.8, 0.8, 1.0)
		_ParticleSize ("Size", Float) = 0.08
	}
	SubShader {
	Pass {

		Tags { "RenderType"="Opaque" }
		LOD 200
		
		CGPROGRAM
		#pragma target 5.0

		#pragma vertex VS_Main
		#pragma fragment FS_Main
		#pragma geometry GS_Main
		#include "UnityCG.cginc" 

		struct GS_INPUT
		{
			float4	pos		: POSITION;
			float3	normal	: NORMAL;
			float2  tex0	: TEXCOORD0;
			float lifetime: TEXCOORD1;
		};

		struct FS_INPUT
		{
			float4	pos		: POSITION;
			float2  tex0	: TEXCOORD0;
			float lifetime: TEXCOORD1;
		};


		sampler2D _MainTex;
		sampler2D _DataTex;
		float _ParticleSize;
		float _DataTexPitch;
		float4 _Color;


		GS_INPUT VS_Main(appdata_base v)
		{
			GS_INPUT output = (GS_INPUT)0;
			
			float4 pitch = float4(_DataTexPitch, 0.0, 0.0, 0.0);
			float4 position = tex2Dlod(_DataTex, v.texcoord);
			float4 velocity = tex2Dlod(_DataTex, v.texcoord+pitch);
			float4 params = tex2Dlod(_DataTex, v.texcoord+pitch*2.0);
			float lifetime = params.y;

			output.pos =  position;
			output.normal = v.normal;
			output.tex0 = float2(0, 0);
			output.lifetime = lifetime;

			return output;
		}


		[maxvertexcount(4)]
		void GS_Main(point GS_INPUT p[1], inout TriangleStream<FS_INPUT> triStream)
		{
			float4x4 vp = mul(UNITY_MATRIX_MVP, _World2Object);
			float3 up = float3(0, 1, 0);
			float3 look = _WorldSpaceCameraPos - p[0].pos.xyz;
			look.y = 0;
			look = normalize(look);
			float3 right = cross(up, look);
			float halfS = _ParticleSize * min(1.0, p[0].lifetime*3.333);

			float4 v[4];
			v[0] = float4(p[0].pos.xyz + halfS * right - halfS * up, 1.0f);
			v[1] = float4(p[0].pos.xyz + halfS * right + halfS * up, 1.0f);
			v[2] = float4(p[0].pos.xyz - halfS * right - halfS * up, 1.0f);
			v[3] = float4(p[0].pos.xyz - halfS * right + halfS * up, 1.0f);

			FS_INPUT pIn;
			pIn.lifetime = p[0].lifetime;

			pIn.pos = mul(vp, v[0]);
			pIn.tex0 = float2(1.0f, 0.0f);
			triStream.Append(pIn);

			pIn.pos =  mul(vp, v[1]);
			pIn.tex0 = float2(1.0f, 1.0f);
			triStream.Append(pIn);

			pIn.pos =  mul(vp, v[2]);
			pIn.tex0 = float2(0.0f, 0.0f);
			triStream.Append(pIn);

			pIn.pos =  mul(vp, v[3]);
			pIn.tex0 = float2(0.0f, 1.0f);
			triStream.Append(pIn);
		}

		float4 FS_Main(FS_INPUT input) : COLOR
		{
			if(input.lifetime<=0.0f) {
				discard;
			}
			return _Color * tex2D(_MainTex, input.tex0);
		}
		ENDCG
	}}
}
