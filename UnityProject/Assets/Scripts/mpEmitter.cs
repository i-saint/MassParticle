using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class mpEmitter : MonoBehaviour {

	[DllImport ("MassParticle")] private static extern uint mpScatterParticlesSphererical(Vector3 center, float radius, uint num);

	public int emitCount = 8;
	public float radius = 0.5f;


	void Start () {
	}

	void Update()
	{
		mpScatterParticlesSphererical (transform.position, radius, (uint)emitCount);
	}

}
