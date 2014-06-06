using UnityEngine;
using System.Collections;

public class excDestroyable : MonoBehaviour {

	public float life = 100.0f;

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		if(life<=0.0f) {
			float volume = transform.localScale.x*transform.localScale.y*transform.localScale.z;
			int numFraction = (int)(volume * 500.0f);
			mp.mpScatterParticlesBoxTransform(transform.localToWorldMatrix, numFraction, Vector3.zero, 3.0f);
			Destroy (gameObject);
		}
		if(Mathf.Abs (transform.position.x)>15.0f ||
		   Mathf.Abs (transform.position.z)>15.0f )
		{
			Destroy (gameObject);
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
