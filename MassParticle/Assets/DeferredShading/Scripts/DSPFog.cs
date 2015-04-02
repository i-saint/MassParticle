using UnityEngine;
using System;
using System.Collections;


public class DSPFog : DSEffectBase
{
    public Material matFog;
    public Vector4 color = new Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    public float near = 5.0f;
    public float far = 10.0f;
    Action m_render;

    void OnEnable()
    {
        ResetDSRenderer();
        if (m_render == null)
        {
            m_render = Render;
            GetDSRenderer().AddCallbackPostEffect(m_render, 1100);
        }
    }

    void Render()
    {
        if (!enabled) { return; }

        matFog.SetTexture("_PositionBuffer", GetDSRenderer().rtPositionBuffer);
        matFog.SetVector("_Color", color);
        matFog.SetFloat("_Near", near);
        matFog.SetFloat("_Far", far);
        matFog.SetPass(0);
        DSRenderer.DrawFullscreenQuad();
    }
}
