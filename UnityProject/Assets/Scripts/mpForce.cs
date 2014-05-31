using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public class mpForce : MonoBehaviour {

	[DllImport ("MassParticle")] private static extern uint mpAddDirectionalForce (Vector3 direction, float strength);


	public float gravityStrength = 10.0f;
	public Vector3 gravityDirection = new Vector3(0.0f,-1.0f,0.0f);


	void Start () {
	}

	void Update()
	{
		mpAddDirectionalForce (gravityDirection, gravityStrength);
	}
}
