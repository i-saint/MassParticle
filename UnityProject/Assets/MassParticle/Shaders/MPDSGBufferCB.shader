Shader "MassParticle/ParticleGBufferCB" {

Properties {
    _BaseColor ("BaseColor", Vector) = (0.15, 0.15, 0.2, 5.0)
    _GlowColor ("GlowColor", Vector) = (0.0, 0.0, 0.0, 0.0)
    _HeatColor ("HeatColor", Vector) = (0.25, 0.05, 0.025, 0.0)
    _HeatThreshold ("HeatThreshold", Float) = 2.5
    _HeatIntensity ("HeatIntensity", Float) = 1.0
    _Scale ("Scale", Float) = 1.0
    _FadeTime ("FadeTime", Float) = 0.1
}
SubShader {
    Tags { "RenderType"="Opaque" }

    CGINCLUDE
    #include "UnityCG.cginc"

    float4 _BaseColor;
    float4 _GlowColor;
    float4 _HeatColor;
    float _HeatThreshold;
    float _HeatIntensity;
    float _Scale;
    float _FadeTime;

    sampler2D particle_data;
    float particle_data_pitch;
    float particle_size;

    struct Vertex
    {
        float3 position;
        float3 normal;
    };
    StructuredBuffer<Vertex> vertices;

    struct ia_out {
        uint vertexID : SV_VertexID;
        uint instanceID : SV_InstanceID;
    };

    struct vs_out {
        float4 vertex : SV_POSITION;
        float4 screen_pos : TEXCOORD0;
        float4 position : TEXCOORD1;
        float4 normal : TEXCOORD2;
        float4 emission : TEXCOORD3;
    };

    struct ps_out
    {
        float4 normal : COLOR0;
        float4 position : COLOR1;
        float4 color : COLOR2;
        float4 glow : COLOR3;
    };

    vs_out vert(ia_out io)
    {
        float4 pitch = float4(particle_data_pitch, 1.0/256.0, 0.0, 0.0);
        float4 px = float4(pitch.x, 0.0, 0.0, 0.0);
        float4 coord = float4(pitch.x*3.0 * (io.instanceID%1024), pitch.y * (io.instanceID/1024), 0.0, 0.0) + pitch*0.5;
        float4 position = tex2Dlod(particle_data, coord+px*0.0);
        float4 velocity = tex2Dlod(particle_data, coord+px*1.0);
        float4 params = tex2Dlod(particle_data, coord+px*2.0);
        float lifetime = params.w;

        float4 v = float4(vertices[io.vertexID].position*(particle_size*100.0)+position.xyz, 1.0);
        float4 n = float4(vertices[io.vertexID].normal, 0.0);
        float4 vp = mul(UNITY_MATRIX_VP, v);

        vs_out o;
        o.vertex = vp;
        o.screen_pos = vp;
        o.position = v;
        o.normal.xyz = normalize(n.xyz);
        o.normal.w = 1.0;

        float speed = velocity.w;
        float heat = max(speed-_HeatThreshold, 0.0) * _HeatIntensity;
        o.emission = _GlowColor + _HeatColor * heat;
        o.emission.w = lifetime;
        return o;
    }

    ps_out frag(vs_out vo)
    {
        if(vo.emission.w==0.0) {
            discard;
        }
        ps_out o;
        o.normal = vo.normal;
        o.position = float4(vo.position.xyz, vo.screen_pos.z);
        o.color = _BaseColor;
        o.glow = vo.emission;
        return o;
    }

    float4 frag_dummy(vs_out vo) : COLOR0
    {
        if(vo.emission.w==0.0) {
            discard;
        }
        return 0.0;
    }

    ENDCG

    Pass {
        Name "DepthPrePass"
        ColorMask 0
        ZWrite On
        ZTest Less
        Cull Back

        CGPROGRAM
        #pragma vertex vert
        #pragma fragment frag_dummy
        #pragma target 5.0
        #ifdef SHADER_API_OPENGL 
            #pragma glsl
        #endif
        ENDCG
    }
    Pass {
        Name "Shading"
        Cull Back
        ZWrite Off
        ZTest Equal

        CGPROGRAM
        #pragma vertex vert
        #pragma fragment frag
        #pragma target 5.0
        #ifdef SHADER_API_OPENGL 
            #pragma glsl
        #endif
        ENDCG
    }
}
Fallback Off

}
