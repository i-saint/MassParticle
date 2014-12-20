using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public unsafe class MPRenderer : MonoBehaviour
{
    public enum RenderMode {
        Points,
        Cubes,
    }

    public MPWorld world;
    public RenderMode renderMode = RenderMode.Cubes;
    public Material material;
    public float size = 0.2f;
    public bool castShadows = true;
    public bool receiveShadows = true;

    MaterialPropertyBlock mpb;
    MPMeshData mesh_data;
    List<GameObject> children;
    public RenderTexture data_texture;
    GameObject meshes;
    Bounds bounds;


    void OnEnable()
    {
        if (meshes == null)
        {
            meshes = new GameObject("MPMeshes");
            mesh_data = new MPMeshData();
            children = new List<GameObject>();
        }
        if (data_texture == null || !data_texture.IsCreated())
        {
            data_texture = new RenderTexture(MPWorld.DataTextureWidth, MPWorld.DataTextureHeight, 0, RenderTextureFormat.ARGBFloat, RenderTextureReadWrite.Default);
            data_texture.filterMode = FilterMode.Point;
            data_texture.Create();
        }
        if (mpb == null)
        {
            mpb = new MaterialPropertyBlock();
        }
    }

    void Update()
    {
        Transform t = world.GetComponent<Transform>();
        Vector3 min = t.position - t.localScale;
        Vector3 max = t.position + t.localScale;
        bounds.SetMinMax(min, max);

        switch (renderMode)
        {
            case RenderMode.Points:
                UpdatePointMeshes();
                break;
            case RenderMode.Cubes:
                UpdateCubeMeshes();
                break;
        }
    }

    GameObject CreateChildMesh()
    {
        GameObject child = new GameObject("MPMesh");
        child.transform.parent = meshes.transform;
        child.AddComponent<MeshFilter>();
        child.AddComponent<MeshRenderer>();
        return child;
    }

    void UpdateCubeMeshes()
    {
        int batch_size = MPWorld.CubeBatchSize;
        int num_particles = MPAPI.mpGetNumParticles(world.GetContext());
        int num_active_children = num_particles / batch_size + (num_particles % batch_size == 0 ? 0 : 1);
        while (children.Count < num_active_children)
        {
            GameObject child = CreateChildMesh();
            Vector3[] vertices = new Vector3[batch_size * 24];
            Vector3[] normals = new Vector3[batch_size * 24];
            Vector2[] uv = new Vector2[batch_size * 24];
            int[] indices = new int[batch_size * 36];
            fixed (Vector3* v = vertices) {
            fixed (Vector3* n = normals) {
            fixed (Vector2* t = uv) {
            fixed (int* idx = indices) {
                mesh_data.vertices = v;
                mesh_data.normals = n;
                mesh_data.uv = t;
                mesh_data.indices = idx;
                MPAPI.mpGenerateCubeMesh(world.GetContext(), children.Count, ref mesh_data);
            }}}}
            Mesh mesh = child.GetComponent<MeshFilter>().mesh;
            mesh.vertices = vertices;
            mesh.normals = normals;
            mesh.uv = uv;
            mesh.SetIndices(indices, MeshTopology.Triangles, 0);
            children.Add(child);
        }
        UpdateChildMeshes(num_active_children);
    }

    void UpdatePointMeshes()
    {
        int batch_size = MPWorld.PointBatchSize;
        int num_particles = MPAPI.mpGetNumParticles(world.GetContext());
        int num_active_children = num_particles / batch_size + (num_particles % batch_size == 0 ? 0 : 1);
        while (children.Count < num_active_children)
        {
            GameObject child = CreateChildMesh();
            Vector3[] vertices = new Vector3[batch_size];
            Vector2[] uv = new Vector2[batch_size];
            int[] indices = new int[batch_size];
            fixed (Vector3* v = vertices) {
            fixed (Vector2* t = uv) {
            fixed (int* idx = indices) {
                mesh_data.vertices = v;
                mesh_data.uv = t;
                mesh_data.indices = idx;
                MPAPI.mpGeneratePointMesh(world.GetContext(), children.Count, ref mesh_data);
            }}}
            Mesh mesh = child.GetComponent<MeshFilter>().mesh;
            mesh.vertices = vertices;
            mesh.uv = uv;
            mesh.SetIndices(indices, MeshTopology.Points, 0);
            children.Add(child);
        }
        UpdateChildMeshes(num_active_children);
    }



    void UpdateChildMeshes(int num_active_children)
    {
        for (int i = 0; i < num_active_children; ++i)
        {
            GameObject child = children[i];
            Mesh mesh = child.GetComponent<MeshFilter>().mesh;
            MeshRenderer renderer = child.GetComponent<MeshRenderer>();
            renderer.enabled = true;
            renderer.castShadows = castShadows;
            renderer.receiveShadows = receiveShadows;
            renderer.material = material;
            renderer.SetPropertyBlock(mpb);
            mesh.bounds = bounds;
        }
        for (int i = num_active_children; i < children.Count; ++i)
        {
            children[i].GetComponent<MeshRenderer>().enabled = false;
        }
    }

    void OnPreRender()
    {
        world.UpdateDataTexture(data_texture);
        mpb.SetTexture("_DataTex", data_texture);
        mpb.SetFloat("_DataTexPitch", 1.0f / MPWorld.DataTextureWidth);
        mpb.SetFloat("_ParticleSize", size);
    }

}
