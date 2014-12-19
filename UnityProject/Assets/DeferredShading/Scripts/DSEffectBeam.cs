using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;



[Serializable]
public class DSBeam
{
    public enum State
    {
        Active,
        Fading,
    }

    public Vector3 pos;
    public Vector3 dir;
    public float speed = 20.0f;
    public float length = 0.0f;
    public float fade_speed = 0.025f;
    public float lifetime = 2.0f;
    public float scale = 1.0f;
    public float time = 0.0f;
    public State state = State.Active;

    public Matrix4x4 matrix;
    public Vector4 beam_params;

    public void Update()
    {
        time += Time.deltaTime;
        length = speed * time;
        beam_params.Set(dir.x, dir.y, dir.z, length);

        if (state == State.Active)
        {
            if (time > lifetime)
            {
                state = State.Fading;
            }
        }
        else if (state == State.Fading)
        {
            scale -= fade_speed;
        }
        matrix = Matrix4x4.TRS(pos, Quaternion.LookRotation(dir), Vector3.one * scale);
    }

    public bool IsDead()
    {
        return scale <= 0.0f;
    }
}


public class DSEffectBeam : DSEffectBase
{
    public static DSEffectBeam s_instance;

    public Material m_material;
    public Mesh m_mesh;
    public List<DSBeam> m_entries = new List<DSBeam>();
    int m_i_beam_direction;
    int m_i_base_position;
    Action m_depth_prepass;
    Action m_render;

    public static DSBeam AddEntry(Vector3 pos, Vector3 dir, float fade_speed = 0.025f, float lifetime = 2.0f, float scale = 1.0f)
    {
        if (!s_instance.enabled) return null;
        DSBeam e = new DSBeam {
            pos = pos,
            dir = dir,
            fade_speed = fade_speed,
            lifetime = lifetime,
            scale = scale,
        };
        s_instance.m_entries.Add(e);
        return e;
    }


    void OnEnable()
    {
        ResetDSRenderer();
        s_instance = this;
        if (m_depth_prepass==null)
        {
            m_depth_prepass = DepthPrePass;
            m_render = Render;
            GetDSRenderer().AddCallbackPreGBuffer(m_depth_prepass);
            GetDSRenderer().AddCallbackPostGBuffer(m_render);

            m_i_beam_direction = Shader.PropertyToID("beam_direction");
            m_i_base_position = Shader.PropertyToID("base_position");
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

    void DepthPrePass()
    {
        if (!enabled || m_entries.Count == 0) { return; }
        m_entries.ForEach((a) =>
        {
            m_material.SetVector(m_i_beam_direction, a.beam_params);
            m_material.SetVector(m_i_base_position, a.pos);
            m_material.SetPass(0);
            Graphics.DrawMeshNow(m_mesh, a.matrix);
        });
    }

    void Render()
    {
        if (!enabled || m_entries.Count==0) { return; }
        m_entries.ForEach((a) =>
        {
            m_material.SetVector(m_i_beam_direction, a.beam_params);
            m_material.SetVector(m_i_base_position, a.pos);
            m_material.SetPass(1);
            Graphics.DrawMeshNow(m_mesh, a.matrix);
        });
    }
}
