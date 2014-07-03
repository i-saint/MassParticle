using UnityEngine;
using System.Collections;

public class Player : MonoBehaviour {

	MPForceProperties fprops;
	Matrix4x4 blowMatrix;
	Transform trans;
	public GameObject playerBullet;



	// Use this for initialization
	void Start () {
		trans = transform;
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
			trans.position += move * 0.1f;
		}
		{			
			Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
			Plane plane = new Plane(Vector3.up, Vector3.zero);
			float distance = 0; 
			if (plane.Raycast(ray, out distance)){
				trans.rotation = Quaternion.LookRotation(ray.GetPoint(distance) - trans.position);
			}
		}

		{
			Matrix4x4 ts = Matrix4x4.identity;
			ts.SetColumn (3, new Vector4(0.0f,0.0f,0.5f,1.0f));
			ts = Matrix4x4.Scale(new Vector3(5.0f, 5.0f, 10.0f)) * ts;
			blowMatrix = trans.localToWorldMatrix * ts;
		}
	}

	void Shot()
	{
		Instantiate(playerBullet, trans.position + trans.forward.normalized * 1.0f, trans.rotation);
	}

	void Blow()
	{
		Vector3 pos = trans.position;
		float strength = 2000.0f;

		fprops.SetDefaultValues();
		fprops.shape_type = MPForceShape.Box;
		fprops.dir_type = MPForceDirection.Radial;
		fprops.strength_near = strength;
		fprops.strength_far = strength;
		fprops.radial_center = pos - (trans.forward * 6.0f);
		MPAPI.mpAddForce(ref fprops, ref blowMatrix);
	}
	
	void OnDrawGizmos()
	{
		Gizmos.color = Color.blue;
		Gizmos.matrix = blowMatrix * Matrix4x4.Scale (new Vector3(1.0f, 0.0f, 1.0f));
		Gizmos.DrawWireCube(Vector3.zero, Vector3.one);;
	}

}
