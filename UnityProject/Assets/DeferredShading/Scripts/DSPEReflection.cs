using UnityEngine;
using System.Collections;

[RequireComponent(typeof(DSRenderer))]
public class DSPEReflection : MonoBehaviour
{
    public enum Type
    {
        Simple = 0,
        Temporal = 1,
    }

    public bool halfResolution = true;
    public Type type = Type.Simple;
    public float intensity = 0.3f;
    public float rayMarchDistance = 0.2f;
    public float rayDiffusion = 0.01f;
    public float falloffDistance = 20.0f;
    public RenderTexture[] rtTemp;
    public Material matReflection;
    public Material matCombine;
    DSRenderer dscam;


    void Start()
    {
        dscam = GetComponent<DSRenderer>();
        dscam.AddCallbackPostEffect(() => { Render(); }, 5000);
    }

    void Render()
    {
        if (!enabled) { return; }
        if (rtTemp == null || rtTemp.Length == 0)
        {
            rtTemp = new RenderTexture[2];
            int div = halfResolution ? 2 : 1;
            Camera cam = GetComponent<Camera>();
            Vector2 reso = dscam.GetRenderResolution();
            for (int i = 0; i < rtTemp.Length; ++i)
            {
                rtTemp[i] = DSRenderer.CreateRenderTexture((int)reso.x / div, (int)reso.y / div, 0, RenderTextureFormat.ARGBHalf);
                rtTemp[i].filterMode = FilterMode.Point;
            }
        }

        Graphics.SetRenderTarget(rtTemp[0]);
        //GL.Clear(false, true, Color.black);
        matReflection.SetFloat("_Intensity", intensity);
        matReflection.SetFloat("_RayMarchDistance", rayMarchDistance);
        matReflection.SetFloat("_RayDiffusion", rayDiffusion);
        matReflection.SetFloat("_FalloffDistance", falloffDistance);

        matReflection.SetTexture("_FrameBuffer", dscam.rtComposite);
        matReflection.SetTexture("_PositionBuffer", dscam.rtPositionBuffer);
        matReflection.SetTexture("_PrevPositionBuffer", dscam.rtPrevPositionBuffer);
        matReflection.SetTexture("_NormalBuffer", dscam.rtNormalBuffer);
        matReflection.SetTexture("_PrevResult", rtTemp[1]);
        matReflection.SetMatrix("_ViewProjInv", dscam.viewProjInv);
        matReflection.SetMatrix("_PrevViewProj", dscam.prevViewProj);
        matReflection.SetMatrix("_PrevViewProjInv", dscam.prevViewProjInv);
        matReflection.SetPass((int)type);
        DSRenderer.DrawFullscreenQuad();

        rtTemp[0].filterMode = FilterMode.Trilinear;
        Graphics.SetRenderTarget(dscam.rtComposite);
        matCombine.SetTexture("_MainTex", rtTemp[0]);
        matCombine.SetPass(2);
        DSRenderer.DrawFullscreenQuad();
        rtTemp[0].filterMode = FilterMode.Point;

        DSRenderer.Swap(ref rtTemp[0], ref rtTemp[1]);
    }
}
