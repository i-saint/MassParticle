using UnityEngine;
using System.Collections;

[RequireComponent(typeof(DSRenderer))]
public class DSPEGlowNormal : MonoBehaviour
{
	public Vector4 baseColor = new Vector4(0.75f, 0.75f, 1.25f, 0.0f);
	public float intensity = 1.0f;
	public float threshold = 0.5f;
	public float edge = 0.2f;
	public Material matGlowNormal;
	DSRenderer dscam;

	void Start()
	{
		dscam = GetComponent<DSRenderer>();
		dscam.AddCallbackPostEffect(() => { Render(); });

		//matGlowNormal = new Material(Shader.Find("Custom/PostEffect_GlowNormal"));
	}


	void Render()
	{
		if (!enabled) { return; }

		matGlowNormal.SetVector("_BaseColor", baseColor);
		matGlowNormal.SetFloat("_Intensity", intensity);
		matGlowNormal.SetFloat("_Threshold", threshold);
		matGlowNormal.SetFloat("_Edge", edge);
		matGlowNormal.SetTexture("_PositionBuffer", dscam.rtPositionBuffer);
		matGlowNormal.SetTexture("_NormalBuffer", dscam.rtNormalBuffer);
		matGlowNormal.SetPass(0);
		DSRenderer.DrawFullscreenQuad();
	}
}
