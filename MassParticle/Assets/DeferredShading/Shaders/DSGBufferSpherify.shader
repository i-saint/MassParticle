Shader "DeferredShading/GBufferSpherify" {

Properties {
	_MainTex ("Base (RGB)", 2D) = "white" {}
	_BaseColor ("BaseColor", Vector) = (0.3, 0.3, 0.3, 10.0)
	_GlowColor ("GlowColor", Vector) = (0.0, 0.0, 0.0, 0.0)
	_Sphere ("Sphere", Vector) = (0.0, 0.0, 0.0, 1.0)
}
SubShader {
	Tags { "RenderType"="Opaque" "Queue"="Geometry" }

	CGINCLUDE
#include "Compat.cginc"
#include "DS.cginc"

sampler2D _MainTex;
float4 _BaseColor;
float4 _GlowColor;
float4 _Center;
float4 _Sphere;

struct ia_out
{
	float4 vertex : POSITION;
	float4 normal : NORMAL;
};

struct vs_out
{
	float4 vertex : SV_POSITION;
	float4 screen_pos : TEXCOORD0;
	float4 position : TEXCOORD1;
	float4 normal : TEXCOORD2;
	float4 instance_pos : TEXCOORD3;
};

struct ps_out
{
	float4 normal : COLOR0;
	float4 position : COLOR1;
	float4 color : COLOR2;
	float4 glow : COLOR3;
};


vs_out vert(ia_out v)
{
	vs_out o;
	float4 vmvp = mul(UNITY_MATRIX_MVP, v.vertex);
	o.vertex = vmvp;
	o.screen_pos = vmvp;
	o.position = mul(_Object2World, v.vertex);
	o.normal = normalize(mul(_Object2World, float4(v.normal.xyz,0.0)));
	o.instance_pos = float4(_Object2World[0].w, _Object2World[1].w, _Object2World[2].w, _Object2World[3].w);
	return o;
}

ps_out frag(vs_out i)
{
	float3 sphere_pos = i.instance_pos.xyz;
	float sphere_radius = _Sphere.x;

	float3 s_normal = normalize(_WorldSpaceCameraPos.xyz - i.position.xyz);
	float3 pos_rel = i.position.xyz - sphere_pos;
	float s_dist = dot(pos_rel, s_normal);
	float3 pos_proj = i.position.xyz - s_dist*s_normal;

	float dist_proj = length(pos_proj-sphere_pos);
	if(dist_proj>sphere_radius) {
		discard;
	}

	ps_out o;
	float len = length(pos_rel);
	if(len<sphere_radius) {
		o.normal = i.normal;
		o.position = float4(i.position.xyz, i.screen_pos.z);
	}
	else {
		float s_dist2 = length(pos_proj-sphere_pos);
		float s_dist3 = sqrt(sphere_radius*sphere_radius - s_dist2*s_dist2);
		float3 ps = pos_proj + s_normal * s_dist3;

		float3 dir = normalize(ps-sphere_pos);
		float3 pos = sphere_pos+dir*sphere_radius;
		float4 spos = mul(UNITY_MATRIX_VP, float4(pos,1.0));
		o.normal = float4(dir, 0.0);
		o.position = float4(pos, spos.z);
	}

	float glow_radius = _Sphere.y;
	float rcp_glow_length = glow_radius==0.0 ? 0.0 : 1.0/glow_radius;
	float g = max(len-sphere_radius+glow_radius, 0.0) * rcp_glow_length;

	o.color = _BaseColor;
	o.glow = _GlowColor * g;
	return o;
}
	ENDCG

	Pass {
		Name "Shading"
		Cull Back
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
Fallback Off
}
