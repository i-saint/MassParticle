int         g_batch_begin;
sampler2D   g_instance_data;
float       g_size;
float       g_fade_time;
float       g_spin;
float4      g_instance_data_size;


#define PI      3.1415926535897932384626433832795
float deg2rad(float  deg) { return deg*PI/180.0; }
float2 deg2rad(float2 deg) { return deg*PI/180.0; }
float3 deg2rad(float3 deg) { return deg*PI/180.0; }
float4 deg2rad(float4 deg) { return deg*PI/180.0; }

float3 iq_rand( float3 p )
{
    p = float3( dot(p,float3(127.1,311.7,311.7)), dot(p,float3(269.5,183.3,183.3)), dot(p,float3(269.5,183.3,183.3)) );
    return frac(sin(p)*43758.5453)*2.0-1.0;
}

float3x3 rotation_matrix33(float3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return float3x3(
        oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
        oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
        oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c          );
}

float4x4 rotation_matrix44(float3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return float4x4(
        oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
        oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
        oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
        0.0,                                0.0,                                0.0,                                1.0);
}


// o_pos: w=ID
// o_params: y=lifetime
void GetParticleParams(float2 i, out float4 o_pos, out float4 o_vel, out float4 o_params)
{
    int iid = i.x + g_batch_begin;
    float4 t = float4(
        g_instance_data_size.xy * float2(fmod(iid*3, g_instance_data_size.z) + 0.5, floor(iid*3/g_instance_data_size.z) + 0.5),
        0.0, 0.0);
    float4 pitch = float4(g_instance_data_size.x, 0.0, 0.0, 0.0);
    o_pos   = tex2Dlod(g_instance_data, t + pitch*0.0);
    o_vel   = tex2Dlod(g_instance_data, t + pitch*1.0);
    o_params= tex2Dlod(g_instance_data, t + pitch*2.0);
}

void ParticleTransform(inout appdata_full v, inout float4 pos, inout float4 vel, inout float4 params)
{
    GetParticleParams(v.texcoord1.xy, pos, vel, params);
    float lifetime = params.y;

    v.vertex.xyz *= g_size;
    v.vertex.xyz *= min(1.0, lifetime/g_fade_time);
    if(lifetime<=0.0) {
        v.vertex.xyz = 0.0;
    }
    if(g_spin != 0.0) {
        float ang = (dot(pos.xyz, 1.0) * min(1.0, vel.w*0.02)) * g_spin;
        float3x3 rot = rotation_matrix33(normalize(iq_rand(pos.www)), ang);
        v.vertex.xyz = mul(rot, v.vertex.xyz);
        v.normal.xyz = mul(rot, v.normal.xyz);
    }
    v.vertex.xyz += pos.xyz;
}
