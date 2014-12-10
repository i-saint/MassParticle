using UnityEngine;
using System.Collections;

public class DSMotionTrail : MonoBehaviour
{
	public Shader shMotionTrail;
	public int delayFrame = 4;
	public int materialSlot = 0;
	Material matMotionTrail;
	Matrix4x4[] prevObjToWorld;

	void Start()
	{
		Debug.Log("DSMotionTrail");
		matMotionTrail = new Material(shMotionTrail);

		Material[] materials = renderer.materials;
		materials[materialSlot] = matMotionTrail;
		renderer.materials = materials;
	}

	void Update()
	{
		delayFrame = Mathf.Max(delayFrame, 1);
		if (prevObjToWorld == null || prevObjToWorld.Length != delayFrame)
		{
			prevObjToWorld = new Matrix4x4[delayFrame];
			for (int i = 0; i < prevObjToWorld.Length; ++i)
			{
				prevObjToWorld[i] = transform.localToWorldMatrix;
			}
		}
	}

	void OnWillRenderObject()
	{
		int last = prevObjToWorld.Length-1;
		for (int i = last; i > 0; --i)
		{
			prevObjToWorld[i] = prevObjToWorld[i - 1];
		}
		matMotionTrail.SetMatrix("prev_Object2World", prevObjToWorld[last]);
		prevObjToWorld[0] = transform.localToWorldMatrix;
	}
}
