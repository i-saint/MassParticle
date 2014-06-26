using UnityEngine;
using System.Collections;

public class MPColliderAttribute : MonoBehaviour {

	public bool sendCollision = true;
	public bool receiveCollision = false;
	public float stiffness = 1500.0f;
	public float rebound = 1.0f;
	public float lifetimeOnHit = 0.0f;

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}
}
