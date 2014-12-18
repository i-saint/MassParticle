using UnityEngine;
using System.Collections;


public class DSPFog : DSEffectBase
{
    public Material matFog;
    public Vector4 color = new Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    public float near = 5.0f;
    public float far = 10.0f;

    public override void Awake()
    {
        base.Awake();
        GetDSRenderer().AddCallbackPostEffect(() => { Render(); }, 1100);
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
