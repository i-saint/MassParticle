using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class mpEmitter : MonoBehaviour {

	[DllImport ("MassParticle")] private static extern uint mpScatterParticlesSphererical(Vector3 center, float radius, uint num);

	public int emitCount = 8;


	void Start () {
	}

	void Update()
	{
		mpScatterParticlesSphererical (transform.position, 0.5f, (uint)emitCount);
	}

}
