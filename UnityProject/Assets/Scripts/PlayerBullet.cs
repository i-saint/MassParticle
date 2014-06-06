using UnityEngine;
using System.Collections;

public class PlayerBullet : MonoBehaviour {

	public float speed = 10.0f;
	public float power = 30.0f;

	// Use this for initialization
	void Start () {
		rigidbody.velocity = transform.forward.normalized * speed;
	}
	
	// Update is called once per frame
	void Update () {
		if(Mathf.Abs (transform.position.x)>20.0f ||
		   Mathf.Abs (transform.position.z)>20.0f )
		{
			Destroy (gameObject);
		}
	}
}
