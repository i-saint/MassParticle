using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public class mpForce : MonoBehaviour {

	public enum Region {
		All,
		Sphere,
		Box
	}
	public enum Direction {
		Directional,
		Radial,
	}

	public Region regionType;
	public Direction directionType; 

	public float strength = 10.0f;
	public Vector3 direction = new Vector3(0.0f,-1.0f,0.0f);

	mp.mpForceParams forceParams;

	void Start () {
	}

	void Update()
	{
		forceParams.strength = strength;
		
		switch(directionType) {
		case Direction.Directional:
			forceParams.x = direction.x;
			forceParams.y = direction.y;
			forceParams.z = direction.z;
			break;
			
		case Direction.Radial:
			forceParams.x = transform.position.x;
			forceParams.y = transform.position.y;
			forceParams.z = transform.position.z;
			break;
		}
		mp.mpAddForce ((int)regionType, transform.localToWorldMatrix, (int)directionType, forceParams);
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
			case Region.Sphere:
				Gizmos.DrawWireSphere(Vector3.zero, 0.5f);
				break;
				
			case Region.Box:
				Gizmos.color = Color.yellow;
				Gizmos.DrawWireCube(Vector3.zero, Vector3.one);
				break;
			}
			Gizmos.matrix = Matrix4x4.identity;
		}
	}
}
