using UnityEngine;
using System.Collections;

public class DSPESurfaceLight : DSEffectBase
{
    public float resolution_scale = 1.0f;
    public float intensity = 0.1f;
    public float rayAdvance = 3.0f;
    public Material matSurfaceLight;
    public Material matCombine;
    public Material matFill;
    public RenderTexture[] rtTemp;

    public override void Awake()
    {
        base.Awake();
        GetDSRenderer().AddCallbackPostLighting(() => { Render(); }, 100);
        rtTemp = new RenderTexture[2];
    }

    void UpdateRenderTargets()
    {
        Vector2 reso = GetDSRenderer().GetInternalResolution() * resolution_scale;
        if (rtTemp[0] != null && rtTemp[0].width != reso.x)
        {
            for (int i = 0; i < rtTemp.Length; ++i)
            {
                rtTemp[i].Release();
                rtTemp[i] = null;
            }
        }
        if (rtTemp[0] == null || !rtTemp[0].IsCreated())
        {
            for (int i = 0; i < rtTemp.Length; ++i)
            {
                rtTemp[i] = DSRenderer.CreateRenderTexture((int)reso.x, (int)reso.y, 0, RenderTextureFormat.ARGBHalf);
                rtTemp[i].filterMode = FilterMode.Bilinear;
            }
        }
    }

    void Render()
    {
        if (!enabled) { return; }

        UpdateRenderTargets();

        DSRenderer dsr = GetDSRenderer();
        Graphics.SetRenderTarget(rtTemp[1]);
        matFill.SetVector("_Color", new Vector4(0.0f, 0.0f, 0.0f, 0.02f));
        matFill.SetTexture("_PositionBuffer1", dsr.rtPositionBuffer);
        matFill.SetTexture("_PositionBuffer2", dsr.rtPrevPositionBuffer);
        matFill.SetPass(1);
        DSRenderer.DrawFullscreenQuad();

        Graphics.SetRenderTarget(rtTemp[0]);
        matSurfaceLight.SetFloat("_Intensity", intensity);
        matSurfaceLight.SetFloat("_RayAdvance", rayAdvance);
        matSurfaceLight.SetTexture("_NormalBuffer", dsr.rtNormalBuffer);
        matSurfaceLight.SetTexture("_PositionBuffer", dsr.rtPositionBuffer);
        matSurfaceLight.SetTexture("_ColorBuffer", dsr.rtColorBuffer);
        matSurfaceLight.SetTexture("_GlowBuffer", dsr.rtGlowBuffer);
        matSurfaceLight.SetTexture("_PrevResult", rtTemp[1]);
        matSurfaceLight.SetPass(0);
        DSRenderer.DrawFullscreenQuad();

        rtTemp[0].filterMode = FilterMode.Trilinear;
        Graphics.SetRenderTarget(dsr.rtComposite);
        matCombine.SetTexture("_MainTex", rtTemp[0]);
        matCombine.SetPass(2);
        DSRenderer.DrawFullscreenQuad();
        rtTemp[0].filterMode = FilterMode.Point;

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
