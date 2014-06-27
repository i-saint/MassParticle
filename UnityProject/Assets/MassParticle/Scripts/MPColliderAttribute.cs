using UnityEngine;
using System.Collections;

public class MPColliderAttribute : MonoBehaviour {

	public delegate void ParticleHitHandler(MPWorld world, GameObject obj, ref MPParticle particle);
	public delegate void GatheredHitHandler(MPWorld world, GameObject obj, ref MPHitData hit);

	public bool sendCollision = true;
	public bool receiveCollision = false;
	public uint groupMask = 0xffffffff;
	public float stiffness = 1500.0f;
	public float bounce = 1.0f;
	public float damageOnHit = 0.0f;

	public ParticleHitHandler particleHitHandler;
	public GatheredHitHandler gatheredHitHandler;
	public MPColliderProperties cprops;


	public MPColliderAttribute()
	{
		particleHitHandler = DefaultParticleHitHandler;
		gatheredHitHandler = DefaultGatheredHitHandler;
	}

	public void UpdateColliderProperties()
	{
		cprops.group_mask = groupMask;
		cprops.stiffness = stiffness;
		cprops.bounce = bounce;
		cprops.damage_on_hit = damageOnHit;
	}


	public static void DefaultParticleHitHandler(MPWorld world, GameObject obj, ref MPParticle particle)
	{
		float force = world.force;
		Vector3 vel = particle.velocity3;

		Rigidbody rb3d = obj.GetComponent<Rigidbody>();
		if (rb3d)
		{
			rb3d.AddForceAtPosition(vel * force, particle.position3);
		}

		Rigidbody2D rb2d = obj.GetComponent<Rigidbody2D>();
		if (rb2d)
		{
			rb2d.AddForceAtPosition(vel * force, particle.position3);
		}
	}

	public static void DefaultGatheredHitHandler(MPWorld world, GameObject obj, ref MPHitData hit)
	{
		float force = world.force;
		Vector3 vel = hit.velocity3;

		Rigidbody rb3d = obj.GetComponent<Rigidbody>();
		if (rb3d)
		{
			rb3d.AddForceAtPosition(vel * force, hit.position3);
		}

		Rigidbody2D rb2d = obj.GetComponent<Rigidbody2D>();
		if (rb2d)
		{
			rb2d.AddForceAtPosition(vel * force, hit.position3);
		}
	}
}
