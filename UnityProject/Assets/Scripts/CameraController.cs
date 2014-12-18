using UnityEngine;
using System.Collections;

public class CameraController : MonoBehaviour {

	public GameObject cube;


	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		Vector3 move = new Vector3 ();
		move.x = Input.GetAxisRaw ("Horizontal");
		move.z = Input.GetAxisRaw ("Vertical");
		move *= 0.1f;

		if (Input.GetButtonDown("Fire1"))
		{
			Instantiate(cube, new Vector3(Random.Range(-2.0f, 2.0f), 5.0f, Random.Range(-2.0f, 2.0f)), new Quaternion());
		}

		Collider[] colliders = Physics.OverlapSphere(transform.position, 10.0f);
		for (int i = 0; i < colliders.Length; ++i)
		{
			Collider col = colliders[i];
			col.transform.position += move;
		}
	}
}

