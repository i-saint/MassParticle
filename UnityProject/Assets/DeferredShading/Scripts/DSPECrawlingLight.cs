using UnityEngine;
using System.Collections;

[RequireComponent(typeof(DSRenderer))]
public class DSPECrawlingLight : MonoBehaviour
{
    public bool halfResolution = false;
    public float rayAdvance = 1.0f;
    public Material matSurfaceLight;
    public Material matCombine;
    public Material matFill;
    public RenderTexture[] rtTemp;
    DSRenderer dscam;


    void Start()
    {
        rtTemp = new RenderTexture[2];
        dscam = GetComponent<DSRenderer>();
        dscam.AddCallbackPostGBuffer(() => { Render(); }, 1100);

        //cbParams = new ComputeBuffer(1, Params.size);
    }

    void OnDisable()
    {
        //cbParams.Release();
    }

    void Render()
    {
        if (!enabled) { return; }

        Camera cam = GetComponent<Camera>();
        Vector2 reso = dscam.GetRenderResolution();
        if (rtTemp[0] == null)
        {
            int div = halfResolution ? 2 : 1;
            for (int i = 0; i < rtTemp.Length; ++i )
            {
                rtTemp[i] = DSRenderer.CreateRenderTexture((int)reso.x / div, (int)reso.y / div, 0, RenderTextureFormat.ARGBHalf);
                rtTemp[i].filterMode = FilterMode.Bilinear;
            }
        }
        Graphics.SetRenderTarget(rtTemp[1]);
        matFill.SetVector("_Color", new Vector4(0.0f, 0.0f, 0.0f, 0.02f));
        matFill.SetTexture("_PositionBuffer1", dscam.rtPositionBuffer);
        matFill.SetTexture("_PositionBuffer2", dscam.rtPrevPositionBuffer);
        matFill.SetPass(1);
        DSRenderer.DrawFullscreenQuad();

        Graphics.SetRenderTarget(rtTemp[0]);
        matSurfaceLight.SetFloat("_RayAdvance", rayAdvance);
        matSurfaceLight.SetTexture("_NormalBuffer", dscam.rtNormalBuffer);
        matSurfaceLight.SetTexture("_PositionBuffer", dscam.rtPositionBuffer);
        matSurfaceLight.SetTexture("_ColorBuffer", dscam.rtColorBuffer);
        matSurfaceLight.SetTexture("_GlowBuffer", dscam.rtGlowBuffer);
        matSurfaceLight.SetTexture("_GlowBufferB", dscam.rtPrevGlowBuffer);
        matSurfaceLight.SetTexture("_PrevResult", rtTemp[1]);
        matSurfaceLight.SetPass(0);
        DSRenderer.DrawFullscreenQuad();

        Graphics.SetRenderTarget(dscam.rtGlowBuffer);
        matCombine.SetTexture("_MainTex", rtTemp[0]);
        matCombine.SetVector("_PixelSize", new Vector4(1.0f / rtTemp[0].width, 1.0f / rtTemp[0].height, 0.0f, 0.0f));
        matCombine.SetPass(3);
        DSRenderer.DrawFullscreenQuad();

        Swap(ref rtTemp[0], ref rtTemp[1]);
    }

    public static void Swap<T>(ref T lhs, ref T rhs)
    {
        T temp;
        temp = lhs;
        lhs = rhs;
        rhs = temp;
    }
}
