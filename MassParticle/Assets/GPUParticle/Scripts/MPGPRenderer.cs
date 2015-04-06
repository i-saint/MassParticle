using System.Collections;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif // UNITY_EDITOR


[RequireComponent(typeof(MPGPWorld))]
public class MPGPRendererBase : MonoBehaviour
{
    public Camera[] m_camera;
    public Mesh m_mesh;
    public Material m_material;

    protected MPGPWorld m_pw;
    protected ComputeBuffer m_buf_vertices;
    protected int m_num_vertices;

    public virtual void OnEnable()
    {
        m_pw = GetComponent<MPGPWorld>();
        MPGPUtils.CreateVertexBuffer(m_mesh, ref m_buf_vertices, ref m_num_vertices);
        if (m_camera == null || m_camera.Length == 0)
        {
            m_camera = new Camera[1] { Camera.main };
        }
    }
}

[AddComponentMenu("GPUParticle/Renderer")]
public class MPGPRenderer : MPGPRendererBase
{
    public bool m_enable_depth_prepass;

    System.Action m_act_depth_prepass;
    System.Action m_act_render;

#if UNITY_EDITOR
    void Reset()
    {
        m_mesh = AssetDatabase.LoadAssetAtPath("Assets/BatchRenderer/Meshes/cube.asset", typeof(Mesh)) as Mesh;
        m_material = AssetDatabase.LoadAssetAtPath("Assets/GPUParticle/Materials/ParticleGBuffer.mat", typeof(Material)) as Material;
    }
#endif // UNITY_EDITOR

    public override void OnEnable()
    {
        base.OnEnable();

        if (m_act_depth_prepass==null)
        {
            m_act_depth_prepass = DepthPrePass;
            m_act_render = Render;
            foreach (var c in m_camera)
            {
                if (c == null) continue;
                DSRenderer dsr = c.GetComponent<DSRenderer>();
                dsr.AddCallbackPreGBuffer(m_act_depth_prepass);
                dsr.AddCallbackPostGBuffer(m_act_render);
            }

        }
    }

    void OnDisable()
    {
        if (m_buf_vertices != null)
        {
            m_buf_vertices.Release();
            m_buf_vertices = null;
        }
    }


    public void DepthPrePass()
    {
        if (!enabled || !m_pw.enabled) return;

        m_material.SetBuffer("vertices", m_buf_vertices);
        m_material.SetBuffer("particles", m_pw.GetParticleBuffer());
        m_material.SetPass(1);
        Graphics.DrawProcedural(m_mesh.GetTopology(0), m_num_vertices, m_pw.GetNumMaxParticles());
    }

    public void Render()
    {
        if (!enabled || !m_pw.enabled) return;

        m_material.SetBuffer("vertices", m_buf_vertices);
        m_material.SetBuffer("particles", m_pw.GetParticleBuffer());
        m_material.SetPass(m_enable_depth_prepass ? 2 : 0);
        Graphics.DrawProcedural(m_mesh.GetTopology(0), m_num_vertices, m_pw.GetNumMaxParticles());
    }
}
