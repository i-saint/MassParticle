using UnityEngine;
using System.Collections;
using System.Collections.Generic;

[RequireComponent(typeof(MeshFilter))]
public class DSLogicOpReceiver : MonoBehaviour
{
	static HashSet<DSLogicOpReceiver> _instances;
	public static HashSet<DSLogicOpReceiver> instances
	{
		get
		{
			if (_instances == null) { _instances = new HashSet<DSLogicOpReceiver>(); }
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
	public Material matGBuffer;
	public Material matReverseDepth;
	public Material matDepthClear;


	void Start ()
	{
		gameObject.layer = DSLogicOpRenderer.instance.layerLogicOp;
		trans = GetComponent<Transform>();
		mesh = GetComponent<MeshFilter>().mesh;
		matGBuffer = GetComponent<MeshRenderer>().material;
	}
}
