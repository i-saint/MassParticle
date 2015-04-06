using UnityEngine;
using System.Collections;
using System.Collections.Generic;

[AddComponentMenu("MassParticle/CapsuleCollider")]
public class MPCapsuleCollider : MPCollider
{
    public enum Direction
    {
        X,
        Y,
        Z,
    }
    public Direction m_direction = Direction.Y;
    public Vector3 m_center;
    public float m_radius = 0.5f;
    public float m_height = 0.5f;
    Vector4 m_pos1 = Vector4.zero;
    Vector4 m_pos2 = Vector4.zero;

    public override void MPUpdate()
    {
        Vector3 pos1_3 = m_pos1;
        Vector3 pos2_3 = m_pos2;
        base.MPUpdate();
        UpdateCapsule();
        EachTargets((w) =>
        {
            MPAPI.mpAddCapsuleCollider(w.GetContext(), ref m_cprops, ref pos1_3, ref pos2_3, m_radius);
        });
    }

    void UpdateCapsule()
    {
        switch (m_direction)
        {
            case Direction.X:
                m_pos1.Set(m_height * 0.5f + m_center.x, m_center.y, m_center.z, 1.0f);
                m_pos2.Set(-m_height * 0.5f + m_center.x, m_center.y, m_center.z, 1.0f);
                break;
            case Direction.Y:
                m_pos1.Set(m_center.x, m_height * 0.5f + m_center.y, m_center.z, 1.0f);
                m_pos2.Set(m_center.x, -m_height * 0.5f + m_center.y, m_center.z, 1.0f);
                break;
            case Direction.Z:
                m_pos1.Set(m_center.x, m_center.y, m_height * 0.5f + m_center.z, 1.0f);
                m_pos2.Set(m_center.x, m_center.y, -m_height * 0.5f + m_center.z, 1.0f);
                break;
        }
        m_pos1 = m_trans.localToWorldMatrix * m_pos1;
        m_pos2 = m_trans.localToWorldMatrix * m_pos2;
    }

    void OnDrawGizmos()
    {
        m_trans = GetComponent<Transform>();
        UpdateCapsule(); // エディタから実行される都合上必要
        Gizmos.color = MPImpl.ColliderGizmoColor;
        Gizmos.DrawWireSphere(m_pos1, m_radius);
        Gizmos.DrawWireSphere(m_pos2, m_radius);
        Gizmos.DrawLine(m_pos1, m_pos2);
        Gizmos.matrix = Matrix4x4.identity;
    }

}
