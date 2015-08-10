using UnityEngine;
using System.Collections;

public class Goal : MonoBehaviour
{
    public Material matLine;

    void Start()
    {

    }

    void Update()
    {

    }

    void OnGUI()
    {
        {
            Color blue = Color.blue;
            blue.a = 0.25f;
            Matrix4x4 mat = transform.localToWorldMatrix * Matrix4x4.Scale(new Vector3(1.0f, 1.0f, 0.0f));
            matLine.SetPass(0);
            MPGPPlayer.DrawWireCube(mat, blue);
        }
    }

    void OnDrawGizmos()
    {
        Gizmos.color = Color.blue;
        Gizmos.matrix = transform.localToWorldMatrix;
        Gizmos.DrawWireCube(Vector3.zero, Vector3.one);
        Gizmos.matrix = Matrix4x4.identity;
    }
}
