using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class MPCollider : MonoBehaviour
{
    public delegate void ParticleHitHandler(MPWorld world, MPCollider obj, ref MPParticle particle);
    public delegate void GatheredHitHandler(MPWorld world, MPCollider obj, ref MPForceData hit);

    public static List<MPCollider> s_instances = new List<MPCollider>();
    public static List<MPCollider> s_instances_prev = new List<MPCollider>();

    public MPWorld[] m_targets;

    public bool m_receive_hit = false;
    public bool m_receive_force = false;
    public float m_stiffness = 1500.0f;

    public MPParticleHandler m_hit_handler;
    public MPForceHandler m_force_handler;
    public MPColliderProperties cprops;

    protected Transform m_trans;
    protected Rigidbody m_rigid3d;
    protected Rigidbody2D m_rigid2d;

    protected delegate void TargetEnumerator(MPWorld world);
    protected void EachTargets(TargetEnumerator e)
    {
        if (m_targets.Length != 0)
        {
            foreach (var w in m_targets)
            {
                e(w);
            }
        }
        else
        {
            foreach (var w in MPWorld.s_instances)
            {
                e(w);
            }
        }
    }


    public static MPCollider GetHitOwner(int id)
    {
        if (id == -1) { return null; }
        return s_instances_prev[id];
    }

    void Awake()
    {
        m_trans = GetComponent<Transform>();
        m_rigid3d = GetComponent<Rigidbody>();
        m_rigid2d = GetComponent<Rigidbody2D>();
        if (m_hit_handler == null) m_hit_handler = DefaultHitHandler;
        if (m_force_handler == null) m_force_handler = DefaultForceHandler;
    }

    void OnEnable()
    {
        s_instances.Add(this);
    }

    void OnDisable()
    {
        s_instances.Remove(this);
    }

    public virtual void MPUpdate()
    {
        cprops.stiffness = m_stiffness;
        cprops.hit_handler = m_receive_hit ? m_hit_handler : null;
        cprops.force_handler = m_receive_force ? m_force_handler : null;
    }

    public static void MPUpdateAll()
    {
        int i = 0;
        foreach(var o in s_instances) {
            o.cprops.owner_id = i++;
            o.MPUpdate();
        }
        s_instances_prev = s_instances;
    }



    public void DefaultHitHandler(ref MPParticle particle)
    {
        //Debug.Log("DefaultHitHandler(): " + GetHashCode());

        float mass = 0.5f;
        Vector3 vel = particle.velocity3;
        particle.lifetime = 0.0f;

        if (m_rigid3d)
        {
            m_rigid3d.AddForceAtPosition(vel * mass, particle.position3);
        }
        if (m_rigid2d)
        {
            m_rigid2d.AddForceAtPosition(vel * mass, particle.position3);
        }
    }

    public void DefaultForceHandler(ref MPForceData force)
    {
        //Debug.Log("DefaultForceHandler(): " + GetHashCode());

        Vector3 pos = force.position;
        Vector3 vel = force.velocity;

        if (m_rigid3d)
        {
            m_rigid3d.AddForceAtPosition(vel, pos);
        }
        if (m_rigid2d)
        {
            m_rigid2d.AddForceAtPosition(vel, pos);
        }
    }
}
