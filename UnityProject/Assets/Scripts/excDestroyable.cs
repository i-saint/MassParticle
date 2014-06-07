using UnityEngine;
using System.Collections;

public class excDestroyable : MonoBehaviour {

	private Rigidbody rb;
	public float life = 10.0f;
	
	public Vector3 accelDir = Vector3.zero;
	public float accel = 0.02f;
	public float deccel = 0.99f;
	public float maxSpeed = 5.0f;

	// Use this for initialization
	void Start () {
		rb = GetComponent<Rigidbody> ();
	}
	
	// Update is called once per frame
	void Update () {
		if(IsDead()) {
			float volume = transform.localScale.x*transform.localScale.y*transform.localScale.z;
			int numFraction = (int)(volume * 500.0f);
			mp.mpScatterParticlesBoxTransform(transform.localToWorldMatrix, numFraction, Vector3.zero, 3.0f);
			Destroy (gameObject);
		}
		if(Mathf.Abs (transform.position.x)>25.0f ||
		   Mathf.Abs (transform.position.z)>25.0f )
		{
			Destroy (gameObject);
		}

		
		if(rb) {
			Vector3 vel = rb.velocity;
			vel.x -= accel;
			rigidbody.velocity = vel;
			
			Vector3 pos = rb.transform.position;
			pos.y *= 0.98f;
			rb.transform.position = pos;
			
			float speed = rb.velocity.magnitude;
			rb.velocity = rb.velocity.normalized * (Mathf.Min (speed, maxSpeed) * deccel);
			
			rb.angularVelocity *= 0.98f;
		}
	}

	public void Damage(float v)
	{
		life -= v;
	}

	public bool IsDead()
	{
		return life<=0.0f;
	}
}
