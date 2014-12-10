using UnityEngine;
using System.Collections;

[RequireComponent(typeof(DSRenderer))]
public class DSPEGlowline : MonoBehaviour
{
	public enum SpreadPattern
	{
		Radial = 0,
		Voronoi = 1,
	}
	public enum GridPattern
	{
		Square = 0,
		Hexagon = 1,
		BoxCell = 2,
	}

	public GridPattern gridPattern = GridPattern.Square;
	public SpreadPattern spreadPattern = SpreadPattern.Radial;
	public float intensity = 1.0f;
	public Vector4 baseColor = new Vector4(0.45f, 0.4f, 2.0f, 0.0f);
	public Material matGlowLine;
	RenderBuffer[] rbBuffers;
	DSRenderer dscam;


	void Start()
	{
		dscam = GetComponent<DSRenderer>();
		dscam.AddCallbackPostGBuffer(() => { Render(); }, 100);

		//matGlowLine = new Material(Shader.Find("Custom/PostEffect_Glowline"));
	}

	void Render()
	{
		if (!enabled) { return; }
		if (rbBuffers==null)
		{
			rbBuffers = new RenderBuffer[2];
		}
		rbBuffers[0] = dscam.rtGlowBuffer.colorBuffer;
		rbBuffers[1] = dscam.rtColorBuffer.colorBuffer;

		Graphics.SetRenderTarget(rbBuffers, dscam.rtNormalBuffer.depthBuffer);
		matGlowLine.SetTexture("_PositionBuffer", dscam.rtPositionBuffer);
		matGlowLine.SetTexture("_NormalBuffer", dscam.rtNormalBuffer);
		matGlowLine.SetFloat("_Intensity", intensity);
		matGlowLine.SetVector("_BaseColor", baseColor);
		matGlowLine.SetInt("_GridPattern", (int)gridPattern);
		matGlowLine.SetInt("_SpreadPattern", (int)spreadPattern);
		matGlowLine.SetPass(0);
		DSRenderer.DrawFullscreenQuad();
		Graphics.SetRenderTarget(dscam.rtComposite);
	}
}
