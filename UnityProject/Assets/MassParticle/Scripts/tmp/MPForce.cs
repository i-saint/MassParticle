using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public class MPForce : MonoBehaviour {

	public MPForceShape regionType;
	public MPForceDirection directionType; 

	public float strength = 10.0f;
	public Vector3 direction = new Vector3(0.0f,-1.0f,0.0f);

	MPForceParams forceParams;

	void Start () {
	}

	void Update()
	{
		forceParams.strength = strength;
		
		switch(directionType) {
		case MPForceDirection.Directional:
			forceParams.pos = direction;
			break;

		case MPForceDirection.Radial:
			forceParams.pos = transform.position;
			break;
		}
		MPNative.mpAddForce (regionType, transform.localToWorldMatrix, directionType, forceParams);
	}
	
	
	void OnDrawGizmos()
	{
		{
			float arrowHeadAngle = 30.0f;
			float arrowHeadLength = 0.5f;
			Vector3 pos = transform.position;
			Vector3 dir = direction * strength * 0.5f;
			
			Gizmos.matrix = Matrix4x4.identity;
			Gizmos.color = Color.yellow;
			Gizmos.DrawRay (pos, dir);
			
			Vector3 right = Quaternion.LookRotation(dir) * Quaternion.Euler(0,180+arrowHeadAngle,0) * new Vector3(0,0,1);
			Vector3 left = Quaternion.LookRotation(dir) * Quaternion.Euler(0,180-arrowHeadAngle,0) * new Vector3(0,0,1);
			Gizmos.DrawRay(pos + dir, right * arrowHeadLength);
			Gizmos.DrawRay(pos + dir, left * arrowHeadLength);
		}
		{			
			Gizmos.color = Color.yellow;
			Gizmos.matrix = transform.localToWorldMatrix;
			switch(regionType) {
			case MPForceShape.Sphere:
				Gizmos.DrawWireSphere(Vector3.zero, 0.5f);
				break;

			case MPForceShape.Box:
				Gizmos.color = Color.yellow;
				Gizmos.DrawWireCube(Vector3.zero, Vector3.one);
				break;
			}
			Gizmos.matrix = Matrix4x4.identity;
		}
	}
}
