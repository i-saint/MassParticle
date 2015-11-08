using System;
using System.Collections;
using UnityEngine;
using Ist;

public class Scanner : MonoBehaviour
{
    public MPWorld m_target;
    public float m_radius = 1.5f;
    Transform m_trans;
    Action m_mpupdate;
    MPHitHandler m_handler;

    void OnEnable()
    {
        m_trans = GetComponent<Transform>();
        if (m_mpupdate == null)
        {
            m_mpupdate = MPUpdate;
            m_handler = Handler;
        }
        m_target.AddUpdateRoutine(m_mpupdate);
    }

    void OnDisable()
    {
        m_target.RemoveUpdateRoutine(m_mpupdate);
    }

    void MPUpdate()
    {
        Vector3 pos = m_trans.position;
        //MPAPI.mpScanAABBParallel(m_target.GetContext(), m_handler, ref pos, ref scale);
        MPAPI.mpScanSphereParallel(m_target.GetContext(), m_handler, ref pos, m_radius);
        //MPAPI.mpScanSphere(m_target.GetContext(), m_handler, ref pos, m_radius);

        //Vector3 mv = new Vector3(0.0f, 0.0f, 0.01f);
        //MPAPI.mpMoveAll(m_target.GetContext(), ref mv);
    }

    void Handler(ref MPParticle particle)
    {
        particle.lifetime = 0.0f;
    }

    void OnDrawGizmos()
    {
        Transform t = GetComponent<Transform>(); // エディタから実行されるので trans は使えない
        Gizmos.color = Color.yellow;
        Gizmos.matrix = t.localToWorldMatrix;
        Gizmos.DrawWireSphere(Vector3.zero, m_radius);
        Gizmos.matrix = Matrix4x4.identity;
    }

}
