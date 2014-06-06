using UnityEngine;
using System.Collections;

public class Enemy : MonoBehaviour {


	// Use this for initialization
	void Start () {
		GetComponent<excDestroyable>().life = 100.0f;
	}
	
	// Update is called once per frame
	void Update () {
		if(rigidbody) {
			Vector3 vel = rigidbody.velocity;
			vel.z -= 0.01f;
			rigidbody.velocity = vel;
		}
	}
}
