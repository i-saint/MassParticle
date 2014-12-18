using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class MPCollider : MonoBehaviour
{
    public delegate void ParticleHitHandler(MPWorld world, MPCollider obj, ref MPParticle particle);
    public delegate void GatheredHitHandler(MPWorld world, MPCollider obj, ref MPHitData hit);

    public static List<MPCollider> instances = new List<MPCollider>();
    public static List<MPCollider> instances_prev = new List<MPCollider>();

    public MPWorld[] targets;

    public bool sendCollision = true;
    public bool receiveCollision = false;
    public float stiffness = 1500.0f;
    public float bounce = 1.0f;

    public ParticleHitHandler particleHitHandler = DefaultParticleHitHandler;
    public GatheredHitHandler gatheredHitHandler = DefaultGatheredHitHandler;
    public MPColliderProperties cprops;

    protected Transform trans;
    protected Rigidbody rigid3d;
    protected Rigidbody2D rigid2d;

    protected delegate void TargetEnumerator(MPWorld world);
    protected void EachTargets(TargetEnumerator e)
    {
        if (targets.Length != 0)
        {
            foreach (var w in targets)
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
        return instances_prev[id];
    }

    void Awake()
    {
        trans = GetComponent<Transform>();
        rigid3d = GetComponent<Rigidbody>();
        rigid2d = GetComponent<Rigidbody2D>();
    }

    void OnEnable()
    {
        instances.Add(this);
    }

    void OnDisable()
    {
        instances.Remove(this);
    }

    public virtual void MPUpdate()
    {
        cprops.group_mask = 0;
        cprops.stiffness = stiffness;
        cprops.bounce = bounce;
        cprops.damage_on_hit = 0.0f;
    }

    public static void MPUpdateAll()
    {
        int i = 0;
        foreach(var o in instances) {
            o.cprops.owner_id = i++;
            o.MPUpdate();
        }
        instances_prev = instances;
    }



    public static void DefaultParticleHitHandler(MPWorld world, MPCollider obj, ref MPParticle particle)
    {
        float force = world.force;
        Vector3 vel = particle.velocity3;

        if (obj.rigid3d)
        {
            obj.rigid3d.AddForceAtPosition(vel * force, particle.position3);
        }
        if (obj.rigid2d)
        {
            obj.rigid2d.AddForceAtPosition(vel * force, particle.position3);
        }
    }

    public static void DefaultGatheredHitHandler(MPWorld world, MPCollider obj, ref MPHitData hit)
    {
        float force = world.force;
        Vector3 vel = hit.velocity3;

        if (obj.rigid3d)
        {
            obj.rigid3d.AddForceAtPosition(vel * force, hit.position3);
        }
        if (obj.rigid2d)
        {
            obj.rigid2d.AddForceAtPosition(vel * force, hit.position3);
        }
    }
}
