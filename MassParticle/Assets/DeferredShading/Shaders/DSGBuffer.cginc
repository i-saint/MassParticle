
#ifdef SHADER_API_PSSL
#	define COLOR0 SV_Target0
#	define COLOR1 SV_Target1
#	define COLOR2 SV_Target2
#	define COLOR3 SV_Target3
#endif

sampler2D _AlbedoMap;
sampler2D _GlossMap;
sampler2D _NormalMap;
sampler2D _SpecularMap;
float4 _BaseColor = 1.0;
float4 _GlowColor = 0.0;
float _Gloss = 1.0;


struct ia_out
{
	float4 vertex : POSITION;
	float4 normal : NORMAL;
	float2 texcoord : TEXCOORD0;
	float4 tangent : TANGENT;
};

struct vs_out
{
	float4 vertex : SV_POSITION;
	float4 screen_pos : TEXCOORD0;
	float4 position : TEXCOORD1;
	float3 normal : TEXCOORD2;
	float2 texcoord : TEXCOORD3;
	float4 tangent : TEXCOORD4;
	float3 binormal : TEXCOORD5;
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
	o.normal = normalize(mul(_Object2World, float4(v.normal.xyz,0.0)).xyz);
	o.tangent = float4(normalize(mul(_Object2World, float4(v.tangent.xyz,0.0)).xyz), v.tangent.w);
	o.binormal = normalize(cross(o.normal, o.tangent) * v.tangent.w);
	o.texcoord = v.texcoord;
	return o;
}

ps_out frag_no_texture(vs_out i)
{
	ps_out o;
	o.normal = float4(i.normal.xyz, _Gloss);
	o.position = float4(i.position.xyz, i.screen_pos.z);
	o.color = _BaseColor;
	o.glow = _GlowColor;
	return o;
}

ps_out frag_textured(vs_out i)
{
	float2 coord = i.texcoord;

	float3 albedo = tex2D(_AlbedoMap, coord).rgb;

	float3x3 tbn = float3x3( i.tangent.xyz, i.binormal, i.normal.xyz);
	float3 normal = tex2D(_NormalMap, coord).rgb*2.0-1.0;
	normal = normalize(mul(normal, tbn));

	float gloss = tex2D(_GlossMap, coord).r;
	float spec = tex2D(_SpecularMap, coord).r;

	ps_out o;
	o.normal = float4(normal, gloss);
	o.position = float4(i.position.xyz, i.screen_pos.z);
	o.color = float4(albedo*_BaseColor.rgb, spec);
	o.glow = _GlowColor;
	return o;
}
