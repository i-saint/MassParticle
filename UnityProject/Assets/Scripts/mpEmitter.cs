using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class mpEmitter : MonoBehaviour {


	public enum Shape {
		Sphere,
		Box,
	}

	public Shape shape = Shape.Sphere;
	public Vector3 velosityBase = Vector3.zero;
	public float velosityDiffuse = 0.5f;
	public int emitCount = 8;


	void Start () {
	}

	void Update()
	{
		switch (shape) {
		case Shape.Sphere:
			mp.mpScatterParticlesSphereTransform (transform.localToWorldMatrix, emitCount, velosityBase, velosityDiffuse);
			break;

		case Shape.Box:
			mp.mpScatterParticlesBoxTransform (transform.localToWorldMatrix, emitCount, velosityBase, velosityDiffuse);
			break;
		}
	}
	
	
	void OnDrawGizmos()
	{
		Gizmos.color = Color.yellow;
		Gizmos.matrix = transform.localToWorldMatrix;
		switch(shape) {
		case Shape.Sphere:
			Gizmos.DrawWireSphere(Vector3.zero, 0.5f);
			break;
			
		case Shape.Box:
			Gizmos.color = Color.yellow;
			Gizmos.DrawWireCube(Vector3.zero, Vector3.one);
			break;
		}
		Gizmos.matrix = Matrix4x4.identity;
	}
}
