using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;

[Serializable]
public class DSRadialBlur
{
    public Vector3 pos;
    public float size = 15.0f;
    public float strength = 0.2f;
    public float fade_speed = 0.1f;
    public float pow = 0.7f;
    public float time = 0.0f;

    public Matrix4x4 matrix;
    public Vector4 radialblur_params;

    public void Update()
    {
        time += Time.deltaTime;
        strength -= fade_speed*Time.deltaTime;
        float s = size * 0.49f;
        radialblur_params.Set(s, 1.0f / s, strength, pow);
        matrix = Matrix4x4.TRS(pos, Quaternion.identity, Vector3.one*size);
    }

    public bool IsDead()
    {
        return strength <= 0.0f;
    }
}


public class DSEffectRadialBlur : DSEffectBase
{
    public static DSEffectRadialBlur s_instance;

    public Material m_material;
    public Mesh m_mesh;
    int m_i_radialblur_params;
    int m_i_base_position;
    public List<DSRadialBlur> m_entries = new List<DSRadialBlur>();


    public static DSRadialBlur AddEntry(
        Vector3 pos, float size = 20.0f, float strength = 0.5f, float fade_speed = 0.5f, float pow = 0.7f)
    {
        if (!s_instance.enabled) return null;
        DSRadialBlur e = new DSRadialBlur
        {
            pos = pos,
            size = size,
            strength = strength,
            fade_speed = fade_speed,
            pow = pow,
        };
        s_instance.m_entries.Add(e);
        return e;
    }


    public override void Awake()
    {
        base.Awake();
        s_instance = this;
        GetDSRenderer().AddCallbackPostEffect(() => { Render(); }, 10000);
        m_i_radialblur_params = Shader.PropertyToID("radialblur_params");
        m_i_base_position = Shader.PropertyToID("base_position");
    }

    void OnDestroy()
    {
        if (s_instance == this) s_instance = null;
    }

    public override void Update()
    {
        base.Update();
        m_entries.ForEach((a) => { a.Update(); });
        m_entries.RemoveAll((a) => { return a.IsDead(); });
    }

    void Render()
    {
        if (!enabled || m_entries.Count == 0) { return; }
        GetDSRenderer().UpdateShadowFramebuffer();
        m_entries.ForEach((a) =>
        {
            m_material.SetVector(m_i_radialblur_params, a.radialblur_params);
            m_material.SetVector(m_i_base_position, a.pos);
            m_material.SetPass(0);
            Graphics.DrawMeshNow(m_mesh, a.matrix);
        });
    }
}
