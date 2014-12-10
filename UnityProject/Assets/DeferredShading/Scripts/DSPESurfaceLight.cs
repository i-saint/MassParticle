using UnityEngine;
using System.Collections;

[RequireComponent(typeof(DSRenderer))]
public class DSPESurfaceLight : MonoBehaviour
{
    //struct Params
    //{
    //	public const int size = 72;

    //	public Matrix4x4 vp;
    //	public float intensity;
    //	public float rayadvance;
    //};
    //public ComputeShader csSurfaceLight;
    //ComputeBuffer cbParams;
    //Params[] tmpParams = new Params[1];

    public bool halfResolution = false;
    public float intensity = 1.0f;
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
        dscam.AddCallbackPostLighting(() => { Render(); }, 100);

        //cbParams = new ComputeBuffer(1, Params.size);
    }

    void OnDisable()
    {
        //cbParams.Release();
    }

    void Render()
    {
        if (!enabled) { return; }

        //int kernel = csSurfaceLight.FindKernel("SurfaceLight");
        //tmpParams[0].intensity = intensity;
        //tmpParams[0].rayadvance = rayAdvance;

        //Matrix4x4 view = dscam.cam.worldToCameraMatrix;
        //Matrix4x4 proj = dscam.cam.projectionMatrix;
        //proj[2, 0] = proj[2, 0] * 0.5f + proj[3, 0] * 0.5f;
        //proj[2, 1] = proj[2, 1] * 0.5f + proj[3, 1] * 0.5f;
        //proj[2, 2] = proj[2, 2] * 0.5f + proj[3, 2] * 0.5f;
        //proj[2, 3] = proj[2, 3] * 0.5f + proj[3, 3] * 0.5f;
        //tmpParams[0].vp = proj * view;

        //cbParams.SetData(tmpParams);
        //csSurfaceLight.SetBuffer(kernel, "_Inputs", cbParams);
        //csSurfaceLight.SetTexture(kernel, "_NormalBuffer", dscam.rtNormalBuffer);
        //csSurfaceLight.SetTexture(kernel, "_PositionBuffer", dscam.rtPositionBuffer);
        //csSurfaceLight.SetTexture(kernel, "_ColorBuffer", dscam.rtColorBuffer);
        //csSurfaceLight.SetTexture(kernel, "_GlowBuffer", dscam.rtGlowBuffer);
        //csSurfaceLight.SetTexture(kernel, "_FrameBuffer", dscam.rtComposite);
        //csSurfaceLight.SetTexture(kernel, "_FrameBufferRW", dscam.rtComposite);
        //csSurfaceLight.Dispatch(kernel, 16, 16, 1);

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
        matSurfaceLight.SetFloat("_Intensity", intensity);
        matSurfaceLight.SetFloat("_RayAdvance", rayAdvance);
        matSurfaceLight.SetTexture("_NormalBuffer", dscam.rtNormalBuffer);
        matSurfaceLight.SetTexture("_PositionBuffer", dscam.rtPositionBuffer);
        matSurfaceLight.SetTexture("_ColorBuffer", dscam.rtColorBuffer);
        matSurfaceLight.SetTexture("_GlowBuffer", dscam.rtGlowBuffer);
        matSurfaceLight.SetTexture("_PrevResult", rtTemp[1]);
        matSurfaceLight.SetPass(0);
        DSRenderer.DrawFullscreenQuad();

        Graphics.SetRenderTarget(dscam.rtComposite);
        matCombine.SetTexture("_MainTex", rtTemp[0]);
        matCombine.SetVector("_PixelSize", new Vector4(1.0f / rtTemp[0].width, 1.0f / rtTemp[0].height, 0.0f, 0.0f));
        matCombine.SetPass(2);
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
