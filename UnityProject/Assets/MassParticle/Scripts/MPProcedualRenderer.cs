using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public unsafe class MPProcedualRenderer : MonoBehaviour
{
    public struct Vertex
    {
        public Vector3 position;
        public Vector3 normal;
    }

    public MPWorld target;
    public Material material;
    public float size = 0.2f;
    ComputeBuffer cube_vertices;
    RenderTexture data_texture;
    [System.NonSerialized] bool m_needs_reflesh = true;


    void Awake ()
    {
        m_needs_reflesh = false;
        DSRenderer dsr = GetComponent<DSRenderer>();
        if (dsr != null)
        {
            dsr.AddCallbackPreGBuffer(() => { UpdateBuffer(); }, 0);
            dsr.AddCallbackPreGBuffer(() => { DepthPrePass(); });
            dsr.AddCallbackPostGBuffer(() => { GBufferPass(); }, 10);
        }


        if (cube_vertices==null) cube_vertices = new ComputeBuffer(36, 24);
        {
            const float s = 0.5f / 100.0f;
            const float p = 1.0f;
            const float n = -1.0f;
            const float z = 0.0f;
            Vector3[] positions = new Vector3[24] {
                new Vector3(-s,-s, s), new Vector3( s,-s, s), new Vector3( s, s, s), new Vector3(-s, s, s),
                new Vector3(-s, s,-s), new Vector3( s, s,-s), new Vector3( s, s, s), new Vector3(-s, s, s),
                new Vector3(-s,-s,-s), new Vector3( s,-s,-s), new Vector3( s,-s, s), new Vector3(-s,-s, s),
                new Vector3(-s,-s, s), new Vector3(-s,-s,-s), new Vector3(-s, s,-s), new Vector3(-s, s, s),
                new Vector3( s,-s, s), new Vector3( s,-s,-s), new Vector3( s, s,-s), new Vector3( s, s, s),
                new Vector3(-s,-s,-s), new Vector3( s,-s,-s), new Vector3( s, s,-s), new Vector3(-s, s,-s),
            };
            Vector3[] normals = new Vector3[24] {
                new Vector3(z, z, p), new Vector3(z, z, p), new Vector3(z, z, p), new Vector3(z, z, p),
                new Vector3(z, p, z), new Vector3(z, p, z), new Vector3(z, p, z), new Vector3(z, p, z),
                new Vector3(z, n, z), new Vector3(z, n, z), new Vector3(z, n, z), new Vector3(z, n, z),
                new Vector3(n, z, z), new Vector3(n, z, z), new Vector3(n, z, z), new Vector3(n, z, z),
                new Vector3(p, z, z), new Vector3(p, z, z), new Vector3(p, z, z), new Vector3(p, z, z),
                new Vector3(z, z, n), new Vector3(z, z, n), new Vector3(z, z, n), new Vector3(z, z, n),
            };
            int[] indices = new int[36] {
                0,1,3, 3,1,2,
                5,4,6, 6,4,7,
                8,9,11, 11,9,10,
                13,12,14, 14,12,15,
                16,17,19, 19,17,18,
                21,20,22, 22,20,23,
            };
            Vertex[] vertices = new Vertex[36];
            for (int i = 0; i < vertices.Length; ++i)
            {
                vertices[i].position = positions[indices[i]];
                vertices[i].normal = normals[indices[i]];
            }
            cube_vertices.SetData(vertices);
        }

        if (data_texture == null || !data_texture.IsCreated())
        {
            data_texture = new RenderTexture(MPWorld.DataTextureWidth, MPWorld.DataTextureHeight, 0, RenderTextureFormat.ARGBFloat, RenderTextureReadWrite.Default);
            data_texture.filterMode = FilterMode.Point;
            data_texture.Create();
        }
    }

    void OnDestroy()
    {
        cube_vertices.Release();
    }


    void Update()
    {
        if (m_needs_reflesh) Awake();
    }


    public void UpdateBuffer()
    {
        if (!enabled) return;
        target.UpdateDataTexture(data_texture);
    }

    public void DepthPrePass()
    {
        if (!enabled) return;
        material.SetBuffer("vertices", cube_vertices);
        material.SetTexture("particle_data", data_texture);
        material.SetFloat("particle_data_pitch", 1.0f / MPWorld.DataTextureWidth);
        material.SetFloat("particle_size", size);
        material.SetPass(0);
        Graphics.DrawProcedural(MeshTopology.Triangles, 36, target.m_particle_num);
    }

    public void GBufferPass()
    {
        if (!enabled) return;
        material.SetBuffer("vertices", cube_vertices);
        material.SetTexture("particle_data", data_texture);
        material.SetFloat("particle_data_pitch", 1.0f / MPWorld.DataTextureWidth);
        material.SetFloat("particle_size", size);
        material.SetPass(1);
        Graphics.DrawProcedural(MeshTopology.Triangles, 36, target.m_particle_num);
    }
}
