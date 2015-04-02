using UnityEngine;
using System.Collections;

public class Gear : MonoBehaviour {

	public float maxRotationSpeed = 2.0f;
	public float maxRotation = 36000.0f;
	public float minRotation = -36000.0f;
	public float deccel = 0.99f;
	public float retraction = 0.02f;

	Rigidbody rigid;
	Transform trans;

	// Use this for initialization
	void Start () {
		rigid = GetComponent<Rigidbody>();
		trans = GetComponent<Transform>();
	}
	
	// Update is called once per frame
	void Update () {
		if (!rigid) { return; }

		Vector3 av = rigid.angularVelocity;
		float s = Mathf.Sign (av.z);
		av.z = (Mathf.Min(Mathf.Abs(av.z), maxRotationSpeed) * s - retraction*s) * deccel;
		rigid.angularVelocity = av;

		Quaternion rot = trans.rotation;
		float angle;
		Vector3 axis;
		rot.ToAngleAxis(out angle, out axis);
		angle = Mathf.Clamp(angle, minRotation, maxRotation);
		trans.rotation = Quaternion.AngleAxis(angle, axis);
	}
}
