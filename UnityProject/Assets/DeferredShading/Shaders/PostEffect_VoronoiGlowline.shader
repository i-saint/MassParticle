Shader "Custom/PostEffect_VoronoiGlowline" {
	Properties {
	}
	SubShader {
		Tags { "RenderType"="Opaque" }
		Blend One One
		ZTest Always
		ZWrite Off
		Cull Back

		CGINCLUDE

		sampler2D _PositionBuffer;
		sampler2D _NormalBuffer;

		float  modc(float  a, float  b) { return a - b * floor(a/b); }
		float2 modc(float2 a, float2 b) { return a - b * floor(a/b); }
		float3 modc(float3 a, float3 b) { return a - b * floor(a/b); }
		float4 modc(float4 a, float4 b) { return a - b * floor(a/b); }

		// thanks to iq

		float hash( float n )
		{
			return frac(sin(n)*43758.5453);
		}

		float2 hash( float2 p )
		{
			p = float2( dot(p,float2(127.1,311.7)), dot(p,float2(269.5,183.3)) );
			return frac(sin(p)*43758.5453);
		}

		float3 hash( float3 p )
		{
			p = float3( dot(p,float3(127.1,311.7,311.7)), dot(p,float3(269.5,183.3,183.3)), dot(p,float3(269.5,183.3,183.3)) );
			return frac(sin(p)*43758.5453);
		}

		float voronoi( in float2 x )
		{
			float2 n = floor(x);
			float2 f = frac(x);
			float2 mg, mr;

			float md = 8.0;
			{
				for( int j=-1; j<=1; j++ ) {
				for( int i=-1; i<=1; i++ ) {
					float2 g = float2(float(i),float(j));
					float2 o = hash( n + g );
					float2 r = g + o - f;
					float d = dot(r,r);
					if( d<md ) {
						md = d;
						mr = r;
						mg = g;
					}
				}}
			}

			md = 8.0;
			{
				for( int j=-2; j<=2; j++ ) {
				for( int i=-2; i<=2; i++ ) {
					float2 g = mg + float2(float(i),float(j));
					float2 o = hash( n + g );
					float2 r = g + o - f;
					if( dot(mr-r,mr-r)>0.000001 ) {
						float d = dot( 1.5*(mr+r), normalize(r-mr) );
						md = min( md, d );
					}
				}}
			}

			return md;
		}

		float voronoi( in float3 x )
		{
			float3 n = floor(x);
			float3 f = frac(x);
			float3 mg, mr;

			float md = 8.0;
			{
				for( int j=-1; j<=1; j++ ) {
				for( int i=-1; i<=1; i++ ) {
				for( int k=-1; k<=1; k++ ) {
					float3 g = float3(float(i),float(j),float(k));
					float3 o = hash( n + g );
					float3 r = g + o - f;
					float d = dot(r,r);
					if( d<md ) {
						md = d;
						mr = r;
						mg = g;
					}
				}}}
			}

			md = 8.0;
			{
				for( int j=-1; j<=1; j++ ) {
				for( int i=-1; i<=1; i++ ) {
				for( int k=-1; k<=1; k++ ) {
					float3 g = mg + float3(float(i),float(j),float(k));
					float3 o = hash( n + g );
					float3 r = g + o - f;
					if( dot(mr-r,mr-r)>0.000001 ) {
						float d = dot( 1.5*(mr+r), normalize(r-mr) );
						md = min( md, d );
					}
				}}}
			}

			return md;
		}


		struct vs_in
		{
			float4 vertex : POSITION;
		};

		struct ps_in {
			float4 vertex : SV_POSITION;
			float4 screen_pos : TEXCOORD0;
		};

		struct ps_out
		{
			float4 color : COLOR0;
			float4 glow : COLOR1;
		};


		ps_in vert (vs_in v)
		{
			ps_in o;
			o.vertex = v.vertex;
			o.screen_pos = v.vertex;
			return o;
		}

		ps_out frag (ps_in i)
		{
			float2 coord = (i.screen_pos.xy / i.screen_pos.w + 1.0) * 0.5;
			#if UNITY_UV_STARTS_AT_TOP
				coord.y = 1.0-coord.y;
			#endif

			float t = _Time.x;
			float4 p = tex2D(_PositionBuffer, coord);
			float4 n = tex2D(_NormalBuffer, coord);
			if(dot(p.xyz,p.xyz)==0.0) { discard; }

			float d = voronoi(p.xyz*0.075);
			float vg = max(0.0, frac(1.0-d-t*5.0+p.z*0.01)*3.0-2.0);
			float grid1 = max(0.0, max((modc((p.x+p.y+p.z*2.0)-t*5.0, 5.0)-4.0)*1.5, 0.0) );

			float gridsize = 0.526;
			float linewidth = 0.01;
			float remain = gridsize-linewidth;
			float3 gp1 = abs(modc(p, gridsize));
			if(abs(n.y)>0.9) {
				if(gp1.x<remain && gp1.z<remain) {
					vg = 0.0;
				}
			}
			else if(abs(n.z)>0.9) {
				if(gp1.x<remain && gp1.y<remain) {
					vg = 0.0;
				}
			}
			else {
				if(gp1.y<remain && gp1.z<remain) {
					vg = 0.0;
				}
			}

			float4 c = float4(float3(0.3, 0.25, 1.5) * vg, 0.0);
			ps_out r = {c,c};
			return r;
		}
		ENDCG

	Pass {
		CGPROGRAM
		#pragma vertex vert
		#pragma fragment frag
		#pragma target 3.0
		#pragma glsl
		ENDCG
	}
	} 
}
