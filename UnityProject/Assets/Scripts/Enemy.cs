using UnityEngine;
using System.Collections;

public class Enemy : MonoBehaviour {

	public float life = 5.0f;

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}

	void OnTriggerEnter(Collider col)
	{
		Destroy (col.gameObject);
		life -= 1.0f;
		if(life<=1.0f) {
			float volume = transform.localScale.x*transform.localScale.y*transform.localScale.z;
			int numFraction = (int)(volume * 500.0f);
			mpEmitter.mpScatterParticlesBoxTransform(transform.localToWorldMatrix, numFraction, Vector3.zero, 3.0f);
			Destroy (gameObject);
		}
	}
}
