using UnityEngine;
using System.Collections;

public class mpCollider : MonoBehaviour {
	public enum Shape {
		Sphere,
		Box,
	}

	public Shape shape = Shape.Box;

	void Start () {
	
	}
	
	void Update () {
		switch(shape) {
			case Shape.Sphere:
				{
					mp.mpAddSphereCollider(-1, transform.position, transform.localScale.magnitude*0.25f);
				}
				break;
			case Shape.Box:
				{
					mp.mpAddBoxCollider(-1, transform.localToWorldMatrix, Vector3.one);
				}
				break;
		}
	}

	void OnDrawGizmos()
	{
		{
			Gizmos.color = Color.yellow;
			Gizmos.matrix = transform.localToWorldMatrix;
			switch (shape)
			{
				case Shape.Sphere:
					Gizmos.DrawWireSphere(Vector3.zero, 0.5f);
					break;

				case Shape.Box:
					Gizmos.color = Color.yellow;
					Gizmos.DrawWireCube(Vector3.zero, Vector3.one);
					break;
			}
			Gizmos.matrix = Matrix4x4.identity;
		}
	}

}
