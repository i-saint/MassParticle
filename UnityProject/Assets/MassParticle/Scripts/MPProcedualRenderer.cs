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

    public bool enableDepthPrePass = true;
    public MPWorld[] mpworlds;
    public Material mpmaterial;
    ComputeBuffer cbCubeVertices;


    void Awake ()
    {
        cbCubeVertices = new ComputeBuffer(36, 24);
        {
            const float s = 0.05f;
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
            cbCubeVertices.SetData(vertices);
        }
    }

    void Start()
    {
        DSRenderer dsr = GetComponent<DSRenderer>();
        if (dsr != null)
        {
            dsr.AddCallbackPreGBuffer(() => { DepthPrePass(); });
            dsr.AddCallbackPostGBuffer(() => { GBufferPass(); });
        }
    }

    void OnDestroy()
    {
        cbCubeVertices.Release();
    }

    public void OnPostRender()
    {
        DepthPrePass();
        GBufferPass();
    }

    public void DepthPrePass()
    {
        if (mpmaterial == null) { return; }
        if (!enableDepthPrePass) { return; }
        foreach (MPWorld mpworld in mpworlds)
        {
            mpmaterial.SetBuffer("vertices", cbCubeVertices);
            mpmaterial.SetBuffer("particles", mpworld.GetDataBuffer());
            mpmaterial.SetInt("_FlipY", 1);
            mpmaterial.SetPass(1);
            Graphics.DrawProcedural(MeshTopology.Triangles, 36, mpworld.particleNum);
        }
    }

    public void GBufferPass()
    {
        if (mpmaterial == null) { return; }
        foreach (MPWorld mpworld in mpworlds)
        {
            mpmaterial.SetBuffer("vertices", cbCubeVertices);
            mpmaterial.SetBuffer("particles", mpworld.GetDataBuffer());
            mpmaterial.SetInt("_FlipY", 0);
            mpmaterial.SetPass(enableDepthPrePass ? 2 : 0);
            Graphics.DrawProcedural(MeshTopology.Triangles, 36, mpworld.particleNum);
        }
    }
}
