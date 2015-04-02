Shader "Custom/PostEffect_Glowline" {
Properties {
	_GridPattern ("GridPattern", Int) = 0
	_SpreadPattern ("SpreaddPattern", Int) = 0
	_Intensity ("Intensity", Float) = 1.0
	_BaseColor ("BaseColor", Vector) = (0.45, 0.4, 2.0, 0.0)
	_GridSize ("GridSize", Vector) = (0.526, 0.526, 0.526, 0.0)
	_GridScale ("GridScale", Float) = 1.25

}
SubShader {
	Tags { "RenderType"="Opaque" }
	Blend One One
	ZTest Always
	ZWrite Off
	Cull Back

	CGINCLUDE
	#include "Compat.cginc"

	sampler2D _PositionBuffer;
	sampler2D _NormalBuffer;
	int _GridPattern;
	int _SpreadPattern;
	float _Intensity;
	float4 _BaseColor;
	float _GridScale;

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


	struct ia_out
	{
		float4 vertex : POSITION;
	};

	struct vs_out
	{
		float4 vertex : SV_POSITION;
		float4 screen_pos : TEXCOORD0;
	};

	struct ps_out
	{
		float4 color : COLOR0;
		float4 glow : COLOR1;
	};


	vs_out vert(ia_out v)
	{
		vs_out o;
		o.vertex = v.vertex;
		o.screen_pos = v.vertex;
		return o;
	}

	float square_grid_pattern(float3 p, float3 n, float gridsize, float linewidth)
	{
		float remain = gridsize-linewidth;
		float3 gp1 = abs(modc(p, gridsize));

		// pattern
		{
			int3 div = ceil(p / gridsize);
			int divhs = div.x + div.y + div.z;
			int divhs2 = div.x*2 + -div.y + div.z*4;
			int divhs3 = -div.x*4 + div.y*3 + -div.z;
			if(divhs%5==0 || divhs2%9==0 || divhs3%11==0) { gp1.xyz=0.0f; }
		}

		float r = 1.0;
		float t = 0.7;
		if(abs(n.x)>t) {
			if(gp1.y<remain && gp1.z<remain) {
				r = 0.0;
			}
		}
		else if(abs(n.z)>t) {
			if(gp1.x<remain && gp1.y<remain) {
				r = 0.0;
			}
		}
		else {
			if(gp1.x<remain && gp1.z<remain) {
				r = 0.0;
			}
		}
		return r;
	}
	

	float hex( float2 p, float2 h )
	{
		float2 q = abs(p);
		return max(q.x-h.y,max(q.x+q.y*0.57735,q.y*1.1547)-h.x);
	}
	float hex_pattern(float3 p, float3 n, float scale)
	{
		float2 grid = float2(0.692, 0.4) * scale;
		float radius = 0.22 * scale;

		float2 p2d;
		float t = 0.7;
		if(abs(n.x)>t) {
			p2d = p.yz;
		}
		else if(abs(n.z)>t) {
			p2d = p.xy;
		}
		else {
			p2d = p.xz;
		}

		float2 p1 = modc(p2d, grid) - grid*0.5;
		float c1 = hex(p1, radius);

		float2 p2 = modc(p2d+grid*0.5, grid) - grid*0.5;
		float c2 = hex(p2, radius);
	
		float hexd = min(c1, c2);
		return hexd>0.0 ? 1.0 : 0.0;
	}

	float2 boxcell(float3 p3, float3 n)
	{
		float2 p;
		float t = 0.7;
		if(abs(n.x)>t) {
			p = p3.yz;
		}
		else if(abs(n.z)>t) {
			p = p3.xy;
		}
		else {
			p = p3.xz;
		}

		p = frac(p);
		float r = 0.123;
		float v = 0.0, g = 0.0;
		r = frac(r * 9184.928);
		float cp, d;
		
		d = p.x;
		g += pow(clamp(1.0 - abs(d), 0.0, 1.0), 1000.0);
		d = p.y;
		g += pow(clamp(1.0 - abs(d), 0.0, 1.0), 1000.0);
		d = p.x - 1.0;
		g += pow(clamp(3.0 - abs(d), 0.0, 1.0), 1000.0);
		d = p.y - 1.0;
		g += pow(clamp(1.0 - abs(d), 0.0, 1.0), 10000.0);
		
		
		const int iter = 11;
		for(int i = 0; i < iter; i ++)
		{
			cp = 0.5 + (r - 0.5) * 0.9;
			d = p.x - cp;
			//g += pow(clamp(1.0 - abs(d), 0.0, 1.0), 200.0);
			g += clamp(1.0 - abs(d), 0.0, 1.0) > 0.999-(0.00075*i) ? 1.0 : 0.0;
			if(d > 0.0) {
				r = frac(r * 4829.013);
				p.x = (p.x - cp) / (1.0 - cp);
				v += 1.0;
			}
			else {
				r = frac(r * 1239.528);
				p.x = p.x / cp;
			}
			p = p.yx;
		}
		v /= float(iter);
		return float2(g, v);
	}

	ps_out frag(vs_out i)
	{
		float2 coord = (i.screen_pos.xy / i.screen_pos.w + 1.0) * 0.5;
		#if UNITY_UV_STARTS_AT_TOP
			coord.y = 1.0-coord.y;
		#endif

		float t = _Time.y;
		float4 p = tex2D(_PositionBuffer, coord);
		if(p.w==0.0) { discard; }
		float4 n = tex2D(_NormalBuffer, coord);

		float d = 0.0;
		if     (_SpreadPattern==0) { d = -length(p.xyz*0.15); }
		else if(_SpreadPattern==1) { d = voronoi(p.xyz*0.05); }

		float pc = 0.0;

		float gridsize = 0.526;
		float linewidth = 0.0175;
		if     (_GridPattern==0) { pc = square_grid_pattern(p, n, gridsize, linewidth); }
		else if(_GridPattern==1) { pc = hex_pattern(p, n, _GridScale); }
		else if(_GridPattern==2) {
				float2 dg = boxcell(p*0.1, n);
				pc = 1.0-clamp(1.0 - max(min(dg.x, 2.0)-1.0, 0.0)*2.0, 0.0, 1.0);
				d -= dg.y*0.5;
		}
		float vg = max(0.0, frac(1.0-d*0.75-t*0.25)*3.0-2.0) * pc;

		float4 c = _BaseColor * (vg*_Intensity);
		ps_out r = {c,c};
		return r;
	}

	ENDCG

	Pass {
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
}
