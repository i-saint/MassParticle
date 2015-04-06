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
    RenderTexture m_instance_texture;
    Bounds m_bounds;

#if UNITY_EDITOR
    void Reset()
    {
        m_mesh = AssetDatabase.LoadAssetAtPath("Assets/BatchRenderer/Meshes/cube.asset", typeof(Mesh)) as Mesh;
        m_material = AssetDatabase.LoadAssetAtPath("Assets/GPUParticle/Materials/MPGPStandard.mat", typeof(Material)) as Material;
    }
#endif // UNITY_EDITOR



    public override Material CloneMaterial(int nth)
    {
        Material m = new Material(m_material);
        m.SetInt("g_batch_begin", nth * m_instances_par_batch);
        m.SetTexture("g_instance_data", m_instance_texture);
        m.SetFloat("g_size", m_size);
        m.SetBuffer("particles", m_world.GetParticleBuffer());

        Vector4 ts = new Vector4(
            1.0f / m_instance_texture.width,
            1.0f / m_instance_texture.height,
            m_instance_texture.width,
            m_instance_texture.height);
        m.SetVector("g_instance_data_size", ts);

        // fix rendering order for transparent objects
        if (m.renderQueue >= 3000)
        {
            m.renderQueue = m.renderQueue + (nth + 1);
        }
        return m;
    }


    public virtual void ReleaseGPUResources()
    {
        if (m_instance_texture != null)
        {
            m_instance_texture.Release();
            m_instance_texture = null;
        }
        if (m_materials != null)
        {
            m_materials.Clear();
        }
    }

    public virtual void ResetGPUResoures()
    {
        ReleaseGPUResources();

        m_instance_texture = new RenderTexture(MPWorld.DataTextureWidth, MPWorld.DataTextureHeight, 0, RenderTextureFormat.ARGBFloat, RenderTextureReadWrite.Default);
        m_instance_texture.filterMode = FilterMode.Point;
        m_instance_texture.Create();

        UpdateGPUResources();
    }

    public override void UpdateGPUResources()
    {
        m_materials.ForEach((v) =>
        {
            v.SetInt("g_num_max_instances", m_max_instances);
            v.SetInt("g_num_instances", m_instance_count);
            v.SetFloat("g_size", m_size);
        });
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
            Transform t = m_world.GetComponent<Transform>();
            Vector3 min = t.position - t.localScale;
            Vector3 max = t.position + t.localScale;
            m_bounds.SetMinMax(min, max);
            base.LateUpdate();
        }
    }

    public override void OnDrawGizmos()
    {
    }
}
