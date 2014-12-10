using UnityEngine;
using System.Collections;

[RequireComponent(typeof(DSRenderer))]
public class DSRaymarcher : MonoBehaviour
{
	public Material matRaymarching;
	public float fovy = 1.8f;
	public float fixedRadius = 1.0f;
	public float minRadius = 0.5f;
	DSRenderer dscam;

	void Start()
	{
		dscam = GetComponent<DSRenderer>();
		dscam.AddCallbackPostGBuffer(() => { Render(); }, 100);
	}
	
	void Render()
	{
		if (!enabled || matRaymarching == null) { return; }

		matRaymarching.SetFloat("_Fovy", fovy);
		matRaymarching.SetFloat("_FixedRadius", fixedRadius);
		matRaymarching.SetFloat("_MinRadius", minRadius);
		matRaymarching.SetPass(0);
		DSRenderer.DrawFullscreenQuad();
	}
}
