using UnityEngine;
using System.Collections;

public class DSPENormalPattern : DSEffectBase
{
    public Material matNormalPattern;
    public Material matCopyGBuffer;
    public RenderTexture rtNormalCopy;

    public override void Awake()
    {
        base.Awake();
        GetDSRenderer().AddCallbackPostGBuffer(() => { Render(); }, 100);
    }

    void UpdateRenderTargets()
    {
        Vector2 reso = GetDSRenderer().GetInternalResolution();
        if (rtNormalCopy != null && rtNormalCopy.width!=reso.x)
        {
            rtNormalCopy.Release();
            rtNormalCopy = null;
        }
        if (rtNormalCopy == null || !rtNormalCopy.IsCreated())
        {
            rtNormalCopy = DSRenderer.CreateRenderTexture((int)reso.x, (int)reso.y, 0, RenderTextureFormat.ARGBHalf);
        }
    }

    void Render()
    {
        if (!enabled) { return; }

        UpdateRenderTargets();

        DSRenderer dsr = GetDSRenderer();
        Graphics.SetRenderTarget(rtNormalCopy);
        GL.Clear(false, true, Color.black);
        matNormalPattern.SetTexture("_PositionBuffer", dsr.rtPositionBuffer);
        matNormalPattern.SetTexture("_NormalBuffer", dsr.rtNormalBuffer);
        matNormalPattern.SetPass(0);
        DSRenderer.DrawFullscreenQuad();

        Graphics.SetRenderTarget(dsr.rtNormalBuffer);
        matCopyGBuffer.SetTexture("_NormalBuffer", rtNormalCopy);
        matCopyGBuffer.SetPass(2);
        DSRenderer.DrawFullscreenQuad();

        dsr.SetRenderTargetsGBuffer();
    }
}
