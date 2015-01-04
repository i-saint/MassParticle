using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class MPBoxCollider : MPCollider
{
    public override void MPUpdate()
    {
        base.MPUpdate();

        Matrix4x4 mat = m_trans.localToWorldMatrix;
        Vector3 one = Vector3.one;
        EachTargets((w) =>
        {
            MPAPI.mpAddBoxCollider(w.GetContext(), ref m_cprops, ref mat, ref one);
        });
    }

    void OnDrawGizmos()
    {
        Transform t = GetComponent<Transform>(); // エディタから実行されるので trans は使えない
        Gizmos.color = Color.yellow;
        Gizmos.matrix = t.localToWorldMatrix;
        Gizmos.DrawWireCube(Vector3.zero, Vector3.one);
        Gizmos.matrix = Matrix4x4.identity;
    }

}
