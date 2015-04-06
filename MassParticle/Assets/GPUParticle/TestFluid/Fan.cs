using UnityEngine;
using System.Collections;

public class Fan : MonoBehaviour
{
    public float range = 5.0f;
    public float strength = 5.0f;
    public Material matLine;
    Matrix4x4 forceMatrix;


    // Use this for initialization
    void Start () {
    
    }

    void Update()
    {
        Vector3 s = transform.localScale;
        Matrix4x4 bt = Matrix4x4.identity;
        bt.SetColumn(3, new Vector4(0.0f, 0.0f, 0.5f, 1.0f));
        bt = Matrix4x4.Scale(new Vector3(s.x, s.y, s.z * range)) * bt;
        forceMatrix = transform.localToWorldMatrix * bt;

        CSForce force = new CSForce();
        force.info.shape_type = CSForceShape.Box;
        force.info.dir_type = CSForceDirection.Directional;
        force.info.strength = strength;
        force.info.direction = transform.forward;
        CSImpl.BuildBox(ref force.box, forceMatrix, Vector3.one);
        MPGPWorld.GetInstances().ForEach((t) => { t.AddForce(ref force); });

        foreach (Transform child in transform)
        {
            child.Rotate(new Vector3(0.0f, 0.0f, 1.0f), strength * 0.33f);
        }
    }

    void OnGUI()
    {
        Color blue = Color.blue;
        blue.a = 0.25f;
        Matrix4x4 mat = forceMatrix * Matrix4x4.Scale(new Vector3(0.0f, 1.0f, 1.0f));
        matLine.SetPass(0);
        MPGPPlayer.DrawWireCube(mat, blue);
    }
}
