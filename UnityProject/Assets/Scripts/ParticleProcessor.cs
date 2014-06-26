using UnityEngine;
using System.Collections;
using System;

public unsafe class ParticleProcessor : MonoBehaviour {

	MPWorld mpw;
	public float damageThreshold = 3.0f;


	// Use this for initialization
	void Start () {
		mpw = GetComponentInParent<MPWorld>();
		if(mpw) {
			mpw.particleHandler = (a, b) => Handler(a, b);
		}
	}

	unsafe void Handler(int numParticles, MPParticle* particles)
	{
		for (int i = 0; i < numParticles; ++i)
		{
			if (particles[i].hit != -1 && particles[i].hit != particles[i].hit_prev)
			{
				Collider col = mpw.colliders3d[particles[i].hit];
				Vector3 vel = *(Vector3*)&particles[i].velocity;
				Rigidbody rb = col.GetComponent<Rigidbody>();
				if (rb)
				{
					rb.AddForceAtPosition(vel * mpw.force, *(Vector3*)&particles[i].position);
				}

				if (particles[i].velocity.w > damageThreshold)
				{
					particles[i].lifetime = 0.0f;
					excDestroyable stat = col.GetComponent<excDestroyable>();
					if (stat)
					{
						stat.Damage(Math.Abs(vel.z * 0.02f));
					}
				}
			}
		}
	}
}
