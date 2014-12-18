using UnityEngine;
using System.Collections;

public class DSPEReflection : DSEffectBase
{
    public enum Algorithm
    {
        Simple = 0,
        Temporal = 1,
    }

    public Algorithm type = Algorithm.Temporal;
    public float resolution_scale = 0.5f;
    public float intensity = 0.3f;
    public float rayMarchDistance = 0.2f;
    public float rayDiffusion = 0.01f;
    public float falloffDistance = 20.0f;
    public float maxAccumulation = 50.0f;
    public RenderTexture[] rtTemp;
    public Material matReflection;
    public Material matCombine;


    public override void Awake()
    {
        base.Awake();
        GetDSRenderer().AddCallbackPostEffect(() => { Render(); }, 5000);
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
                rtTemp[i].filterMode = FilterMode.Point;
            }
        }
    }

    void Render()
    {
        if (!enabled) { return; }
        UpdateRenderTargets();

        DSRenderer dsr = GetDSRenderer();
        Graphics.SetRenderTarget(rtTemp[0]);
        //GL.Clear(false, true, Color.black);
        matReflection.SetFloat("_Intensity", intensity);
        matReflection.SetFloat("_RayMarchDistance", rayMarchDistance);
        matReflection.SetFloat("_RayDiffusion", rayDiffusion);
        matReflection.SetFloat("_FalloffDistance", falloffDistance);
        matReflection.SetFloat("_MaxAccumulation", maxAccumulation);

        matReflection.SetTexture("_FrameBuffer", dsr.rtComposite);
        matReflection.SetTexture("_PositionBuffer", dsr.rtPositionBuffer);
        matReflection.SetTexture("_PrevPositionBuffer", dsr.rtPrevPositionBuffer);
        matReflection.SetTexture("_NormalBuffer", dsr.rtNormalBuffer);
        matReflection.SetTexture("_PrevResult", rtTemp[1]);
        matReflection.SetMatrix("_ViewProjInv", dsr.viewProjInv);
        matReflection.SetMatrix("_PrevViewProj", dsr.prevViewProj);
        matReflection.SetMatrix("_PrevViewProjInv", dsr.prevViewProjInv);
        matReflection.SetPass((int)type);
        DSRenderer.DrawFullscreenQuad();

        rtTemp[0].filterMode = FilterMode.Trilinear;
        Graphics.SetRenderTarget(dsr.rtComposite);
        matCombine.SetTexture("_MainTex", rtTemp[0]);
        matCombine.SetPass(2);
        DSRenderer.DrawFullscreenQuad();
        rtTemp[0].filterMode = FilterMode.Point;

        DSRenderer.Swap(ref rtTemp[0], ref rtTemp[1]);
    }
}
