using UnityEngine;
using System.Collections;

[RequireComponent(typeof(DSRenderer))]
public class DSPENormalPattern : MonoBehaviour
{
    public Material matNormalPattern;
    public Material matCopyGBuffer;
    public RenderTexture rtNormalCopy;
    DSRenderer dscam;

    void Start()
    {
        dscam = GetComponent<DSRenderer>();
        dscam.AddCallbackPostGBuffer(() => { Render(); }, 100);
    }


    void Render()
    {
        if (!enabled) { return; }

        Vector2 reso = dscam.GetRenderResolution();
        if (rtNormalCopy == null)
        {
            rtNormalCopy = DSRenderer.CreateRenderTexture((int)reso.x, (int)reso.y, 0, RenderTextureFormat.ARGBHalf);
        }

        Graphics.SetRenderTarget(rtNormalCopy);
        GL.Clear(false, true, Color.black);
        matNormalPattern.SetTexture("_PositionBuffer", dscam.rtPositionBuffer);
        matNormalPattern.SetTexture("_NormalBuffer", dscam.rtNormalBuffer);
        matNormalPattern.SetPass(0);
        DSRenderer.DrawFullscreenQuad();

        Graphics.SetRenderTarget(dscam.rtNormalBuffer);
        matCopyGBuffer.SetTexture("_NormalBuffer", rtNormalCopy);
        matCopyGBuffer.SetPass(2);
        DSRenderer.DrawFullscreenQuad();

        dscam.SetRenderTargetsGBuffer();
    }
}
