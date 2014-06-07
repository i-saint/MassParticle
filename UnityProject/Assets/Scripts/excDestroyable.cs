using UnityEngine;
using System.Collections;

public class excDestroyable : MonoBehaviour {

	private Rigidbody rigid;
	private Transform trans;

	public float life = 10.0f;
	
	public Vector3 accelDir = Vector3.zero;
	public float accel = 0.02f;
	public float deccel = 0.99f;
	public float maxSpeed = 5.0f;

	public bool scatterFractions = true;


	// Use this for initialization
	void Start () {
		rigid = GetComponent<Rigidbody> ();
		trans = GetComponent<Transform> ();
	}
	
	// Update is called once per frame
	void Update () {
		if(IsDead()) {
			if(scatterFractions) {
				ScatterFractions();
			}
			Destroy (gameObject);
		}
		if(Mathf.Abs (trans.position.x)>30.0f ||
		   Mathf.Abs (trans.position.z)>30.0f )
		{
			Destroy (gameObject);
		}

		
		if(rigid) {
			Vector3 vel = rigid.velocity;
			vel.x -= accel;
			rigidbody.velocity = vel;
			
			Vector3 pos = rigid.transform.position;
			pos.y *= 0.98f;
			rigid.transform.position = pos;
			
			float speed = rigid.velocity.magnitude;
			rigid.velocity = rigid.velocity.normalized * (Mathf.Min (speed, maxSpeed) * deccel);
			
			rigid.angularVelocity *= 0.98f;
		}
	}

	void ScatterFractions()
	{
		float volume = trans.localScale.x*trans.localScale.y*trans.localScale.z;
		int numFraction = (int)(volume * 1000.0f);
		mp.mpScatterParticlesBoxTransform(trans.localToWorldMatrix, numFraction, Vector3.zero, 3.0f);
	}

	public void Damage(float v)
	{
		life -= v;
	}

	public bool IsDead()
	{
		return life<=0.0f;
	}

	void OnDestroy()
	{
		float radius = (trans.localScale.x + trans.localScale.y + trans.localScale.z) * 0.5f;
		mp.mpAddRadialSphereForce (trans.position, radius, radius*100.0f);
	}
}
