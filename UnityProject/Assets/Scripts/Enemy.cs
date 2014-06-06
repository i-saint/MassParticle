using UnityEngine;
using System.Collections;

public class Enemy : MonoBehaviour {

	excDestroyable stat;
	public float accel = 0.02f;
	public float deccel = 0.99f;
	public float maxSpeed = 5.0f;

	// Use this for initialization
	void Start () {
		stat = GetComponent<excDestroyable> ();
		stat.life = 100.0f;
		
		rigidbody.maxAngularVelocity = 20.0f;
	}
	
	// Update is called once per frame
	void Update () {
		if(rigidbody) {
			Vector3 vel = rigidbody.velocity;
			vel.z -= accel;
			rigidbody.velocity = vel;

			Vector3 pos = rigidbody.transform.position;
			pos.y *= 0.98f;
			rigidbody.transform.position = pos;

			float speed = rigidbody.velocity.magnitude;
			rigidbody.velocity = rigidbody.velocity.normalized * (Mathf.Min (speed, maxSpeed) * deccel);

			rigidbody.angularVelocity *= 0.98f;
		}
	}

	void OnTriggerEnter(Collider col)
	{
		PlayerBullet bul = col.GetComponent<PlayerBullet>();
		if(bul) {
			stat.Damage(bul.power);
			Destroy (col.gameObject);
		}
	}
}
