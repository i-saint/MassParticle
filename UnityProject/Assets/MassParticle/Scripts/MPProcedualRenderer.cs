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
    public MPWorld[] targets;
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
            dsr.AddCallbackPreGBuffer(() => { UpdateBuffer(); }, 0);
            dsr.AddCallbackPreGBuffer(() => { DepthPrePass(); });
            dsr.AddCallbackPostGBuffer(() => { GBufferPass(); }, 10);
        }
    }

    void OnDestroy()
    {
        cbCubeVertices.Release();
    }


    delegate void TargetEnumerator(MPWorld world);
    void EachTargets(TargetEnumerator e)
    {
        if (targets.Length != 0)
        {
            foreach (var w in targets)
            {
                e(w);
            }
        }
        else
        {
            foreach (var w in MPWorld.instances)
            {
                e(w);
            }
        }
    }

    public void UpdateBuffer()
    {
        EachTargets((w) =>
        {
            w.GetDataBuffer();
        });
    }

    public void DepthPrePass()
    {
        if (!enableDepthPrePass) { return; }
        EachTargets((w) =>
        {
            Material mat = w.mat;
            if (mat == null) { return; }
            mat.SetBuffer("vertices", cbCubeVertices);
            mat.SetBuffer("particles", w.GetDataBuffer());
            mat.SetInt("_FlipY", 1);
            mat.SetPass(1);
            Graphics.DrawProcedural(MeshTopology.Triangles, 36, w.particleNum);
        });
    }

    public void GBufferPass()
    {
        EachTargets((w) =>
        {
            Material mat = w.mat;
            if (mat == null) { return; }
            mat.SetBuffer("vertices", cbCubeVertices);
            mat.SetBuffer("particles", w.GetDataBuffer());
            mat.SetInt("_FlipY", 0);
            mat.SetPass(enableDepthPrePass ? 2 : 0);
            Graphics.DrawProcedural(MeshTopology.Triangles, 36, w.particleNum);
        });
    }
}
