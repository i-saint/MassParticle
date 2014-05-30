//--------------------------------------------------------------------------------------
// File: Tutorial07.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

cbuffer cbChangesEveryFrame : register( b0 )
{
    matrix g_ViewProjection;
    float4 g_CameraPos;

    float4 g_LightPos;
    float4 g_LightColor;

    float4 g_MeshColor;
    float g_MeshShininess;
};


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float4 InstancePos : INSTANCE_POSITION;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 LsPos : TEXCOORD0;
    float4 Color : TEXCOORD1;
    float3 Normal : NORMAL;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.LsPos    = (float4(input.Pos, 0.0f) * float4(0.05f, 0.05f, 0.05f, 1.0f)) + input.InstancePos;
    output.Pos      = mul(output.LsPos, g_ViewProjection);
    output.Color    = float4(0.8f, 0.8f, 0.8f, 1.0f);
    output.Normal   = input.Normal;
    output.Pos.z *= 4.0;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
    float3 FragPos      = input.LsPos.xyz;
    float3 LightPos     = g_LightPos.xyz;
    float3 LightColor   = g_LightColor.rgb;
    float3 LightDiff    = LightPos - FragPos;
    float LightDist2    = dot(LightDiff, LightDiff);
    float LightDist     = sqrt(LightDist2);
    float3 LightDir     = LightDiff / LightDist;

    float3 Albedo       = input.Color.rgb;
    float Shininess     = g_MeshShininess;
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

    return Result;
}
