using UnityEngine;
using System.Collections;

public class DSLogicOpRenderer : MonoBehaviour
{
    public static DSLogicOpRenderer instance;

    public int layerLogicOp = 31;
    public Material matAnd;
    public RenderTexture rtRDepth;
    public RenderTexture rtAndRDepth;
    public RenderTexture[] rtAndGBuffer;
    public RenderBuffer[] rbAndGBuffer;
    public RenderTexture rtAndNormalBuffer		{ get { return rtAndGBuffer[0]; } }
    public RenderTexture rtAndPositionBuffer	{ get { return rtAndGBuffer[1]; } }
    public RenderTexture rtAndColorBuffer		{ get { return rtAndGBuffer[2]; } }
    public RenderTexture rtAndGlowBuffer		{ get { return rtAndGBuffer[3]; } }

    DSRenderer dscam;

    DSLogicOpRenderer()
    {
        instance = this;
    }

    void Start()
    {
        dscam = GetComponent<DSRenderer>();
        dscam.AddCallbackPreGBuffer(() => { Render(); }, 900);

        Camera cam = GetComponent<Camera>();
        cam.cullingMask = cam.cullingMask & (~(1 << layerLogicOp));
    }

    void InitializeResources()
    {
        Vector2 reso = dscam.GetRenderResolution();
        if (rtRDepth == null)
        {
            rtRDepth = DSRenderer.CreateRenderTexture((int)reso.x, (int)reso.y, 32, RenderTextureFormat.RHalf);
        }
        if (DSAnd.instances.Count > 0 && rtAndRDepth==null)
        {
            Camera cam = dscam.cam;
            rtAndRDepth = DSRenderer.CreateRenderTexture((int)reso.x, (int)reso.y, 32, RenderTextureFormat.RHalf);
            rtAndGBuffer = new RenderTexture[4];
            rbAndGBuffer = new RenderBuffer[4];
            for (int i = 0; i < rtAndGBuffer.Length; ++i)
            {
                int depthbits = i == 0 ? 32 : 0;
                rtAndGBuffer[i] = DSRenderer.CreateRenderTexture((int)reso.x, (int)reso.y, depthbits, RenderTextureFormat.ARGBHalf);
                rbAndGBuffer[i] = rtAndGBuffer[i].colorBuffer;
            }
        }
    }

    void Render()
    {
        if (!enabled) { return; }

        // if there is no subtract or and object, just create g-buffer
        if (DSSubtract.instances.Count == 0 && DSAnd.instances.Count==0)
        {
            foreach (DSLogicOpReceiver l in DSLogicOpReceiver.instances)
            {
                l.matGBuffer.SetPass(0);
                Graphics.DrawMeshNow(l.mesh, l.trans.localToWorldMatrix);
            }
            return;
        }


        InitializeResources();


        // and
        if (DSAnd.instances.Count>0)
        {
            Graphics.SetRenderTarget(rtAndRDepth);
            GL.Clear(true, true, Color.black, 0.0f);
            foreach (DSAnd l in DSAnd.instances)
            {
                l.matReverseDepth.SetPass(0);
                Graphics.DrawMeshNow(l.mesh, l.trans.localToWorldMatrix);
            }

            Graphics.SetRenderTarget(rbAndGBuffer, rtAndNormalBuffer.depthBuffer);
            dscam.matGBufferClear.SetPass(0);
            DSRenderer.DrawFullscreenQuad();
            foreach (DSAnd l in DSAnd.instances)
            {
                l.matGBuffer.SetPass(0);
                Graphics.DrawMeshNow(l.mesh, l.trans.localToWorldMatrix);
            }
            dscam.SetRenderTargetsGBuffer();
        }

        // create g-buffer 
        if (DSAnd.instances.Count > 0)
        {
            Graphics.SetRenderTarget(rtRDepth);
            GL.Clear(true, true, Color.black, 0.0f);
            foreach (DSLogicOpReceiver l in DSLogicOpReceiver.instances)
            {
                Graphics.SetRenderTarget(rtRDepth);
                l.matReverseDepth.SetPass(0);
                Graphics.DrawMeshNow(l.mesh, l.trans.localToWorldMatrix);

                dscam.SetRenderTargetsGBuffer();
                l.matGBuffer.SetInt("_EnableLogicOp", 1);
                l.matGBuffer.SetTexture("_RDepthBuffer", rtRDepth);
                l.matGBuffer.SetTexture("_AndRDepthBuffer", rtAndRDepth);
                l.matGBuffer.SetTexture("_AndNormalBuffer", rtAndNormalBuffer);
                l.matGBuffer.SetTexture("_AndPositionBuffer", rtAndPositionBuffer);
                l.matGBuffer.SetTexture("_AndColorBuffer", rtAndColorBuffer);
                l.matGBuffer.SetTexture("_AndGlowBuffer", rtAndGlowBuffer);
                l.matGBuffer.SetPass(0);
                Graphics.DrawMeshNow(l.mesh, l.trans.localToWorldMatrix);

                Graphics.SetRenderTarget(rtRDepth);
                l.matDepthClear.SetPass(0);
                Graphics.DrawMeshNow(l.mesh, l.trans.localToWorldMatrix);
            }
            dscam.SetRenderTargetsGBuffer();
        }
        else
        {
            foreach (DSLogicOpReceiver l in DSLogicOpReceiver.instances)
            {
                l.matGBuffer.SetInt("_EnableLogicOp", 0);
                l.matGBuffer.SetPass(0);
                Graphics.DrawMeshNow(l.mesh, l.trans.localToWorldMatrix);
            }
        }

        // create depth buffer with reversed meshes
        {
            Graphics.SetRenderTarget(rtRDepth);
            GL.Clear(true, true, Color.black, 0.0f);
            foreach (DSLogicOpReceiver l in DSLogicOpReceiver.instances)
            {
                l.matReverseDepth.SetPass(0);
                Graphics.DrawMeshNow(l.mesh, l.trans.localToWorldMatrix);
            }
            dscam.SetRenderTargetsGBuffer();
        }

        // subtraction
        foreach (DSSubtract l in DSSubtract.instances)
        {
            l.matStencilWrite.SetPass(0);
            Graphics.DrawMeshNow(l.mesh, l.trans.localToWorldMatrix);
            l.matSubtractor.SetTexture("_Depth", rtRDepth);
            l.matSubtractor.SetPass(0);
            Graphics.DrawMeshNow(l.mesh, l.trans.localToWorldMatrix);
            l.matStencilClear.SetPass(0);
            Graphics.DrawMeshNow(l.mesh, l.trans.localToWorldMatrix);
        }
    }
}
