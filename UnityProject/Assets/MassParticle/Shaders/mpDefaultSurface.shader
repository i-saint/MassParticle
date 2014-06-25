Shader "Custom/mpDefaultSurface" {

Properties {
	_ParticleSize ("Particle Size", Float) = 0.08
	_Color ("Color", Color) = (0.8, 0.8, 0.8, 1.0)
	_MainTex ("Base (RGB)", 2D) = "white" {}
}
SubShader {
	Tags { "RenderType"="Opaque" }
		
	CGPROGRAM
	#pragma surface surf Lambert vertex:vert

	sampler2D _MainTex;
	sampler2D _DataTex;
	float _ParticleSize;
	float _DataTexPitch;
	float4 _Color;

	struct Input {
		float4 color : COLOR;
		float4 emission;
		float2 uv_MainTex;
		float lifetime;
	};
		
	void vert (inout appdata_full v, out Input data)
	{
		UNITY_INITIALIZE_OUTPUT(Input,data);

		float4 pitch = float4(_DataTexPitch, 0.0, 0.0, 0.0);
		float4 position = tex2Dlod(_DataTex, v.texcoord);
		float4 velocity = tex2Dlod(_DataTex, v.texcoord+pitch);
		float4 params = tex2Dlod(_DataTex, v.texcoord+pitch*2.0);
		float lifetime = params.w;

		v.vertex.xyz *= (_ParticleSize*100.0);
		v.vertex.xyz *= min(1.0, lifetime*3.3333);
		v.vertex.xyz += tex2Dlod(_DataTex,v.texcoord).xyz;

		float ei = max(velocity.w-2.5, 0.0) * 1.0;
		data.emission = float4(ei,ei,ei,ei) * float4(0.25, 0.05, 0.025, 0.0);
		data.lifetime = lifetime;
	}

	void surf (Input data, inout SurfaceOutput o)
	{
		if(data.lifetime<=0.0f) {
			discard;
		}
		o.Albedo = (_Color * tex2D(_MainTex, data.uv_MainTex)).xyz;
		o.Alpha = 1.0f;
		o.Emission = data.emission;
	}
	ENDCG


	Pass {
		Name "ShadowCaster"
		Tags { "LightMode" = "ShadowCaster" }
		
		Fog {Mode Off}
		ZWrite On ZTest LEqual Cull Off
		Offset 1, 1

	CGPROGRAM
		#pragma vertex vert
		#pragma fragment frag
		#pragma multi_compile_shadowcaster
		#include "UnityCG.cginc"

		sampler2D _MainTex;
		sampler2D _DataTex;
		float _ParticleSize;
		float _DataTexPitch;
		float4 _Color;

		struct v2f { 
			V2F_SHADOW_CASTER;
		};

		v2f vert( appdata_base v )
		{
			float4 pitch = float4(_DataTexPitch, 0.0, 0.0, 0.0);
			float4 position = tex2Dlod(_DataTex, v.texcoord);
			float4 params = tex2Dlod(_DataTex, v.texcoord+pitch*2.0);
			float lifetime = params.w;
			v.vertex.xyz *= (_ParticleSize*100.0);
			v.vertex.xyz *= min(1.0, lifetime*3.3333);
			v.vertex.xyz += tex2Dlod(_DataTex,v.texcoord).xyz;

			v2f o;
			TRANSFER_SHADOW_CASTER(o)
			return o;
		}

		float4 frag( v2f i ) : SV_Target
		{
			SHADOW_CASTER_FRAGMENT(i)
		}
	ENDCG
	}

	Pass {
		Name "ShadowCollector"
		Tags { "LightMode" = "ShadowCollector" }
		
		Fog {Mode Off}
		ZWrite On ZTest LEqual

	CGPROGRAM
		#pragma vertex vert
		#pragma fragment frag
		#pragma multi_compile_shadowcollector
		#define SHADOW_COLLECTOR_PASS
		#include "UnityCG.cginc"

		sampler2D _MainTex;
		sampler2D _DataTex;
		float _ParticleSize;
		float _DataTexPitch;
		float4 _Color;

		struct v2f { 
			V2F_SHADOW_COLLECTOR;
		};

		v2f vert( appdata_base v )
		{
			float4 pitch = float4(_DataTexPitch, 0.0, 0.0, 0.0);
			float4 position = tex2Dlod(_DataTex, v.texcoord);
			float4 params = tex2Dlod(_DataTex, v.texcoord+pitch*2.0);
			float lifetime = params.w;
			v.vertex.xyz *= (_ParticleSize*100.0);
			v.vertex.xyz *= min(1.0, lifetime*3.3333);
			v.vertex.xyz += tex2Dlod(_DataTex,v.texcoord).xyz;
			
			v2f o;
			TRANSFER_SHADOW_COLLECTOR(o)
			return o;
		}
		
		fixed4 frag (v2f i) : SV_Target
		{
			SHADOW_COLLECTOR_FRAGMENT(i)
		}
	ENDCG
	}
}
FallBack "Diffuse"

}
