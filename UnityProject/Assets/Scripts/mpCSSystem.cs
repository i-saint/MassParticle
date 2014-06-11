#if MP_PURE_CSHARP
using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace mpCS
{
	unsafe public class mpSystem
	{
		static const int MAX_PARTICLE = 65536;
		fixed mpParticle particles[MAX_PARTICLE];
		fixed mpIMData imdata[MAX_PARTICLE];
		int numActiveParticles;
		mpKernelParams kparams;

		void Update()
		{
			fixed (mpParticle* ps = particles)
			{
				fixed (mpIMData* im = imdata)
				{
					UpdateImpl(ps,im);
					Integrate(ps, im);
				}
			}
		}

		void UpdateImpl(mpParticle* ps, mpIMData* im)
		{
			for (int i = 0; i < numActiveParticles; ++i)
			{
				Vector3 pos1 = ps[i].position3;
				for (int j = 0; j < numActiveParticles; ++j)
				{
					if (i == j) { continue; }
					Vector3 pos2 = ps[j].position3;

					const float mpRcpParticleSize2 = 1.0f / (kparams.ParticleSize * 2.0f);
					Vector3 diff = pos2 - pos1;    
					float d = Vector3.Magnitude(diff);
					Vector3 dir = diff * mpRcpParticleSize2; // vec3 dir = diff / d;
					im[i].accel += dir * (Mathf.Min(0.0f, d - (kparams.ParticleSize * 2.0f)) * kparams.PressureStiffness);
				}
			}
		}

		void Integrate(mpParticle* ps, mpIMData* im)
		{
			for (int i = 0; i < numActiveParticles; ++i)
			{
				Vector3 pos = ps[i].position3;
				Vector3 vel = ps[i].velocity3;
				Vector3 accel = im[i].accel;

				vel += accel * kparams.Timestep;
				vel *= kparams.Decelerate;
				vel *= kparams.Scaler;

				pos += vel * kparams.Timestep;
				pos *= kparams.Scaler;

				ps[i].position3 = pos;
				ps[i].velocity3 = vel;
				ps[i].speed = Vector3.Magnitude(vel);
			}
		}
	};



}
#endif // MP_PURE_CSHARP
