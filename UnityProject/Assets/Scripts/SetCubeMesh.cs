using UnityEngine;
using System.Collections;

public class SetCubeMesh : MonoBehaviour {

	// Use this for initialization
	void Start ()
	{
#if MP_PURE_CSHARP
		mpCS.mpSystem.instance.mesh = GetComponent<MeshFilter>().mesh;
#endif // MP_PURE_CSHARP
	}
	
	// Update is called once per frame
	void Update () {
	
	}
}
