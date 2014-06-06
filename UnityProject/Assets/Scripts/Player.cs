using UnityEngine;
using System.Collections;

public class Player : MonoBehaviour {

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
		Vector3 move = Vector3.zero;
		move.x = Input.GetAxis ("Horizontal");
		move.z = Input.GetAxis ("Vertical");
		transform.position += move * 0.1f;
	}

	void Shot()
	{
		Instantiate(playerBullet, transform.position + transform.forward.normalized * 1.0f, transform.rotation);
	}

	void Blow()
	{
		Matrix4x4 mat = transform.localToWorldMatrix * Matrix4x4.Scale(Vector3.one * 20.0f);
		mp.mpForceParams p = new mp.mpForceParams();
		p.pos = transform.forward;
		p.strength = 2000.0f;
		mp.mpAddForce ((int)mp.mpForceShape.Sphere, mat, (int)mp.mpForceDirection.Directional, p);
	}
}
