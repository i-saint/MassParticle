using UnityEngine;
using System.Collections;
using System.Collections.Generic;


public class MPGPSphereCollider : MPGPColliderBase
{
    public float m_radius = 0.5f;
    MPGPSphereColliderData m_collider_data;

    public override void ActualUpdate()
    {
        MPGPImpl.BuildSphereCollider(ref m_collider_data, m_trans, m_radius, m_id);
        EachTargets((t) => { t.AddSphereCollider(ref m_collider_data); });
    }

    void OnDrawGizmos()
    {
        Transform t = GetComponent<Transform>(); // エディタから実行されるので trans は使えない
        Gizmos.color = MPGPImpl.ColliderGizmoColor;
        Gizmos.matrix = t.localToWorldMatrix;
        Gizmos.DrawWireSphere(Vector3.zero, 0.5f);
        Gizmos.matrix = Matrix4x4.identity;
    }

}
