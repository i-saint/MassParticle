using UnityEngine;
using System.Collections;

public class DSMotionTrail : MonoBehaviour
{
    public int delayFrame = 4;
    MeshRenderer mesh_renderer;
    MaterialPropertyBlock property_block;
    Matrix4x4[] prevObjToWorld;

    void Start()
    {
        property_block = new MaterialPropertyBlock();
        property_block.AddMatrix("prev_Object2World", Matrix4x4.identity);
        mesh_renderer = GetComponent<MeshRenderer>();
        mesh_renderer.SetPropertyBlock(property_block);
    }

    void Update()
    {
        delayFrame = Mathf.Max(delayFrame, 1);
        if (prevObjToWorld == null || prevObjToWorld.Length != delayFrame)
        {
            prevObjToWorld = new Matrix4x4[delayFrame];
            for (int i = 0; i < prevObjToWorld.Length; ++i)
            {
                prevObjToWorld[i] = transform.localToWorldMatrix;
            }
        }
    }

    void OnWillRenderObject()
    {
        int last = prevObjToWorld.Length-1;
        for (int i = last; i > 0; --i)
        {
            prevObjToWorld[i] = prevObjToWorld[i - 1];
        }
        prevObjToWorld[0] = transform.localToWorldMatrix;
        property_block.AddMatrix("prev_Object2World", prevObjToWorld[last]);
        mesh_renderer.SetPropertyBlock(property_block);
    }
}
