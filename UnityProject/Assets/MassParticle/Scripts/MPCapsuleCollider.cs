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
    public Direction direction = Direction.Y;
    float radius = 0.0f;
    Vector4 pos1 = Vector4.zero;
    Vector4 pos2 = Vector4.zero;

    public override void MPUpdate()
    {
        Vector3 pos1_3 = pos1;
        Vector3 pos2_3 = pos2;
        base.MPUpdate();
        UpdateCapsule();
        EachTargets((w) =>
        {
            MPAPI.mpAddCapsuleCollider(w.GetContext(), ref cprops, ref pos1_3, ref pos2_3, radius);
        });
    }

    void UpdateCapsule()
    {
        float h = 0.0f;
        switch (direction)
        {
            case Direction.X:
                radius = (m_trans.localScale.y + m_trans.localScale.z) * 0.5f * 0.5f;
                h = Mathf.Max(0.0f, m_trans.localScale.x - radius * 2.0f);
                pos1.Set(h * 0.5f, 0.0f, 0.0f, 1.0f);
                pos2.Set(-h * 0.5f, 0.0f, 0.0f, 1.0f);
                break;
            case Direction.Y:
                radius = (m_trans.localScale.x + m_trans.localScale.z) * 0.5f * 0.5f;
                h = Mathf.Max(0.0f, m_trans.localScale.y - radius * 2.0f);
                pos1.Set(0.0f, h * 0.5f, 0.0f, 1.0f);
                pos2.Set(0.0f, -h * 0.5f, 0.0f, 1.0f);
                break;
            case Direction.Z:
                radius = (m_trans.localScale.x + m_trans.localScale.y) * 0.5f * 0.5f;
                h = Mathf.Max(0.0f, m_trans.localScale.z - radius * 2.0f);
                pos1.Set(0.0f, 0.0f, h * 0.5f, 1.0f);
                pos2.Set(0.0f, 0.0f, -h * 0.5f, 1.0f);
                break;
        }
        pos1 = m_trans.localToWorldMatrix * pos1;
        pos2 = m_trans.localToWorldMatrix * pos2;
    }

    void OnDrawGizmos()
    {
        m_trans = GetComponent<Transform>();
        UpdateCapsule(); // エディタから実行される都合上必要
        Gizmos.color = Color.yellow;
        Gizmos.DrawWireSphere(pos1, radius);
        Gizmos.DrawWireSphere(pos2, radius);
        Gizmos.DrawLine(pos1, pos2);
        Gizmos.matrix = Matrix4x4.identity;
    }

}
