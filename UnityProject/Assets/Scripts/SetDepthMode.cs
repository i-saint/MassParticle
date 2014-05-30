using UnityEngine;
using System.Collections;

public class SetDepthMode : MonoBehaviour {

	// Use this for initialization
	void Start () {		
		camera.depthTextureMode = DepthTextureMode.Depth;
	}
	
	// Update is called once per frame
	void Update () {
	
	}
}
