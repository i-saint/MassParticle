using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class MPEmitter : MonoBehaviour {

	static HashSet<MPEmitter> _instances;
	public static HashSet<MPEmitter> instances
	{
		get
		{
			if (_instances == null) { _instances = new HashSet<MPEmitter>(); }
			return _instances;
		}
	}

	public enum Shape {
		Sphere,
		Box,
	}

	public Shape shape = Shape.Sphere;
	public Vector3 velosityBase = Vector3.zero;
	public float velosityDiffuse = 0.5f;
	public int emitCount = 8;


	void OnEnable()
	{
		instances.Add(this);
	}

	void OnDisable()
	{
		instances.Remove(this);
	}

	public void MPUpdate()
	{
		Matrix4x4 mat = transform.localToWorldMatrix;
		switch (shape) {
		case Shape.Sphere:
			MPAPI.mpScatterParticlesSphereTransform (ref mat, emitCount, ref velosityBase, velosityDiffuse);
			break;

		case Shape.Box:
			MPAPI.mpScatterParticlesBoxTransform (ref mat, emitCount, ref velosityBase, velosityDiffuse);
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
