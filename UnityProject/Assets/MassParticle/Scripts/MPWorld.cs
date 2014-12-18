using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public unsafe class MPWorld : MonoBehaviour
{
    public static List<MPWorld> s_instances = new List<MPWorld>();
    static int s_update_count = 0;

    void EachWorld(Action<MPWorld> f)
    {
        s_instances.ForEach(f);
    }


    public delegate void ParticleProcessor(MPWorld world, int numParticles, MPParticle* particles);
    public delegate void GatheredHitProcessor(MPWorld world, int numColliders, MPHitData* hits);

    public MPUpdateMode updateMode = MPUpdateMode.Immediate;
    public bool enableInteractions = true;
    public bool enableColliders = true;
    public bool enableForces = true;
    public MPSolverType solverType = MPSolverType.Impulse;
    public float force = 1.0f;
    public float particleLifeTime;
    public float timeScale = 0.6f;
    public float deceleration;
    public float pressureStiffness;
    public float wallStiffness;
    public Vector3 coordScale;
    public int divX = 256;
    public int divY = 1;
    public int divZ = 256;
    public float particleSize = 0.08f;
    public int maxParticleNum = 65536;
    public int particleNum = 0;

    public ParticleProcessor particleProcessor;
    public GatheredHitProcessor gatheredHitProcessor;
    IntPtr context;

    public const int cubeBatchSize = 2700;
    public const int pointBatchSize = 65000;
    public const int dataTextureWidth = 3072;
    public const int dataTextureHeight = 256;

    public IntPtr GetContext() { return context; }


    public int UpdateDataTexture(RenderTexture rt)
    {
        particleNum = MPAPI.mpUpdateDataTexture(context, rt.GetNativeTexturePtr());
        return particleNum;
    }


    MPWorld()
    {
#if UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
        MPAPI.mphInitialize();
#endif
        particleProcessor = DefaultParticleProcessor;
        gatheredHitProcessor = DefaultGatheredHitProcessor;
    }

    void Awake()
    {
        s_instances.Add(this);
        context = MPAPI.mpCreateContext();
    }

    void OnDestroy()
    {
        MPAPI.mpDestroyContext(context);
        s_instances.Remove(this);
    }

    void Update()
    {
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
        if (s_instances[0].updateMode == MPUpdateMode.Immediate)
        {
            ImmediateUpdate();
        }
        else if (s_instances[0].updateMode == MPUpdateMode.Deferred)
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
            MPAPI.mpUpdate(w.GetContext(), Time.deltaTime);
            w.ExecuteProcessors();
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
            w.ExecuteProcessors();

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
        MPKernelParams p = MPAPI.mpGetKernelParams(context);
        p.WorldCenter = transform.position;
        p.WorldSize = transform.localScale;
        p.WorldDiv_x = divX;
        p.WorldDiv_y = divY;
        p.WorldDiv_z = divZ;
        p.enableInteractions = enableInteractions ? 1 : 0;
        p.enableColliders = enableColliders ? 1 : 0;
        p.enableForces = enableForces ? 1 : 0;
        p.SolverType = (int)solverType;
        p.LifeTime = particleLifeTime;
        p.Timestep = Time.deltaTime * timeScale;
        p.Decelerate = deceleration;
        p.PressureStiffness = pressureStiffness;
        p.WallStiffness = wallStiffness;
        p.Scaler = coordScale;
        p.ParticleSize = particleSize;
        p.MaxParticles = maxParticleNum;
        MPAPI.mpSetKernelParams(context, ref p);
    }

    static void UpdateMPObjects()
    {
        MPCollider.MPUpdateAll();
        MPForce.MPUpdateAll();
        MPEmitter.MPUpdateAll();
    }


    void ExecuteProcessors()
    {
        if (particleProcessor != null)
        {
            particleProcessor(this, MPAPI.mpGetNumParticles(context), MPAPI.mpGetParticles(context));
        }
        if (gatheredHitProcessor != null)
        {
            gatheredHitProcessor(this, MPAPI.mpGetNumHitData(context), MPAPI.mpGetHitData(context));
        }
    }


    public static unsafe void DefaultParticleProcessor(MPWorld world, int numParticles, MPParticle* particles)
    {
        for (int i = 0; i < numParticles; ++i)
        {
            if (particles[i].hit != -1 && particles[i].hit != particles[i].hit_prev)
            {
                MPCollider col = MPCollider.GetHitOwner(particles[i].hit);
                if (col && col.particleHitHandler!=null)
                {
                    col.particleHitHandler(world, col, ref particles[i]);
                }
            }
        }
    }

    public static unsafe void DefaultGatheredHitProcessor(MPWorld world, int numColliders, MPHitData* hits)
    {
        for (int i = 0; i < numColliders; ++i)
        {
            if (hits[i].num_hits>0)
            {
                MPCollider col = MPCollider.GetHitOwner(i);
                if (col && col.gatheredHitHandler != null)
                {
                    col.gatheredHitHandler(world, col, ref hits[i]);
                }
            }
        }
    }
}
