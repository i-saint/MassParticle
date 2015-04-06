using UnityEngine;
using System.Collections;
using System.Collections.Generic;


[AddComponentMenu("GPUParticle/BoxCollider")]
public class MPGPBoxCollider : MPGPColliderBase
{
    public Vector3 m_size = Vector3.one;
    MPGPBoxColliderData m_collider_data;

    public override void ActualUpdate()
    {
        MPGPImpl.BuildBoxCollider(ref m_collider_data, m_trans, m_size, m_id);
        EachTargets((t) => { t.AddBoxCollider(ref m_collider_data); });
    }

    void OnDrawGizmos()
    {
        Transform t = GetComponent<Transform>(); // エディタから実行されるので trans は使えない
        Gizmos.color = MPGPImpl.ColliderGizmoColor;
        Gizmos.matrix = t.localToWorldMatrix;
        Gizmos.DrawWireCube(Vector3.zero, Vector3.one);
        Gizmos.matrix = Matrix4x4.identity;
    }

}