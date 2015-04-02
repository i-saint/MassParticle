using UnityEngine;
using System.Collections;
using System.Collections.Generic;

[RequireComponent(typeof(MeshFilter))]
public class DSSubtract : MonoBehaviour
{
	static HashSet<DSSubtract> _instances;
	public static HashSet<DSSubtract> instances
	{
		get
		{
			if (_instances == null) { _instances = new HashSet<DSSubtract>(); }
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
	public Material matStencilWrite;
	public Material matStencilClear;
	public Material matSubtractor;

	void Start ()
	{
		gameObject.layer = DSLogicOpRenderer.instance.layerLogicOp;
		trans = GetComponent<Transform>();
		mesh = GetComponent<MeshFilter>().mesh;
	}
}
