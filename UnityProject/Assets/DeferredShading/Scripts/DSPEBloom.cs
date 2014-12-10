using UnityEngine;
using System.Collections;

[RequireComponent(typeof(DSRenderer))]
public class DSPEBloom : MonoBehaviour
{
	public float intensity = 1.5f;
	public Material matBloomLuminance;
	public Material matBloomBlur;
	public Material matBloom;
	public RenderTexture[] rtBloomH;
	public RenderTexture[] rtBloomQ;
	DSRenderer dscam;

	void Start()
	{
		dscam = GetComponent<DSRenderer>();
		dscam.AddCallbackPostEffect(() => { Render(); }, 2000);

		rtBloomH = new RenderTexture[2];
		rtBloomQ = new RenderTexture[2];
		for (int i = 0; i < 2; ++i)
		{
			rtBloomH[i] = DSRenderer.CreateRenderTexture(256, 256, 0, RenderTextureFormat.ARGBHalf);
			rtBloomH[i].filterMode = FilterMode.Bilinear;
			rtBloomQ[i] = DSRenderer.CreateRenderTexture(128, 128, 0, RenderTextureFormat.ARGBHalf);
			rtBloomQ[i].filterMode = FilterMode.Bilinear;
		}

		//matBloomLuminance = new Material(Shader.Find("Custom/PostEffect_BloomLuminance"));
		//matBloomBlur = new Material(Shader.Find("Custom/PostEffect_BloomBlur"));
		//matBloom = new Material(Shader.Find("Custom/PostEffect_Bloom"));
	}
	
	void Render()
	{
		if (!enabled) { return; }

		Vector4 hscreen = new Vector4(rtBloomH[0].width, rtBloomH[0].height, 1.0f / rtBloomH[0].width, 1.0f / rtBloomH[0].height);
		Vector4 qscreen = new Vector4(rtBloomQ[0].width, rtBloomQ[0].height, 1.0f / rtBloomQ[0].width, 1.0f / rtBloomQ[0].height);
		matBloomBlur.SetVector("_Screen", hscreen);

		Graphics.SetRenderTarget(rtBloomH[0]);
		matBloomBlur.SetTexture("_GlowBuffer", dscam.rtGlowBuffer);
		matBloomBlur.SetPass(0);
		DSRenderer.DrawFullscreenQuad();
		Graphics.SetRenderTarget(rtBloomH[1]);
		matBloomBlur.SetTexture("_GlowBuffer", rtBloomH[0]);
		matBloomBlur.SetPass(0);
		DSRenderer.DrawFullscreenQuad();
		Graphics.SetRenderTarget(rtBloomH[0]);
		matBloomBlur.SetTexture("_GlowBuffer", rtBloomH[1]);
		matBloomBlur.SetPass(0);
		DSRenderer.DrawFullscreenQuad();
		Graphics.SetRenderTarget(rtBloomH[1]);
		matBloomBlur.SetTexture("_GlowBuffer", rtBloomH[0]);
		matBloomBlur.SetPass(1);
		DSRenderer.DrawFullscreenQuad();

		matBloomBlur.SetVector("_Screen", qscreen);
		Graphics.SetRenderTarget(rtBloomQ[0]);
		matBloomBlur.SetTexture("_GlowBuffer", dscam.rtGlowBuffer);
		matBloomBlur.SetPass(0);
		DSRenderer.DrawFullscreenQuad();
		Graphics.SetRenderTarget(rtBloomQ[1]);
		matBloomBlur.SetTexture("_GlowBuffer", rtBloomQ[0]);
		matBloomBlur.SetPass(0);
		DSRenderer.DrawFullscreenQuad();
		Graphics.SetRenderTarget(rtBloomQ[0]);
		matBloomBlur.SetTexture("_GlowBuffer", rtBloomQ[1]);
		matBloomBlur.SetPass(0);
		DSRenderer.DrawFullscreenQuad();
		Graphics.SetRenderTarget(rtBloomQ[1]);
		matBloomBlur.SetTexture("_GlowBuffer", rtBloomQ[0]);
		matBloomBlur.SetPass(1);
		DSRenderer.DrawFullscreenQuad();

		Graphics.SetRenderTarget(dscam.rtComposite);
		matBloom.SetTexture("_GlowBuffer", dscam.rtGlowBuffer);
		matBloom.SetTexture("_HalfGlowBuffer", rtBloomH[1]);
		matBloom.SetTexture("_QuarterGlowBuffer", rtBloomQ[1]);
		matBloom.SetFloat("_Intensity", intensity);
		matBloom.SetPass(0);
		DSRenderer.DrawFullscreenQuad();
	}
}
