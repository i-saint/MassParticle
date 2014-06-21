cbuffer cbChangesEveryFrame : register( b0 )
{
    matrix g_ViewProjection;
    float4 g_CameraPos;
    float4 g_LightPos;
    float4 g_LightColor;
    float4 g_MeshColor;
    float4 g_params; // x: size, y: shininess
};

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float4 InstancePos : INSTANCE_POSITION;
    float4 InstanceVel : INSTANCE_VELOCITY;
    float4 InstanceParams : INSTANCE_PARAMS;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 LsPos : TEXCOORD0;
    float4 Color : TEXCOORD1;
    float4 Emission : TEXCOORD2;
    float3 Normal : NORMAL;
};

struct PS_OUT
{
    float4 Color : SV_Target0;
};


PS_INPUT VS( VS_INPUT input )
{
    float scaleByLifetime = min(input.InstanceParams.w*3.0, 1.0);
    float ei = max(input.InstanceVel.w-2.5, 0.0) * 0.5;

    PS_INPUT output = (PS_INPUT)0;
    output.LsPos    = float4(input.Pos, 0.0f) * g_params.x * scaleByLifetime + input.InstancePos;
    output.Pos      = mul(float4(output.LsPos.xyz, 1.0), g_ViewProjection);
    output.Color    = float4(0.8f, 0.8f, 0.8f, 1.0f);
    output.Emission = float4(ei,ei,ei,ei) * float4(0.25, 0.05, 0.025, 0.0);
    output.Normal   = input.Normal;

    return output;
}

PS_OUT PS( PS_INPUT input)
{
    float3 FragPos      = input.LsPos.xyz;
    float3 LightPos     = g_LightPos.xyz;
    float3 LightColor   = g_LightColor.rgb;
    float3 LightDiff    = LightPos - FragPos;
    float LightDist2    = dot(LightDiff, LightDiff);
    float LightDist     = sqrt(LightDist2);
    float3 LightDir     = LightDiff / LightDist;

    float3 Albedo       = input.Color.rgb;
    float Shininess     = g_params.y;
    float3 Normal       = input.Normal.xyz;
    float3 EyePos       = g_CameraPos.xyz;
    float3 EyeDir       = normalize(EyePos - FragPos);

    float3 h            = normalize(EyeDir + LightDir);
    float nh            = max(dot(Normal, h), 0.0);
    float Specular      = pow(nh, Shininess);
    float Intensity     = max(dot(Normal, LightDir), 0.0);

    float4 Result = float4(0.0, 0.0, 0.0, 1.0);
    Result.rgb += LightColor * (Albedo * Intensity);
    Result.rgb += LightColor * Specular;
    Result.rgb += input.Emission;


    PS_OUT output;
    output.Color = Result;
    return output;
}
