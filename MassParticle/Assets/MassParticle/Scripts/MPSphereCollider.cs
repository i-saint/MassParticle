using UnityEngine;
using System.Collections;
using System.Collections.Generic;

[AddComponentMenu("MassParticle/SphereCollider")]
public class MPSphereCollider : MPCollider
{
    public float radius = 0.5f;


    public override void MPUpdate()
    {
        base.MPUpdate();
        Vector3 pos = m_trans.position;
        EachTargets((w) =>
        {
            MPAPI.mpAddSphereCollider(w.GetContext(), ref m_cprops, ref pos, m_trans.localScale.magnitude * 0.25f);
        });
    }

    void OnDrawGizmos()
    {
        Transform t = GetComponent<Transform>(); // エディタから実行されるので trans は使えない
        Gizmos.color = Color.cyan;
        Gizmos.matrix = t.localToWorldMatrix;
        Gizmos.DrawWireSphere(Vector3.zero, 0.5f);
        Gizmos.matrix = Matrix4x4.identity;
    }

}
