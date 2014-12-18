using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Linq;


//[ExecuteInEditMode]
[RequireComponent(typeof(Camera))]
public class DSRenderer : MonoBehaviour
{
    public enum RenderFormat
    {
        float16,
        float32,
    }

    public delegate void Callback();
    public struct PriorityCallback
    {
        public int priority;
        public Callback callback;

        public PriorityCallback(Callback cb, int p)
        {
            priority = p;
            callback = cb;
        }
    }

    public class PriorityCallbackComp : IComparer<PriorityCallback>
    {
        public int Compare(PriorityCallback a, PriorityCallback b)
        {
            return a.priority.CompareTo(b.priority);
        }
    }

    public float resolution_scale = 1.0f;
    public bool showBuffers = false;
    public RenderFormat textureFormat = RenderFormat.float16;
    public Material matFill;
    public Material matGBufferClear;
    public Material matCombine;
    public Mesh dummy_mesh;

    public Matrix4x4 prevViewProj;
    public Matrix4x4 prevViewProjInv;
    public Matrix4x4 viewProj;
    public Matrix4x4 viewProjInv;

    public RenderTexture[] rtGBuffer;
    public RenderTexture[] rtPrevGBuffer;
    public RenderTexture rtNormalBuffer         { get { return rtGBuffer[0]; } }
    public RenderTexture rtPositionBuffer       { get { return rtGBuffer[1]; } }
    public RenderTexture rtColorBuffer          { get { return rtGBuffer[2]; } }
    public RenderTexture rtGlowBuffer           { get { return rtGBuffer[3]; } }
    public RenderTexture rtPrevNormalBuffer     { get { return rtPrevGBuffer[0]; } }
    public RenderTexture rtPrevPositionBuffer   { get { return rtPrevGBuffer[1]; } }
    public RenderTexture rtPrevColorBuffer      { get { return rtPrevGBuffer[2]; } }
    public RenderTexture rtPrevGlowBuffer       { get { return rtPrevGBuffer[3]; } }

    public RenderBuffer[] rbGBuffer;
    public RenderTexture rtComposite;
    public RenderTexture rtCompositeShadow;
    public Camera cam;

    List<PriorityCallback> cbPreGBuffer = new List<PriorityCallback>();
    List<PriorityCallback> cbPostGBuffer = new List<PriorityCallback>();
    List<PriorityCallback> cbPreLighting = new List<PriorityCallback>();
    List<PriorityCallback> cbPostLighting = new List<PriorityCallback>();
    List<PriorityCallback> cbTransparent = new List<PriorityCallback>();
    List<PriorityCallback> cbPostEffect = new List<PriorityCallback>();

    [System.NonSerialized] bool needs_reflesh = true;


    public void AddCallbackPreGBuffer(Callback cb, int priority = 1000)
    {
        cbPreGBuffer.Add(new PriorityCallback(cb, priority));
        cbPreGBuffer.Sort(new PriorityCallbackComp());
    }
    public void AddCallbackPostGBuffer(Callback cb, int priority = 1000)
    {
        cbPostGBuffer.Add(new PriorityCallback(cb, priority));
        cbPostGBuffer.Sort(new PriorityCallbackComp());
    }
    public void AddCallbackPreLighting(Callback cb, int priority = 1000)
    {
        cbPreLighting.Add(new PriorityCallback(cb, priority));
        cbPreLighting.Sort(new PriorityCallbackComp());
    }
    public void AddCallbackPostLighting(Callback cb, int priority = 1000)
    {
        cbPostLighting.Add(new PriorityCallback(cb, priority));
        cbPostLighting.Sort(new PriorityCallbackComp());
    }
    public void AddCallbackTransparent(Callback cb, int priority = 1000)
    {
        cbTransparent.Add(new PriorityCallback(cb, priority));
        cbTransparent.Sort(new PriorityCallbackComp());
    }
    public void AddCallbackPostEffect(Callback cb, int priority = 1000)
    {
        cbPostEffect.Add(new PriorityCallback(cb, priority));
        cbPostEffect.Sort(new PriorityCallbackComp());
    }

    public Vector2 GetInternalResolution()
    {
        return new Vector2(cam.pixelWidth, cam.pixelHeight) * resolution_scale;
    }


    public static RenderTexture CreateRenderTexture(int w, int h, int d, RenderTextureFormat f)
    {
        RenderTexture r = new RenderTexture(w, h, d, f);
        r.filterMode = FilterMode.Point;
        r.useMipMap = false;
        r.generateMips = false;
        r.enableRandomWrite = true;
        //r.wrapMode = TextureWrapMode.Repeat;
        r.Create();
        return r;
    }

    void Awake ()
    {
        rtGBuffer = new RenderTexture[4];
        rtPrevGBuffer = new RenderTexture[4];
        rbGBuffer = new RenderBuffer[4];
        cam = GetComponent<Camera>();

        UpdateRenderTargets();
        needs_reflesh = false;
    }

    void Update()
    {
        if (needs_reflesh) Awake();
    }

    void UpdateRenderTargets()
    {
        RenderTextureFormat format = textureFormat == RenderFormat.float16 ? RenderTextureFormat.ARGBHalf : RenderTextureFormat.ARGBFloat;
        Vector2 reso = GetInternalResolution();
        if (rtGBuffer[0]!=null && rtGBuffer[0].width != reso.x) {
            for (int i = 0; i < rtGBuffer.Length; ++i)
            {
                rtGBuffer[i].Release();
                rtPrevGBuffer[i].Release();
                rtPrevGBuffer[i] = null;
            }
            rtComposite.Release();
            rtComposite = null;
            if (rtCompositeShadow!=null)
            {
                rtCompositeShadow.Release();
                rtCompositeShadow = null;
            }
        }
        if (rtGBuffer[0] == null || !rtGBuffer[0].IsCreated())
        {
            for (int i = 0; i < rtGBuffer.Length; ++i)
            {
                int depthbits = i == 0 ? 32 : 0;
                rtGBuffer[i] = CreateRenderTexture((int)reso.x, (int)reso.y, depthbits, format);
                rtPrevGBuffer[i] = CreateRenderTexture((int)reso.x, (int)reso.y, depthbits, format);
            }
            rtComposite = CreateRenderTexture((int)reso.x, (int)reso.y, 0, format);
            rtComposite.filterMode = FilterMode.Trilinear;
        }
    }


    public void SetRenderTargetsGBuffer()
    {
        Graphics.SetRenderTarget(rbGBuffer, rtNormalBuffer.depthBuffer);
    }

    public void SetRenderTargetsComposite()
    {
        Graphics.SetRenderTarget(rtComposite);
    }

    public RenderTexture UpdateShadowFramebuffer()
    {
        if (rtCompositeShadow == null)
        {
            RenderTextureFormat format = textureFormat == RenderFormat.float16 ? RenderTextureFormat.ARGBHalf : RenderTextureFormat.ARGBFloat;
            Vector2 reso = GetInternalResolution();
            rtCompositeShadow = CreateRenderTexture((int)reso.x, (int)reso.y, 0, format);
            rtCompositeShadow.filterMode = FilterMode.Trilinear;
        }
        Graphics.Blit(rtComposite, rtCompositeShadow);
        Shader.SetGlobalTexture("frame_buffer", rtCompositeShadow);
        Graphics.SetRenderTarget(rtComposite.colorBuffer, rtNormalBuffer.depthBuffer);
        return rtCompositeShadow;
    }

    void OnPreRender()
    {
        UpdateRenderTargets();
        Matrix4x4 proj = cam.projectionMatrix;
        Matrix4x4 view = cam.worldToCameraMatrix;
        proj[2, 0] = proj[2, 0] * 0.5f + proj[3, 0] * 0.5f;
        proj[2, 1] = proj[2, 1] * 0.5f + proj[3, 1] * 0.5f;
        proj[2, 2] = proj[2, 2] * 0.5f + proj[3, 2] * 0.5f;
        proj[2, 3] = proj[2, 3] * 0.5f + proj[3, 3] * 0.5f;
        prevViewProj = viewProj;
        prevViewProjInv = prevViewProj.inverse;
        viewProj = proj * view;
        viewProjInv = viewProj.inverse;

        for (int i = 0; i < rtGBuffer.Length; ++i)
        {
            Swap(ref rtPrevGBuffer[i], ref rtGBuffer[i]);
        }
        for (int i = 0; i < rtGBuffer.Length; ++i)
        {
            rbGBuffer[i] = rtGBuffer[i].colorBuffer;
        }

        Graphics.SetRenderTarget(rbGBuffer, rtNormalBuffer.depthBuffer);
        matGBufferClear.SetPass(0);
        DrawFullscreenQuad();

        // なんか OnPreRender() の段階で Graphics.Draw 一族で描こうとすると、最初の一回は view project 行列掛けた結果の y が反転する。
        // しょうがないので何も描かない Graphics.DrawMeshNow() をここでやることで回避。
        Graphics.DrawMeshNow(dummy_mesh, Matrix4x4.Scale(Vector3.zero));

        foreach (PriorityCallback cb in cbPreGBuffer) { cb.callback.Invoke(); }
    }

    void OnPostRender()
    {
        foreach (PriorityCallback cb in cbPostGBuffer) {
            cb.callback.Invoke();
            SetRenderTargetsGBuffer();
        }

        Graphics.SetRenderTarget(rtComposite);
        GL.Clear(true, true, Color.black);
        Graphics.SetRenderTarget(rtComposite.colorBuffer, rtNormalBuffer.depthBuffer);

        foreach (PriorityCallback cb in cbPreLighting) { cb.callback.Invoke(); }
        DSLight.RenderLights(this);
        foreach (PriorityCallback cb in cbPostLighting) { cb.callback.Invoke(); }
        foreach (PriorityCallback cb in cbTransparent) { cb.callback.Invoke(); }
        foreach (PriorityCallback cb in cbPostEffect) { cb.callback.Invoke(); }

        //// debug
        //if (Time.frameCount % 60 == 0)
        //{
        //    Debug.Log("cbPreLighting: " + cbPreLighting.Count);
        //    Debug.Log("cbPostLighting: " + cbPostLighting.Count);
        //    Debug.Log("cbTransparent: " + cbTransparent.Count);
        //    Debug.Log("cbPostEffect: " + cbPostEffect.Count);
        //}

        Graphics.SetRenderTarget(null);
        matCombine.SetTexture("_MainTex", rtComposite);
        matCombine.SetPass(1);
        DrawFullscreenQuad();
    }

    void OnGUI()
    {
        if (!showBuffers) { return; }

        Vector2 size = new Vector2(rtNormalBuffer.width, rtNormalBuffer.height) / 6.0f;
        float y = 5.0f;
        for (int i = 0; i < 4; ++i )
        {
            GUI.DrawTexture(new Rect(5, y, size.x, size.y), rtGBuffer[i], ScaleMode.ScaleToFit, false);
            y += size.y + 5.0f;
        }
        GUI.DrawTexture(new Rect(5, y, size.x, size.y), rtComposite, ScaleMode.ScaleToFit, false);
        y += size.y + 5.0f;
    }

    public static void DrawFullscreenQuad(float z=1.0f)
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

    public static void Swap<T>(ref T lhs, ref T rhs)
    {
        T temp;
        temp = lhs;
        lhs = rhs;
        rhs = temp;
    }

}
