using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;

[Serializable]
public class DSShockwave
{
    public Vector3 pos;
    public float gap = -0.5f;
    public float speed = 20.0f;
    public float fade_speed = 2.0f;
    public float opacity = 1.5f;
    public float scale = 1.0f;
    public float time = 0.0f;

    public Matrix4x4 matrix;
    public Vector4 shockwave_params;

    public void Update()
    {
        time += Time.deltaTime;
        scale += speed * Time.deltaTime;
        opacity -= fade_speed * Time.deltaTime;
        shockwave_params.Set(opacity, gap, gap, gap);
        matrix = Matrix4x4.TRS(pos, Quaternion.identity, Vector3.one * scale);
    }

    public bool IsDead()
    {
        return opacity <= 0.0f;
    }
}

public class DSEffectShockwave : DSEffectBase
{
    public static DSEffectShockwave s_instance;

    public Material m_material;
    public Mesh m_mesh;
    public List<DSShockwave> m_entries = new List<DSShockwave>();
    int m_i_shockwave_params;
    Action m_render;


    public static DSShockwave AddEntry(Vector3 pos, float gap = -0.5f, float fade_speed = 2.0f, float opacity = 1.5f, float scale = 1.0f)
    {
        if (!s_instance.enabled) return null;
        DSShockwave e = new DSShockwave
        {
            pos = pos,
            gap = gap,
            fade_speed = fade_speed,
            opacity = opacity,
            scale = scale,
        };
        s_instance.m_entries.Add(e);
        return e;
    }


    void OnEnable()
    {
        ResetDSRenderer();
        s_instance = this;
        if (m_render == null)
        {
            m_render = Render;
            GetDSRenderer().AddCallbackPostEffect(m_render, 5000);
            m_i_shockwave_params = Shader.PropertyToID("shockwave_params");
        }
    }

    void OnDisable()
    {
        if (s_instance == this) s_instance = null;
    }

    void Update()
    {
        m_entries.ForEach((a) => { a.Update(); });
        m_entries.RemoveAll((a) => { return a.IsDead(); });
    }

    void Render()
    {
        if (!enabled || m_entries.Count == 0) { return; }
        GetDSRenderer().UpdateShadowFramebuffer();
        m_entries.ForEach((a) => {
            m_material.SetVector(m_i_shockwave_params, a.shockwave_params);
            m_material.SetPass(0);
            Graphics.DrawMeshNow(m_mesh, a.matrix);
        });
    }
}

