using UnityEngine;
using System.Collections;

public class mpCollider : mpColliderAttribute
{
	public enum Shape {
		Sphere,
		Capsule,
		Box,
	}

	public Shape shape = Shape.Box;
	Transform trans;
	Vector4 pos1;
	Vector4 pos2;
	float radius;

	void Start () {
		trans = GetComponent<Transform>();
	}
	
	void UpdateCapsule()
	{
		radius = (trans.localScale.x + trans.localScale.z) * 0.5f * 0.5f;
		float h = Mathf.Max(0.0f, trans.localScale.y - radius * 2.0f);
		pos1.Set(0.0f, h * 0.5f, 0.0f, 1.0f);
		pos2.Set(0.0f, -h * 0.5f, 0.0f, 1.0f);
		pos1 = trans.localToWorldMatrix * pos1;
		pos2 = trans.localToWorldMatrix * pos2;
	}

	void Update () {
		if (sendCollision) {
			switch (shape)
			{
			case Shape.Sphere:
					mp.mpAddSphereCollider(-1, trans.position, trans.localScale.magnitude * 0.25f);
				break;
			case Shape.Capsule:
				{
					UpdateCapsule();
					mp.mpAddCapsuleCollider(-1, pos1, pos2, radius);
				}
				break;
			case Shape.Box:
				mp.mpAddBoxCollider(-1, trans.localToWorldMatrix, Vector3.one);
				break;
			}
		}
	}

	void OnDrawGizmos()
	{
		trans = GetComponent<Transform>();
		Gizmos.color = Color.yellow;
		switch (shape)
		{
			case Shape.Sphere:
				Gizmos.matrix = trans.localToWorldMatrix;
				Gizmos.DrawWireSphere(Vector3.zero, 0.5f);
				break;

			case Shape.Capsule:
				UpdateCapsule();
				Gizmos.DrawWireSphere(pos1, radius);
				Gizmos.DrawWireSphere(pos2, radius);
				Gizmos.DrawLine(pos1, pos2);
				break;

			case Shape.Box:
				Gizmos.matrix = trans.localToWorldMatrix;
				Gizmos.DrawWireCube(Vector3.zero, Vector3.one);
				break;
		}
		Gizmos.matrix = Matrix4x4.identity;
	}

}
