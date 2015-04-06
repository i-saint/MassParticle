using UnityEngine;
using System.Collections;
using System.Collections.Generic;


public class MPGPSphereCollider : MPGPColliderBase
{
    public float m_radius = 0.5f;
    CSSphereCollider m_collider_data;

    public override void ActualUpdate()
    {
        CSImpl.BuildSphereCollider(ref m_collider_data, m_trans, m_radius, m_id);
        EachTargets((t) => { t.AddSphereCollider(ref m_collider_data); });
    }

    void OnDrawGizmos()
    {
    }
}
