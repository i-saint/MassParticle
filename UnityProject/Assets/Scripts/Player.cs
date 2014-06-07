using UnityEngine;
using System.Collections;

public class Player : MonoBehaviour {

	mp.mpForceParams forceParams;
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
			var aim = Input.mousePosition;
			aim.z = Camera.main.transform.position.y;
			aim = Camera.main.ScreenToWorldPoint(aim);
			transform.rotation = Quaternion.LookRotation(aim-transform.position);
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
		forceParams.pos = pos - (transform.forward*3.0f);
		mp.mpAddForce (mp.mpForceShape.Box, blowMatrix, mp.mpForceDirection.Radial, forceParams);
	}
	
	void OnDrawGizmos()
	{
		Gizmos.color = Color.blue;
		Gizmos.matrix = blowMatrix * Matrix4x4.Scale (new Vector3(1.0f, 0.0f, 1.0f));
		Gizmos.DrawWireCube(Vector3.zero, Vector3.one);;
	}

}
