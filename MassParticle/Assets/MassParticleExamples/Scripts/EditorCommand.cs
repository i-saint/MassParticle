#if UNITY_EDITOR
using System.IO;
using UnityEngine;
using UnityEditor;
using Ist;


public static class ScriptableObjectUtility
{
    /// <summary>
    //	This makes it easy to create, name and place unique new ScriptableObject asset files.
    /// </summary>
    public static void CreateAsset<T> () where T : ScriptableObject
    {
        T asset = ScriptableObject.CreateInstance<T>();
 
        string path = AssetDatabase.GetAssetPath(Selection.activeObject);
        if (path == "") 
        {
            path = "Assets";
        } 
        else if (Path.GetExtension (path) != "") 
        {
            path = path.Replace (Path.GetFileName (AssetDatabase.GetAssetPath (Selection.activeObject)), "");
        }
        string assetPathAndName = AssetDatabase.GenerateUniqueAssetPath(path + "/" + typeof(T).ToString() + ".asset");
        AssetDatabase.CreateAsset(asset, assetPathAndName);
        AssetDatabase.SaveAssets();
        AssetDatabase.Refresh();
        EditorUtility.FocusProjectWindow ();
        Selection.activeObject = asset;
    }
}

public class EditorCommand
{

    static Vector2 GenTexcoord(int nth)
    {
        const float xd = 1.0f / MPWorld.DataTextureWidth;
        const float yd = 1.0f / MPWorld.DataTextureHeight;
        int xi = (nth * 3) % MPWorld.DataTextureWidth;
        int yi = (nth * 3) / MPWorld.DataTextureHeight;
        return new Vector2(xd * xi + xd * 0.5f, yd * yi + yd * 0.5f);
    }

    static void BuildCubeMesh(Mesh mesh, int mi)
    {
        const float s = 0.01f;
        const float p = 1.0f;
        const float n = -1.0f;
        const float z = 0.0f;

        Vector3[] c_vertices = new Vector3[]
        {
            new Vector3(-s,-s, s), new Vector3( s,-s, s), new Vector3( s, s, s), new Vector3(-s, s, s),
            new Vector3(-s, s,-s), new Vector3( s, s,-s), new Vector3( s, s, s), new Vector3(-s, s, s),
            new Vector3(-s,-s,-s), new Vector3( s,-s,-s), new Vector3( s,-s, s), new Vector3(-s,-s, s),
            new Vector3(-s,-s, s), new Vector3(-s,-s,-s), new Vector3(-s, s,-s), new Vector3(-s, s, s),
            new Vector3( s,-s, s), new Vector3( s,-s,-s), new Vector3( s, s,-s), new Vector3( s, s, s),
            new Vector3(-s,-s,-s), new Vector3( s,-s,-s), new Vector3( s, s,-s), new Vector3(-s, s,-s),
        };
        Vector3[] c_normals = new Vector3[]
        {
            new Vector3(z, z, p), new Vector3(z, z, p), new Vector3(z, z, p), new Vector3(z, z, p),
            new Vector3(z, p, z), new Vector3(z, p, z), new Vector3(z, p, z), new Vector3(z, p, z),
            new Vector3(z, n, z), new Vector3(z, n, z), new Vector3(z, n, z), new Vector3(z, n, z),
            new Vector3(n, z, z), new Vector3(n, z, z), new Vector3(n, z, z), new Vector3(n, z, z),
            new Vector3(p, z, z), new Vector3(p, z, z), new Vector3(p, z, z), new Vector3(p, z, z),
            new Vector3(z, z, n), new Vector3(z, z, n), new Vector3(z, z, n), new Vector3(z, z, n),
        };
        int[] c_indices = new int[] 
        {
            0,1,3, 3,1,2,
            5,4,6, 6,4,7,
            8,9,11, 11,9,10,
            13,12,14, 14,12,15,
            16,17,19, 19,17,18,
            21,20,22, 22,20,23,
        };

        Vector3[] vertices = new Vector3[64800];
        Vector3[] normals = new Vector3[64800];
        Vector2[] uv = new Vector2[64800];
        int[] indices = new int[97200];

        int begin = MPWorld.CubeBatchSize * mi;
        int end = MPWorld.CubeBatchSize * (mi + 1);
        for (int pi = begin; pi != end; ++pi) {
            int si = pi - begin;
            Vector2 t = GenTexcoord(pi);
            for (int vi = 0; vi < 24; ++vi) {
                vertices[24 * si + vi] = c_vertices[vi];
                normals[24 * si + vi] = c_normals[vi];
                uv[24 * si + vi] = t;
            }
            for (int vi = 0; vi < 36; ++vi) {
                indices[36 * si + vi] = 24 * si + c_indices[vi];
            }
        }

        mesh.vertices = vertices;
        mesh.normals = normals;
        mesh.uv = uv;
        mesh.triangles = indices;
    }
}
#endif
