using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class MPCapsuleCollider : MPCollider
{
    public enum Direction
    {
        X,
        Y,
        Z,
    }
    public Direction m_direction = Direction.Y;
    float m_radius = 0.0f;
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
        float h = 0.0f;
        switch (m_direction)
        {
            case Direction.X:
                m_radius = (m_trans.localScale.y + m_trans.localScale.z) * 0.5f * 0.5f;
                h = Mathf.Max(0.0f, m_trans.localScale.x - m_radius * 2.0f);
                m_pos1.Set(h * 0.5f, 0.0f, 0.0f, 1.0f);
                m_pos2.Set(-h * 0.5f, 0.0f, 0.0f, 1.0f);
                break;
            case Direction.Y:
                m_radius = (m_trans.localScale.x + m_trans.localScale.z) * 0.5f * 0.5f;
                h = Mathf.Max(0.0f, m_trans.localScale.y - m_radius * 2.0f);
                m_pos1.Set(0.0f, h * 0.5f, 0.0f, 1.0f);
                m_pos2.Set(0.0f, -h * 0.5f, 0.0f, 1.0f);
                break;
            case Direction.Z:
                m_radius = (m_trans.localScale.x + m_trans.localScale.y) * 0.5f * 0.5f;
                h = Mathf.Max(0.0f, m_trans.localScale.z - m_radius * 2.0f);
                m_pos1.Set(0.0f, 0.0f, h * 0.5f, 1.0f);
                m_pos2.Set(0.0f, 0.0f, -h * 0.5f, 1.0f);
                break;
        }
        m_pos1 = m_trans.localToWorldMatrix * m_pos1;
        m_pos2 = m_trans.localToWorldMatrix * m_pos2;
    }

    void OnDrawGizmos()
    {
        m_trans = GetComponent<Transform>();
        UpdateCapsule(); // エディタから実行される都合上必要
        Gizmos.color = Color.yellow;
        Gizmos.DrawWireSphere(m_pos1, m_radius);
        Gizmos.DrawWireSphere(m_pos2, m_radius);
        Gizmos.DrawLine(m_pos1, m_pos2);
        Gizmos.matrix = Matrix4x4.identity;
    }

}
