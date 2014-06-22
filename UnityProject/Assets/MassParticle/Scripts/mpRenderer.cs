using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public unsafe class mpRenderer : MonoBehaviour
{
	public enum RenderMode {
		Plugin,
		Points,
		Cubes
	}

	mp.mpMeshData meshData;
	List<GameObject> children;
	public RenderMode renderMode;
	public GameObject childPrefab;
	public Material material;
	RenderMode renderModePrev;

	const int dataTextureWidth = 3072;
	const int dataTextureHeight = 2048;
	RenderTexture dataTexture;


	mpRenderer()
	{
	}

	void Start () {
		meshData = new mp.mpMeshData();
		children = new List<GameObject>();
		dataTexture = new RenderTexture(dataTextureWidth, dataTextureHeight, 0, RenderTextureFormat.ARGBFloat, RenderTextureReadWrite.Default);
		dataTexture.isPowerOfTwo = false;
		dataTexture.filterMode = FilterMode.Point;
		dataTexture.Create();
		if (material)
		{
			material.SetTexture("_DataTex", dataTexture);
			material.SetFloat("_DataTexPitch", 1.0f / dataTextureWidth);
		}
	}

	void Update()
	{
		if (renderMode != renderModePrev)
		{
			foreach(GameObject child in children) {
				Destroy(child);
			}
			children.Clear();
		}
		renderModePrev = renderMode;

		int num = mp.mpGetNumParticles();
		if (num == 0) { return; }

		switch (renderMode)
		{
			case RenderMode.Points:
				UpdatePointMeshes();
				mp.mpUpdateDataTexture(dataTexture.GetNativeTexturePtr());
				break;
			case RenderMode.Cubes:
				UpdateCubeMeshes();
				mp.mpUpdateDataTexture(dataTexture.GetNativeTexturePtr());
				break;
		}
	}

	void UpdateCubeMeshes()
	{
		int numParticles = mp.mpGetNumParticles();
		int numChildren = numParticles / 2700 + (numParticles % 2700 == 0 ? 0 : 1);
		while (children.Count < numChildren)
		{
			GameObject child = (GameObject)Instantiate(childPrefab, Vector3.zero, Quaternion.identity);
			Vector3[] vertices = new Vector3[64800];
			Vector3[] normals = new Vector3[64800];
			Vector2[] uv = new Vector2[64800];
			int[] indices = new int[97200];
			fixed (Vector3* v = vertices) {
			fixed (Vector3* n = normals) {
			fixed (Vector2* t = uv) {
			fixed (int* idx = indices) {
				meshData.vertices = v;
				meshData.normals = n;
				meshData.uv = t;
				meshData.indices = idx;
				mp.mpGenerateCubeMesh(children.Count, ref meshData);
			}}}}
			Mesh mesh = child.GetComponent<MeshFilter>().mesh;
			mesh.vertices = vertices;
			mesh.normals = normals;
			mesh.uv = uv;
			mesh.SetIndices(indices, MeshTopology.Triangles, 0);
			children.Add(child);
		}
		for (int i = 0; i < numChildren; ++i)
		{
			GameObject child = children[i];
			MeshRenderer renderer = child.GetComponent<MeshRenderer>();
			renderer.enabled = true;
			renderer.material = material;
		}
		for (int i = numChildren; i < children.Count; ++i)
		{
			children[i].GetComponent<MeshRenderer>().enabled = false;
		}
	}

	void UpdatePointMeshes()
	{
		int numParticles = mp.mpGetNumParticles();
		int numChildren = numParticles / 65000 + (numParticles % 65000 == 0 ? 0 : 1);
		while (children.Count < numChildren)
		{
			GameObject child = (GameObject)Instantiate(childPrefab, Vector3.zero, Quaternion.identity);
			Vector3[] vertices = new Vector3[65000];
			Vector2[] uv = new Vector2[65000];
			int[] indices = new int[65000];
			fixed (Vector3* v = vertices) {
			fixed (Vector2* t = uv) {
			fixed (int* idx = indices) {
				meshData.vertices = v;
				meshData.uv = t;
				meshData.indices = idx;
				mp.mpGeneratePointMesh(children.Count, ref meshData);
			}}}
			Mesh mesh = child.GetComponent<MeshFilter>().mesh;
			mesh.vertices = vertices;
			mesh.uv = uv;
			mesh.SetIndices(indices, MeshTopology.Points, 0);
			children.Add(child);
		}
		for (int i = 0; i < numChildren; ++i)
		{
			GameObject child = children[i];
			MeshRenderer renderer = child.GetComponent<MeshRenderer>();
			renderer.enabled = true;
			renderer.material = material;
		}
		for (int i = numChildren; i < children.Count; ++i)
		{
			children[i].GetComponent<MeshRenderer>().enabled = false;
		}
	}

	void OnRenderObject()
	{
		if (renderMode == RenderMode.Plugin)
		{
			UnityEngine.Camera cam = UnityEngine.Camera.current;
			if (cam)
			{
				mp.mpSetViewProjectionMatrix(cam.worldToCameraMatrix, cam.projectionMatrix, cam.transform.position);
			}
			GL.IssuePluginEvent(1);
		}
	}
}
