using UnityEngine;
using System;
using System.Collections;

public class DSPEGlowline : DSEffectBase
{
    public enum SpreadPattern
    {
        Radial = 0,
        Voronoi = 1,
    }
    public enum GridPattern
    {
        Square = 0,
        Hexagon = 1,
        BoxCell = 2,
    }

    public GridPattern gridPattern = GridPattern.BoxCell;
    public SpreadPattern spreadPattern = SpreadPattern.Radial;
    public float intensity = 1.0f;
    public Vector4 baseColor = new Vector4(0.45f, 0.4f, 2.0f, 0.0f);
    public Material matGlowLine;
    RenderBuffer[] rbBuffers;
    Action m_render;

    void OnEnable()
    {
        ResetDSRenderer();
        if (m_render == null)
        {
            m_render = Render;
            GetDSRenderer().AddCallbackPostGBuffer(m_render, 100);
            rbBuffers = new RenderBuffer[2];
        }
    }

    void Render()
    {
        if (!enabled) { return; }

        DSRenderer dsr = GetDSRenderer();
        rbBuffers[0] = dsr.rtGlowBuffer.colorBuffer;
        rbBuffers[1] = dsr.rtColorBuffer.colorBuffer;

        Graphics.SetRenderTarget(rbBuffers, dsr.rtNormalBuffer.depthBuffer);
        matGlowLine.SetTexture("_PositionBuffer", dsr.rtPositionBuffer);
        matGlowLine.SetTexture("_NormalBuffer", dsr.rtNormalBuffer);
        matGlowLine.SetFloat("_Intensity", intensity);
        matGlowLine.SetVector("_BaseColor", baseColor);
        matGlowLine.SetInt("_GridPattern", (int)gridPattern);
        matGlowLine.SetInt("_SpreadPattern", (int)spreadPattern);
        matGlowLine.SetPass(0);
        DSRenderer.DrawFullscreenQuad();
        Graphics.SetRenderTarget(dsr.rtComposite);
    }
}
