using UnityEngine;
using System.Collections;

public unsafe class ParticleHandler : MonoBehaviour {

	mpWorld mpw;


	void Start()
	{
		mpw = GetComponentInParent<mpWorld>();
		if (mpw)
		{
			mpw.particleHandler = (a, b) => Handler(a, b);
		}
	}
	
	// Update is called once per frame
	void Update () {
	
	}

	void Handler(int numParticles, mp.mpParticle* particles)
	{
		for (int i = 0; i < numParticles; ++i)
		{
			if (particles[i].hit != -1 && particles[i].hit != particles[i].hit_prev)
			{
				Collider col = mpw.colliders3d[particles[i].hit];
				Vector3 vel = *(Vector3*)&particles[i].velocity;

				RedirectForceToParent cp = col.GetComponent<RedirectForceToParent>();
				if (cp)
				{
					Rigidbody rb = cp.GetComponentInParent<Rigidbody>();
					if (rb)
					{
						rb.AddForceAtPosition(vel * mpw.force, *(Vector3*)&particles[i].position);
					}
				}
				else
				{
					Rigidbody rb = col.GetComponent<Rigidbody>();
					if (rb)
					{
						rb.AddForceAtPosition(vel * mpw.force, *(Vector3*)&particles[i].position);
					}
				}
			}
		}
	}
}
