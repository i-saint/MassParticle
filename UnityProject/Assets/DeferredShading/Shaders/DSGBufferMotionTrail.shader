Shader "DeferredShading/GBufferMotionTrail" {

Properties {
	_BaseColor ("BaseColor", Vector) = (0.15, 0.15, 0.2, 1.0)
	_GlowColor ("GlowColor", Vector) = (0.0, 0.0, 0.0, 0.0)
}
SubShader {
	Tags { "RenderType"="Opaque" "Queue"="Geometry" }

CGINCLUDE
#include "Compat.cginc"
#include "DS.cginc"
#include "DSGBuffer.cginc"

float4x4 prev_Object2World;
	
vs_out vert_motiontrail(ia_out v)
{
	vs_out o;

	float4 pos1 = mul(_Object2World, v.vertex);
	float4 pos2 = mul(prev_Object2World, v.vertex);
	float3 vel = pos1.xyz - pos2.xyz;
	float3 vel_dir = normalize(vel);
	float4 pos = dot(vel_dir, v.normal.xyz)>0.0 ? pos1 : pos2;

	float4 vmvp = mul(UNITY_MATRIX_VP, pos);
	o.vertex = vmvp;
	o.screen_pos = vmvp;
	o.position = pos;
	o.normal = normalize(mul(_Object2World, float4(v.normal.xyz,0.0)).xyz);
	return o;
}
ENDCG

	Pass {
		Name "Shading"
		Cull Back
		ZWrite On
		ZTest LEqual

		CGPROGRAM
		#pragma vertex vert_motiontrail
		#pragma fragment frag_no_texture
		#pragma target 3.0
		#ifdef SHADER_API_OPENGL 
			#pragma glsl
		#endif
		ENDCG
	}
}
Fallback Off
}
