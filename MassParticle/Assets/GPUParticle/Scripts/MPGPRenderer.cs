using System.Collections;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif // UNITY_EDITOR


[AddComponentMenu("GPUParticle/Renderer")]
[RequireComponent(typeof(MPGPWorld))]
public class MPGPRenderer : BatchRendererBase
{
    public float m_size = 0.2f;

    MPGPWorld m_world;

#if UNITY_EDITOR
    void Reset()
    {
        m_mesh = AssetDatabase.LoadAssetAtPath("Assets/BatchRenderer/Meshes/cube.asset", typeof(Mesh)) as Mesh;
        m_material = AssetDatabase.LoadAssetAtPath("Assets/GPUParticle/Materials/MPGPStandard.mat", typeof(Material)) as Material;
        m_bounds_size = Vector3.one * 2.0f;
    }
#endif // UNITY_EDITOR



    public override Material CloneMaterial(int nth)
    {
        Material m = new Material(m_material);
        m.SetInt("g_batch_begin", nth * m_instances_par_batch);
        m.SetFloat("g_size", m_size);
        m.SetBuffer("particles", m_world.GetParticleBuffer());

        // fix rendering order for transparent objects
        if (m.renderQueue >= 3000)
        {
            m.renderQueue = m.renderQueue + (nth + 1);
        }
        return m;
    }


    public virtual void ReleaseGPUResources()
    {
        if (m_materials != null)
        {
            m_materials.Clear();
        }
    }

    public virtual void ResetGPUResoures()
    {
        ReleaseGPUResources();

        UpdateGPUResources();
    }

    public override void UpdateGPUResources()
    {
        if (m_materials != null)
        {
            m_materials.ForEach((v) =>
            {
                v.SetInt("g_num_max_instances", m_max_instances);
                v.SetInt("g_num_instances", m_instance_count);
                v.SetFloat("g_size", m_size);
            });
        }
    }


    public override void OnEnable()
    {
        m_world = GetComponent<MPGPWorld>();
        m_max_instances = m_world.GetNumMaxParticles();

        base.OnEnable();
        ResetGPUResoures();
    }

    public override void OnDisable()
    {
        base.OnDisable();
        ReleaseGPUResources();
    }

    public override void LateUpdate()
    {
        if (m_world != null)
        {
            m_instance_count = m_max_instances;
            base.LateUpdate();
        }
    }

    public override void OnDrawGizmos()
    {
    }
}
