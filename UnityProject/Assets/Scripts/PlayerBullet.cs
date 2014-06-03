using UnityEngine;
using System.Collections;

public class PlayerBullet : MonoBehaviour {

	public float speed = 10.0f;

	// Use this for initialization
	void Start () {
		rigidbody.velocity = transform.forward.normalized * speed;
	}
	
	// Update is called once per frame
	void Update () {
	
	}
}
