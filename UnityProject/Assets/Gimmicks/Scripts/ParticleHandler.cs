using UnityEngine;
using System.Collections;

public unsafe class ParticleHandler : MonoBehaviour {

	MPWorld mpw;


	void Start()
	{
		mpw = GetComponentInParent<MPWorld>();
		if (mpw)
		{
			mpw.particleProcessor = ParticleProcessor;
		}
	}
	
	// Update is called once per frame
	void Update () {
	
	}

	public static void ParticleProcessor(MPWorld world, int numParticles, MPParticle* particles)
	{
		for (int i = 0; i < numParticles; ++i)
		{
			if (particles[i].hit==-1 || particles[i].hit == particles[i].hit_prev) { continue; }

            MPCollider col = MPCollider.GetHitOwner(particles[i].hit);
			RedirectForceToParent cp = col.GetComponent<RedirectForceToParent>();
			if (cp)
			{
				Transform parent = col.transform.parent;
				if (parent)
				{
					MPUtils.CallParticleHitHandler(world, parent.gameObject.GetComponent<MPCollider>(), ref particles[i]);
				}
			}
			else
			{
				MPUtils.CallParticleHitHandler(world, col, ref particles[i]);
			}
		}
	}


	public static unsafe void GatheredHitProcessor(MPWorld world, int numHits, MPHitData* hits)
	{
		for (int i = 0; i < numHits; ++i)
		{
			if (hits[i].num_hits == 0) { continue; }
            MPCollider col = MPCollider.GetHitOwner(i);
			RedirectForceToParent cp = col.GetComponent<RedirectForceToParent>();
			if (cp)
			{
				Transform parent = col.transform.parent;
				if (parent)
				{
                    MPUtils.CallGathereditHandler(world, parent.gameObject.GetComponent<MPCollider>(), ref hits[i]);
				}
			}
			else
			{
				MPUtils.CallGathereditHandler(world, col, ref hits[i]);
			}
		}
	}
}
