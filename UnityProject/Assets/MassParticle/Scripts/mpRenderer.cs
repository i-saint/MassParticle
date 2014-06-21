using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public unsafe class mpRenderer : MonoBehaviour
{
	public struct ChildMeshData
	{
		public Vector3[] vertices;
		public Vector3[] normals;
		public Vector2[] uv;
		public Color[] colors;
		public int[] indices;
		public GameObject obj;
	}

	public enum RenderMode {
		Plugin,
		Points,
		Cubes
	}

	mp.mpMeshData meshData;
	List<ChildMeshData> children;
	public RenderMode renderMode;
	public GameObject childPrefab;
	public Material material;
	RenderMode renderModePrev;

	mpRenderer()
	{
	}

	void Start () {
		meshData = new mp.mpMeshData();
		children = new List<ChildMeshData>();
	}

	void Update()
	{
		if (renderMode != renderModePrev)
		{
			foreach(ChildMeshData cmd in children) {
				Destroy(cmd.obj);
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
				break;
			case RenderMode.Cubes:
				UpdateCubeMeshes();
				break;
		}
	}

	void UpdateCubeMeshes()
	{
		int numParticles = mp.mpGetNumParticles();
		int numChildren = numParticles / 2500 + (numParticles % 2500 == 0 ? 0 : 1);
		while (children.Count < numChildren)
		{
			ChildMeshData child = new ChildMeshData();
			child.vertices = new Vector3[60000];
			child.normals = new Vector3[60000];
			child.uv = new Vector2[60000];
			child.colors = new Color[60000];
			child.indices = new int[90000];
			child.obj = (GameObject)Instantiate(childPrefab, Vector3.zero, Quaternion.identity);
			children.Add(child);
		}
		for (int i = 0; i < numChildren; ++i)
		{
			ChildMeshData child = children[i];
			fixed (Vector3* v = child.vertices) {
			fixed (Vector3* n = child.normals) {
			fixed (Vector2* t = child.uv) {
			fixed (Color* c = child.colors) {
			fixed (int* idx = child.indices) {
				meshData.vertices = v;
				meshData.normals = n;
				meshData.uv = t;
				meshData.colors = c;
				meshData.indices = idx;
				mp.mpGenerateCubeMesh(i, ref meshData);
			}}}}}
			Mesh mesh = child.obj.GetComponent<MeshFilter>().mesh;
			mesh.vertices = child.vertices;
			mesh.normals = child.normals;
			//mesh.uv = child.uv;
			//mesh.colors = child.colors;
			mesh.SetIndices(child.indices, MeshTopology.Triangles, 0);

			MeshRenderer renderer = child.obj.GetComponent<MeshRenderer>();
			renderer.enabled = true;
			renderer.material = material;
		}
		for (int i = numChildren; i < children.Count; ++i)
		{
			children[i].obj.GetComponent<MeshRenderer>().enabled = false;
		}
	}

	void UpdatePointMeshes()
	{
		int numParticles = mp.mpGetNumParticles();
		int numChildren = numParticles / 65000 + (numParticles % 65000 == 0 ? 0 : 1);
		while (children.Count < numChildren)
		{
			ChildMeshData child = new ChildMeshData();
			child.vertices = new Vector3[65000];
			child.normals = new Vector3[65000];
			child.uv = new Vector2[65000];
			child.colors = new Color[65000];
			child.indices = new int[65000];
			child.obj = (GameObject)Instantiate(childPrefab, Vector3.zero, Quaternion.identity);
			children.Add(child);
		}
		for (int i = 0; i < numChildren; ++i)
		{
			ChildMeshData child = children[i];
			fixed (Vector3* v = child.vertices) {
			fixed (Vector3* n = child.normals) {
			fixed (Vector2* t = child.uv) {
			fixed (Color* c = child.colors) {
			fixed (int* idx = child.indices) {
				meshData.vertices = v;
				meshData.normals = n;
				meshData.uv = t;
				meshData.colors = c;
				meshData.indices = idx;
				mp.mpGeneratePointMesh(i, ref meshData);
			}}}}}
			Mesh mesh = child.obj.GetComponent<MeshFilter>().mesh;
			mesh.vertices = child.vertices;
			mesh.normals = child.normals;
			//mesh.uv = child.uv;
			//mesh.colors = child.colors;
			mesh.SetIndices(child.indices, MeshTopology.Points, 0);

			MeshRenderer renderer = child.obj.GetComponent<MeshRenderer>();
			renderer.enabled = true;
			renderer.material = material;
		}
		for (int i = numChildren; i < children.Count; ++i)
		{
			children[i].obj.GetComponent<MeshRenderer>().enabled = false;
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
