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
			Fire();
		}
	}

	void Fire()
	{
		Instantiate(playerBullet, transform.position + transform.forward.normalized * 1.0f, transform.rotation);
	}
}
