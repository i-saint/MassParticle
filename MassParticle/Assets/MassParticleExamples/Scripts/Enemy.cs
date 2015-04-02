using UnityEngine;
using System.Collections;

public class Enemy : MonoBehaviour {

	Rigidbody rigid;
	Destroyable stat;

	// Use this for initialization
	void Start () {
		rigid = GetComponent<Rigidbody> ();
		stat = GetComponent<Destroyable> ();
		
		rigid.maxAngularVelocity = 20.0f;
	}
	
	// Update is called once per frame
	void Update () {
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
