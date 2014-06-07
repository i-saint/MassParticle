using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public class mpWorld : MonoBehaviour {

	//public float particleRadius;
	public mp.mpSolverType solverType;
	public float particleLifeTime;
	public float timeStep;
	public float deceleration;
	public float pressureStiffness;
	public float wallStiffness;
	public Vector3 coordScale;
	public float damageThreshold = 3.0f;

	private Collider[] colliders;


	void Reset()
	{
		mp.mpKernelParams p = mp.mpGetKernelParams();
		transform.position = p.WorldCenter;
		transform.localScale = p.WorldSize;
		solverType 			= (mp.mpSolverType)p.SolverType;
		particleLifeTime 	= p.LifeTime;
		timeStep 			= p.Timestep;
		deceleration 		= p.Decelerate;
		pressureStiffness 	= p.PressureStiffness;
		wallStiffness 		= p.WallStiffness;
		coordScale 			= p.Scaler;
	}

	void Start () {
		mp.mpClearParticles();
	}

	void Update()
	{
		{
			mp.mpKernelParams p = mp.mpGetKernelParams();
			p.WorldCenter 		= transform.position;
			p.WorldSize 		= transform.localScale;
			p.SolverType		= (int)solverType;
			p.LifeTime			= particleLifeTime;
			p.Timestep			= timeStep;
			p.Decelerate		= deceleration;
			p.PressureStiffness	= pressureStiffness;
			p.WallStiffness		= wallStiffness;
			p.Scaler			= coordScale;
			mp.mpSetKernelParams(ref p);
		}

		colliders = Physics.OverlapSphere( transform.position, transform.localScale.magnitude );
		for (int i = 0; i < colliders.Length; ++i )
		{
			Collider col = colliders[i];
			SphereCollider sphere = col as SphereCollider;
			BoxCollider box = col as BoxCollider;
			int ownerid = col.rigidbody ? i : -1;
			if(sphere) {
				mp.mpAddSphereCollider(ownerid, sphere.transform.position, sphere.radius);
			}
			else if (box)
			{
				mp.mpAddBoxCollider(ownerid, box.transform.localToWorldMatrix, box.size);
			}
		}

		mp.mpUpdate (Time.timeSinceLevelLoad);
		ProcessParticleCollision();
	}
	
	void OnRenderObject()
	{		
		UnityEngine.Camera cam = UnityEngine.Camera.current;
		if (cam) {
			mp.mpSetViewProjectionMatrix(cam.worldToCameraMatrix, cam.projectionMatrix);
		}
		GL.IssuePluginEvent (1);
	}

	void OnDrawGizmos()
	{
		Gizmos.color = Color.yellow;
		Gizmos.DrawWireCube(transform.position, transform.localScale*2.0f);
	}


	unsafe void ProcessParticleCollision()
	{
		uint numParticles = mp.mpGetNumParticles();
		mp.mpParticle *particles = mp.mpGetParticles();
		for(uint i=0; i<numParticles; ++i) {
			if(particles[i].hit != -1 && particles[i].hit!=particles[i].hit_prev) {
				Collider col = colliders[particles[i].hit];
				if(col.isTrigger) { continue; }
				Rigidbody rb = col.GetComponent<Rigidbody>();
				if(rb) {
					Vector3 vel = *(Vector3*)&particles[i].velocity;
					rb.AddForceAtPosition( vel * 0.02f, *(Vector3*)&particles[i].position );

					if(particles[i].velocity.w > damageThreshold) {
						particles[i].lifetime = 0.0f;
						excDestroyable stat = col.GetComponent<excDestroyable>();
						if(stat) {
							stat.Damage(Math.Abs(vel.z*0.02f));
						}
					}
				}
			}
		}
	}
}
