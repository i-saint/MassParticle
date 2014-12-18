using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class MPEmitter : MonoBehaviour {

    static List<MPEmitter> instances = new List<MPEmitter>();

    public enum Shape {
        Sphere,
        Box,
    }

    public MPWorld[] targets;
    public Shape shape = Shape.Sphere;
    public Vector3 velosityBase = Vector3.zero;
    public float velosityDiffuse = 0.5f;
    public int emitCount = 8;


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
            foreach (var w in MPWorld.s_instances)
            {
                e(w);
            }
        }
    }


    void OnEnable()
    {
        instances.Add(this);
    }

    void OnDisable()
    {
        instances.Remove(this);
    }

    public void MPUpdate()
    {
        if (Time.deltaTime == 0.0f) { return; }
        Matrix4x4 mat = transform.localToWorldMatrix;
        switch (shape) {
        case Shape.Sphere:
            EachTargets((w) =>
            {
                MPAPI.mpScatterParticlesSphereTransform(w.GetContext(), ref mat, emitCount, ref velosityBase, velosityDiffuse);
            });
            break;

        case Shape.Box:
            EachTargets((w) =>
            {
                MPAPI.mpScatterParticlesBoxTransform(w.GetContext(), ref mat, emitCount, ref velosityBase, velosityDiffuse);
            });
            break;
        }
    }

    public static void MPUpdateAll()
    {
        foreach (var o in instances)
        {
            o.MPUpdate();
        }
    }

    void OnDrawGizmos()
    {
        Gizmos.color = Color.yellow;
        Gizmos.matrix = transform.localToWorldMatrix;
        switch(shape) {
        case Shape.Sphere:
            Gizmos.DrawWireSphere(Vector3.zero, 0.5f);
            break;
            
        case Shape.Box:
            Gizmos.color = Color.yellow;
            Gizmos.DrawWireCube(Vector3.zero, Vector3.one);
            break;
        }
        Gizmos.matrix = Matrix4x4.identity;
    }
}
