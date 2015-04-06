using UnityEngine;
using System.Collections;
using System.Collections.Generic;


[AddComponentMenu("GPUParticle/BoxCollider")]
public class MPGPBoxCollider : MPGPColliderBase
{
    public Vector3 m_size = Vector3.one;
    CSBoxCollider m_collider_data;

    public override void ActualUpdate()
    {
        CSImpl.BuildBoxCollider(ref m_collider_data, m_trans, m_size, m_id);
        EachTargets((t) => { t.AddBoxCollider(ref m_collider_data); });
    }

    void OnDrawGizmos()
    {
    }
}