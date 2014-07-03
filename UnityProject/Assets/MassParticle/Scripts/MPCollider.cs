using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class MPCollider : MPColliderAttribute
{
	public enum Shape {
		Sphere,
		Capsule,
		Box,
	}

	static HashSet<MPCollider> _instances;
	public static HashSet<MPCollider> instances
	{
		get
		{
			if (_instances == null) { _instances = new HashSet<MPCollider>(); }
			return _instances;
		}
	}

	public Shape shape = Shape.Box;
	Transform trans;
	Vector4 pos1;
	Vector4 pos2;
	float radius;

	void OnEnable()
	{
		instances.Add(this);
		trans = GetComponent<Transform>();
	}

	void OnDisable()
	{
		instances.Remove(this);
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

	public void MPUpdate()
	{
		UpdateColliderProperties();
		if (sendCollision) {
			switch (shape)
			{
			case Shape.Sphere:
			{
				Vector3 pos = trans.position;
				MPAPI.mpAddSphereCollider(ref cprops, ref pos, trans.localScale.magnitude * 0.25f);
			}
				break;
			case Shape.Capsule:
			{
				UpdateCapsule();
				Vector3 pos13 = pos1;
				Vector3 pos23 = pos2;
				MPAPI.mpAddCapsuleCollider(ref cprops, ref pos13, ref pos23, radius);
			}
				break;
			case Shape.Box:
			{
				Matrix4x4 mat = trans.localToWorldMatrix;
				Vector3 one = Vector3.one;
				MPAPI.mpAddBoxCollider(ref cprops, ref mat, ref one);
			}
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
