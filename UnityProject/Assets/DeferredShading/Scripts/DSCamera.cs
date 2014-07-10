using UnityEngine;
using System.Collections;
using System.Collections.Generic;

[RequireComponent(typeof(Camera))]
public class DSCamera : MonoBehaviour
{
	public enum TextureFormat
	{
		Half,
		Float,
	}

	public bool showBuffers = false;
	public bool voronoiGlowline = true;
	public bool normalGlow = true;
	public bool reflection = true;
	public bool bloom = true;
	public TextureFormat textureFormat = TextureFormat.Half;
	public Material matGBufferClear;
	public Material matPointLight;
	public Material matDirectionalLight;
	public Material matGlowLine;
	public Material matGlowNormal;
	public Material matReflection;
	public Material matBloomLuminance;
	public Material matBloomHBlur;
	public Material matBloomVBlur;
	public Material matBloom;
	public GameObject sphereMeshObject;

	public RenderTexture[] mrtTex;
	RenderBuffer[] mrtRB4;
	RenderBuffer[] mrtRB2;
	public RenderTexture[] rtComposite;
	public RenderTexture[] rtBloomH;
	public RenderTexture[] rtBloomQ;
	Camera cam;


	RenderTexture CreateRenderTexture(int w, int h, int d, RenderTextureFormat f)
	{
		RenderTexture r = new RenderTexture(w, h, d, f);
		r.filterMode = FilterMode.Point;
		r.useMipMap = false;
		r.generateMips = false;
		return r;
	}

	void Start ()
	{
		mrtTex = new RenderTexture[4];
		mrtRB4 = new RenderBuffer[4];
		mrtRB2 = new RenderBuffer[2];
		rtComposite = new RenderTexture[2];
		rtBloomH = new RenderTexture[2];
		rtBloomQ = new RenderTexture[2];
		cam = GetComponent<Camera>();

		RenderTextureFormat format = textureFormat == TextureFormat.Half ? RenderTextureFormat.ARGBHalf : RenderTextureFormat.ARGBFloat;
		for (int i = 0; i < mrtTex.Length; ++i )
		{
			int depthbits = i == 0 ? 32 : 0;
			mrtTex[i] = CreateRenderTexture((int)cam.pixelWidth, (int)cam.pixelHeight, depthbits, format);
			mrtRB4[i] = mrtTex[i].colorBuffer;
		}
		for (int i = 0; i < rtComposite.Length; ++i)
		{
			rtComposite[i] = CreateRenderTexture((int)cam.pixelWidth, (int)cam.pixelHeight, 0, format);
			rtBloomH[i] = CreateRenderTexture(256, 512 / 2, 0, format);
			rtBloomH[i].filterMode = FilterMode.Bilinear;
			rtBloomQ[i] = CreateRenderTexture(128, 256, 0, format);
			rtBloomQ[i].filterMode = FilterMode.Bilinear;
		}
		matPointLight.SetTexture("_NormalBuffer", mrtTex[0]);
		matPointLight.SetTexture("_PositionBuffer", mrtTex[1]);
		matPointLight.SetTexture("_ColorBuffer", mrtTex[2]);
		matPointLight.SetTexture("_GlowBuffer", mrtTex[3]);
		matDirectionalLight.SetTexture("_NormalBuffer", mrtTex[0]);
		matDirectionalLight.SetTexture("_PositionBuffer", mrtTex[1]);
		matDirectionalLight.SetTexture("_ColorBuffer", mrtTex[2]);
		matDirectionalLight.SetTexture("_GlowBuffer", mrtTex[3]);

		matGlowLine.SetTexture("_PositionBuffer", mrtTex[1]);
		matGlowLine.SetTexture("_NormalBuffer", mrtTex[0]);
		matGlowNormal.SetTexture("_PositionBuffer", mrtTex[1]);
		matGlowNormal.SetTexture("_NormalBuffer", mrtTex[0]);
		matReflection.SetTexture("_FrameBuffer", rtComposite[0]);
		matReflection.SetTexture("_PositionBuffer", mrtTex[1]);
		matReflection.SetTexture("_NormalBuffer", mrtTex[0]);
		matBloom.SetTexture("_FrameBuffer", rtComposite[0]);
		matBloom.SetTexture("_GlowBuffer", mrtTex[3]);
	}
	
	void Update ()
	{
	
	}

	void OnPreRender()
	{
		Graphics.SetRenderTarget(mrtRB4, mrtTex[0].depthBuffer);
		matGBufferClear.SetPass(0);
		DrawFullscreenQuad();
	}

	void OnPostRender()
	{
		Graphics.SetRenderTarget(rtComposite[0]);
		GL.Clear(true, true, Color.black);
		Graphics.SetRenderTarget(rtComposite[0].colorBuffer, mrtTex[0].depthBuffer);

		DSLight.sphereMesh = sphereMeshObject.GetComponent<MeshFilter>().mesh;
		DSLight.matPointLight = matPointLight;
		DSLight.matDirectionalLight = matDirectionalLight;
		DSLight.RenderLights(this);

		if (voronoiGlowline)
		{
			mrtRB2[0] = rtComposite[0].colorBuffer;
			mrtRB2[1] = mrtTex[3].colorBuffer;
			Graphics.SetRenderTarget(mrtRB2, mrtTex[0].depthBuffer);
			matGlowLine.SetPass(0);
			DrawFullscreenQuad();
			Graphics.SetRenderTarget(rtComposite[0]);
		}
		if(normalGlow) {
			matGlowNormal.SetPass(0);
			DrawFullscreenQuad();
		}

		if (bloom)
		{
			Vector4 hscreen = new Vector4(rtBloomH[0].width, rtBloomH[0].height, 1.0f / rtBloomH[0].width, 1.0f / rtBloomH[0].height);
			Vector4 qscreen = new Vector4(rtBloomQ[0].width, rtBloomQ[0].height, 1.0f / rtBloomQ[0].width, 1.0f / rtBloomQ[0].height);
			matBloomHBlur.SetVector("_Screen", hscreen);
			matBloomVBlur.SetVector("_Screen", hscreen);

			Graphics.SetRenderTarget(rtBloomH[0]);
			matBloomHBlur.SetTexture("_GlowBuffer", mrtTex[3]);
			matBloomHBlur.SetPass(0);
			DrawFullscreenQuad();
			Graphics.SetRenderTarget(rtBloomH[1]);
			matBloomHBlur.SetTexture("_GlowBuffer", rtBloomH[0]);
			matBloomHBlur.SetPass(0);
			DrawFullscreenQuad();
			Graphics.SetRenderTarget(rtBloomH[0]);
			matBloomHBlur.SetTexture("_GlowBuffer", rtBloomH[1]);
			matBloomHBlur.SetPass(0);
			DrawFullscreenQuad();
			Graphics.SetRenderTarget(rtBloomH[1]);
			matBloomVBlur.SetTexture("_GlowBuffer", rtBloomH[0]);
			matBloomVBlur.SetPass(0);
			DrawFullscreenQuad();

			matBloomHBlur.SetVector("_Screen", qscreen);
			matBloomVBlur.SetVector("_Screen", qscreen);
			Graphics.SetRenderTarget(rtBloomQ[0]);
			matBloomHBlur.SetTexture("_GlowBuffer", mrtTex[3]);
			matBloomHBlur.SetPass(0);
			DrawFullscreenQuad();
			Graphics.SetRenderTarget(rtBloomQ[1]);
			matBloomHBlur.SetTexture("_GlowBuffer", rtBloomQ[0]);
			matBloomHBlur.SetPass(0);
			DrawFullscreenQuad();
			Graphics.SetRenderTarget(rtBloomQ[0]);
			matBloomHBlur.SetTexture("_GlowBuffer", rtBloomQ[1]);
			matBloomHBlur.SetPass(0);
			DrawFullscreenQuad();
			Graphics.SetRenderTarget(rtBloomQ[1]);
			matBloomVBlur.SetTexture("_GlowBuffer", rtBloomQ[0]);
			matBloomVBlur.SetPass(0);
			DrawFullscreenQuad();

			Graphics.SetRenderTarget(rtComposite[0]);
			matBloom.SetTexture("_GlowBuffer", mrtTex[3]);
			matBloom.SetTexture("_HalfGlowBuffer", rtBloomH[1]);
			matBloom.SetTexture("_QuarterGlowBuffer", rtBloomQ[1]);
			matBloom.SetPass(0);
			DrawFullscreenQuad();
		}
		Graphics.SetRenderTarget(null);
		GL.Clear(false, true, Color.black);
		matReflection.SetPass(0);
		DrawFullscreenQuad();


		Graphics.SetRenderTarget(null);
	}

	void OnGUI()
	{
		if (!showBuffers) { return; }

		Vector2 size = new Vector2(mrtTex[0].width, mrtTex[0].height) / 6.0f;
		float y = 5.0f;
		for (int i = 0; i < 4; ++i )
		{
			GUI.DrawTexture(new Rect(5, y, size.x, size.y), mrtTex[i], ScaleMode.ScaleToFit, false);
			y += size.y + 5.0f;
		}
		GUI.DrawTexture(new Rect(5, y, size.x, size.y), rtComposite[0], ScaleMode.ScaleToFit, false);
		y += size.y + 5.0f;
	}

	static public void DrawFullscreenQuad(float z=1.0f)
	{
		GL.Begin(GL.QUADS);
		GL.Vertex3(-1.0f, -1.0f, z);
		GL.Vertex3(1.0f, -1.0f, z);
		GL.Vertex3(1.0f, 1.0f, z);
		GL.Vertex3(-1.0f, 1.0f, z);

		GL.Vertex3(-1.0f, 1.0f, z);
		GL.Vertex3(1.0f, 1.0f, z);
		GL.Vertex3(1.0f, -1.0f, z);
		GL.Vertex3(-1.0f, -1.0f, z);
		GL.End();
	}
}
