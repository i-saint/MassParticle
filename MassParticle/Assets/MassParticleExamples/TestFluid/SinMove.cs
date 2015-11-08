using System.Collections;
using UnityEngine;
using Ist;

public class SinMove : MonoBehaviour
{
    public enum Axis
    {
        X,
        Y,
        Z
    }

    public Axis axis;
    public float scale = 1.0f;
    public float elapsed = 0.0f;
    public float moveAmount = 2.0f;


    void Update()
    {
        Vector3 pos = transform.position;
        if(transform.parent!=null) {
            pos = transform.parent.worldToLocalMatrix * pos;
        }

        float dt = Time.deltaTime * scale;
        float a1 = Mathf.Sin(elapsed);
        float a2 = Mathf.Sin(elapsed + dt);
        float a = (a2 - a1) * moveAmount;
        elapsed += dt;
        switch(axis) {
            case Axis.X: pos.x += a; break;
            case Axis.Y: pos.y += a; break;
            case Axis.Z: pos.z += a; break;
        }
        if (transform.parent != null)
        {
            pos = transform.parent.localToWorldMatrix * pos;
        }
        transform.position = pos;
    }
}
