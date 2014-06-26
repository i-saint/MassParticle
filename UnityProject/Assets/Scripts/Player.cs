using UnityEngine;
using System.Collections;

public class Player : MonoBehaviour {

	MPForceParams forceParams;
	Matrix4x4 blowMatrix;
	public GameObject playerBullet;



	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		if(Input.GetButtonDown("Fire1")) {
			Shot();
		}
		if(Input.GetButtonDown("Fire2")) {
			Blow();
		}
		{
			Vector3 move = Vector3.zero;
			move.x = Input.GetAxis ("Horizontal");
			move.z = Input.GetAxis ("Vertical");
			transform.position += move * 0.1f;
		}
		{			
			Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
			Plane plane = new Plane(Vector3.up, Vector3.zero);
			float distance = 0; 
			if (plane.Raycast(ray, out distance)){
				transform.rotation = Quaternion.LookRotation(ray.GetPoint(distance)-transform.position);
			}
		}

		{
			Matrix4x4 ts = Matrix4x4.identity;
			ts.SetColumn (3, new Vector4(0.0f,0.0f,0.5f,1.0f));
			ts = Matrix4x4.Scale(new Vector3(5.0f, 5.0f, 10.0f)) * ts;
			blowMatrix = transform.localToWorldMatrix * ts;
		}
	}

	void Shot()
	{
		Instantiate(playerBullet, transform.position + transform.forward.normalized * 1.0f, transform.rotation);
	}

	void Blow()
	{
		Vector3 pos = transform.position;
		float strength = 2000.0f;

		forceParams.strength = strength;
		forceParams.pos = pos - (transform.forward*6.0f);
		MPNative.mpAddForce (MPForceShape.Box, blowMatrix, MPForceDirection.Radial, forceParams);
	}
	
	void OnDrawGizmos()
	{
		Gizmos.color = Color.blue;
		Gizmos.matrix = blowMatrix * Matrix4x4.Scale (new Vector3(1.0f, 0.0f, 1.0f));
		Gizmos.DrawWireCube(Vector3.zero, Vector3.one);;
	}

}
