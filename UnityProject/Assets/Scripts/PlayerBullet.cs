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
		if(Mathf.Abs (transform.position.x)>10.0f ||
		   Mathf.Abs (transform.position.z)>10.0f )
		{
			Destroy (gameObject);
		}
	}
	
	
	void OnCollisionEnter(Collision col)
	{
		excDestroyable destroyable = col.collider.GetComponent<excDestroyable>();
		if(destroyable) {
			destroyable.Damage(30.0f);
		}
		Destroy (gameObject);
	}
}
