using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters.Binary;
using System.IO;

[Serializable]
public struct MPContext
{
    public int context;
}

public unsafe class MPWorld : MonoBehaviour
{
    public static List<MPWorld> s_instances = new List<MPWorld>();
    public static MPWorld s_current;
    static int s_update_count = 0;

    void EachWorld(Action<MPWorld> f)
    {
        s_instances.ForEach(f);
    }



    public delegate void ParticleProcessor(MPWorld world, int numParticles, MPParticle* particles);
    public delegate void GatheredHitProcessor(MPWorld world, int numColliders, MPForceData* hits);

    public const int CubeBatchSize = 2700;
    public const int PointBatchSize = 65000;
    public const int DataTextureWidth = 3072;
    public const int DataTextureHeight = 256;

    public MPUpdateMode m_update_mode = MPUpdateMode.Deferred;
    public MPSolverType m_solver = MPSolverType.Impulse;
    public bool m_enable_interaction = true;
    public bool m_enable_colliders = true;
    public bool m_enable_orces = true;
    public float m_particle_mass = 0.1f;
    public float m_particle_lifetime = 30.0f;
    public float m_timescale = 0.6f;
    public float m_deceleration = 0.99f;
    public float m_advection = 0.1f;
    public float m_pressure_stiffness = 500.0f;
    public float m_wall_stiffness = 1500.0f;
    public float m_particle_size = 0.08f;
    public int m_max_particle_num = 100000;
    public Vector3 m_coord_scale = Vector3.one;
    public int m_world_div_x = 256;
    public int m_world_div_y = 1;
    public int m_world_div_z = 256;
    public int m_particle_num = 0;
    public MPContext m_context;



    public IntPtr GetContext() { return (IntPtr)m_context.context; }


    public int UpdateDataTexture(RenderTexture rt)
    {
        m_particle_num = MPAPI.mpUpdateDataTexture(GetContext(), rt.GetNativeTexturePtr());
        return m_particle_num;
    }



    MPWorld()
    {
#if UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
        MPAPI.mphInitialize();
#endif
    }


    void Awake()
    {
        s_instances.Add(this);
        m_context.context = (int)MPAPI.mpCreateContext();
    }


    void OnDestroy()
    {
        MPAPI.mpDestroyContext(GetContext());
        s_instances.Remove(this);
    }

    void Update()
    {
        //if(Time.frameCount%10==0)
        //    Debug.Log("MPWorld: "+GetContext());

        if (s_update_count++ == 0)
        {
            if (Time.deltaTime != 0.0f)
            {
                ActualUpdate();
            }
        }
    }


    void LateUpdate()
    {
        --s_update_count;
    }


    static void ActualUpdate()
    {
        if (s_instances.Count == 0) { return; }
        if (s_instances[0].m_update_mode == MPUpdateMode.Immediate)
        {
            ImmediateUpdate();
        }
        else if (s_instances[0].m_update_mode == MPUpdateMode.Deferred)
        {
            DeferredUpdate();
        }
    }

    static void ImmediateUpdate()
    {
        foreach (MPWorld w in s_instances)
        {
            w.UpdateKernelParams();
        }
        UpdateMPObjects();
        foreach (MPWorld w in s_instances)
        {
            s_current = w;
            MPAPI.mpUpdate(w.GetContext(), Time.deltaTime);
            MPAPI.mpCallHandlers(w.GetContext());
            MPAPI.mpClearCollidersAndForces(w.GetContext());
        }
    }

    static void DeferredUpdate()
    {
        foreach (MPWorld w in s_instances)
        {
            MPAPI.mpEndUpdate(w.GetContext());
        }
        foreach (MPWorld w in s_instances)
        {
            s_current = w;
            MPAPI.mpCallHandlers(w.GetContext());
            MPAPI.mpClearCollidersAndForces(w.GetContext());
            w.UpdateKernelParams();
        }
        UpdateMPObjects();
        foreach (MPWorld w in s_instances)
        {
            MPAPI.mpBeginUpdate(w.GetContext(), Time.deltaTime);
        }
    }


    void OnDrawGizmos()
    {
        Gizmos.color = Color.yellow;
        Gizmos.DrawWireCube(transform.position, transform.localScale*2.0f);
    }


    void UpdateKernelParams()
    {
        MPKernelParams p = MPAPI.mpGetKernelParams(GetContext());
        p.world_center = transform.position;
        p.world_size = transform.localScale;
        p.world_div_x = m_world_div_x;
        p.world_div_y = m_world_div_y;
        p.world_div_z = m_world_div_z;
        p.solver_type = (int)m_solver;
        p.enable_interaction = m_enable_interaction ? 1 : 0;
        p.enable_colliders = m_enable_colliders ? 1 : 0;
        p.enable_forces = m_enable_orces ? 1 : 0;
        p.lifetime = m_particle_lifetime;
        p.timestep = Time.deltaTime * m_timescale;
        p.decelerate = m_deceleration;
        p.advection = m_advection;
        p.pressure_stiffness = m_pressure_stiffness;
        p.wall_stiffness = m_wall_stiffness;
        p.scaler = m_coord_scale;
        p.particle_size = m_particle_size;
        p.max_particles = m_max_particle_num;
        MPAPI.mpSetKernelParams(GetContext(), ref p);
    }

    static void UpdateMPObjects()
    {
        MPCollider.MPUpdateAll();
        MPForce.MPUpdateAll();
        MPEmitter.MPUpdateAll();
    }
}
