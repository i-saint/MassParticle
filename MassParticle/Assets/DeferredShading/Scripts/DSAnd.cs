using UnityEngine;
using System.Collections;
using System.Collections.Generic;

[RequireComponent(typeof(MeshFilter))]
public class DSAnd : MonoBehaviour
{
	static HashSet<DSAnd> _instances;
	public static HashSet<DSAnd> instances
	{
		get
		{
			if (_instances == null) { _instances = new HashSet<DSAnd>(); }
			return _instances;
		}
	}

	void OnEnable()
	{
		instances.Add(this);
	}

	void OnDisable()
	{
		instances.Remove(this);
	}


	public Transform trans;
	public Mesh mesh;
	public Material matReverseDepth;
	public Material matGBuffer;

	void Start ()
	{
		gameObject.layer = DSLogicOpRenderer.instance.layerLogicOp;
		trans = GetComponent<Transform>();
		mesh = GetComponent<MeshFilter>().mesh;
	}
}
