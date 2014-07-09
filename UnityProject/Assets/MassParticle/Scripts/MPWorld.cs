using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;


public unsafe class MPWorld : MonoBehaviour {

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
	public bool include3DColliders = true;
	public bool include2DColliders = true;
	public int divX = 256;
	public int divY = 1;
	public int divZ = 256;
	public float particleSize = 0.08f;
	public int maxParticleNum = 200000;

	public bool enableParticleHitHandler = false;
	public bool enableGatheredHitHandler = true;

	public ParticleProcessor particleProcessor;
	public GatheredHitProcessor gatheredHitProcessor;
	public List<GameObject> colliders;
	MPRenderer mprenderer;

	MPWorld()
	{
#if UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
		MPAPI.mphInitialize();
#endif
		particleProcessor = DefaultParticleProcessor;
		gatheredHitProcessor = DefaultGatheredHitProcessor;
	}

	void Reset()
	{
		MPKernelParams p = MPAPI.mpGetKernelParams();
		transform.position = p.WorldCenter;
		transform.localScale = p.WorldSize;
		solverType 			= (MPSolverType)p.SolverType;
		particleLifeTime 	= p.LifeTime;
		deceleration 		= p.Decelerate;
		pressureStiffness 	= p.PressureStiffness;
		wallStiffness 		= p.WallStiffness;
		coordScale 			= p.Scaler;
		particleSize		= p.ParticleSize;
		maxParticleNum		= p.MaxParticles;
	}

	void OnEnable()
	{
		MPAPI.mpOnEnable();
	}

	void OnDisable()
	{
		MPAPI.mpOnDisable();
		MPAPI.mpClearCollidersAndForces();
		MPAPI.mpClearParticles();
		colliders.Clear();
	}

	void Start()
	{
		mprenderer = GetComponent<MPRenderer>();
	}

	void Update()
	{
		if (Time.deltaTime != 0.0f)
		{
			switch (updateMode)
			{
				case MPUpdateMode.Immediate: ImmediateUpdate(); break;
				case MPUpdateMode.Deferred: DeferredUpdate(); break;
			}
		}
	}


	void ImmediateUpdate()
	{
		UpdateKernelParams();
		UpdateMPObjects();
		MPAPI.mpUpdate(Time.deltaTime);
		ExecuteProcessors();
		MPAPI.mpClearCollidersAndForces();
	}

	void DeferredUpdate()
	{
		MPAPI.mpEndUpdate();
		ExecuteProcessors();

		MPAPI.mpClearCollidersAndForces();
		UpdateKernelParams();
		UpdateMPObjects();
		MPAPI.mpBeginUpdate(Time.deltaTime);
	}


	void OnDrawGizmos()
	{
		Gizmos.color = Color.yellow;
		Gizmos.DrawWireCube(transform.position, transform.localScale*2.0f);
	}


	void UpdateKernelParams()
	{
		MPKernelParams p = MPAPI.mpGetKernelParams();
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
		MPAPI.mpSetKernelParams(ref p);
	}

	void UpdateMPObjects()
	{
		colliders.Clear();
		if (include3DColliders)
		{
			Collider[] colliders3d = Physics.OverlapSphere(transform.position, transform.localScale.magnitude);
			for (int i = 0; i < colliders3d.Length; ++i)
			{
				Collider col = colliders3d[i];
				if (col.isTrigger) { continue; }

				MPColliderProperties cprops;
				bool recv = false;
				var attr = col.gameObject.GetComponent<MPColliderAttribute>();
				if (attr)
				{
					if (!attr.sendCollision) { continue; }
					attr.UpdateColliderProperties();
					recv = attr.receiveCollision;
					cprops = attr.cprops;
				}
				else
				{
					cprops = new MPColliderProperties();
					cprops.SetDefaultValues();
				}
				int id = colliders.Count;
				cprops.owner_id = recv ? id : -1;
				colliders.Add(col.gameObject);

				SphereCollider sphere = col as SphereCollider;
				CapsuleCollider capsule = col as CapsuleCollider;
				BoxCollider box = col as BoxCollider;
				if (sphere)
				{
					Vector3 pos = sphere.transform.position;
					MPAPI.mpAddSphereCollider(ref cprops, ref pos, sphere.radius * col.gameObject.transform.localScale.magnitude * 0.5f);
				}
				else if (capsule)
				{
					Vector3 e = Vector3.zero;
					float h = Mathf.Max(0.0f, capsule.height - capsule.radius * 2.0f);
					float r = capsule.radius * capsule.transform.localScale.x;
					switch (capsule.direction)
					{
						case 0: e.Set(h * 0.5f, 0.0f, 0.0f); break;
						case 1: e.Set(0.0f, h * 0.5f, 0.0f); break;
						case 2: e.Set(0.0f, 0.0f, h * 0.5f); break;
					}
					Vector4 pos1 = new Vector4(e.x, e.y, e.z, 1.0f);
					Vector4 pos2 = new Vector4(-e.x, -e.y, -e.z, 1.0f);
					pos1 = capsule.transform.localToWorldMatrix * pos1;
					pos2 = capsule.transform.localToWorldMatrix * pos2;
					Vector3 pos13 = pos1;
					Vector3 pos23 = pos2;
					MPAPI.mpAddCapsuleCollider(ref cprops, ref pos13, ref pos23, r);
				}
				else if (box)
				{
					Matrix4x4 mat = box.transform.localToWorldMatrix;
					Vector3 size = box.size;
					MPAPI.mpAddBoxCollider(ref cprops, ref mat, ref size);
				}
			}
		}

		if (include2DColliders)
		{
			Vector2 xy = new Vector2(transform.position.x, transform.position.y);
			Collider2D[] colliders2d = Physics2D.OverlapCircleAll(xy, transform.localScale.magnitude);
			for (int i = 0; i < colliders2d.Length; ++i)
			{
				Collider2D col = colliders2d[i];
				if (col.isTrigger) { continue; }

				MPColliderProperties cprops;
				bool recv = false;
				var attr = col.gameObject.GetComponent<MPColliderAttribute>();
				if (attr)
				{
					if (!attr.sendCollision) { continue; }
					attr.UpdateColliderProperties();
					recv = attr.receiveCollision;
					cprops = attr.cprops;
				}
				else
				{
					cprops = new MPColliderProperties();
					cprops.SetDefaultValues();
				}
				int id = colliders.Count;
				cprops.owner_id = recv ? id : -1;
				colliders.Add(col.gameObject);

				CircleCollider2D sphere = col as CircleCollider2D;
				BoxCollider2D box = col as BoxCollider2D;
				if (sphere)
				{
					Vector3 pos = sphere.transform.position;
					MPAPI.mpAddSphereCollider(ref cprops, ref pos, sphere.radius * col.gameObject.transform.localScale.x);
				}
				else if (box)
				{
					Matrix4x4 mat = box.transform.localToWorldMatrix;
					Vector3 size = new Vector3(box.size.x, box.size.y, box.size.x);
					MPAPI.mpAddBoxCollider(ref cprops, ref mat, ref size);
				}
			}
		}

		foreach (MPCollider col in MPCollider.instances)
		{
			if (!col.sendCollision) { continue; }
			col.MPUpdate();
			col.cprops.owner_id = colliders.Count;
			colliders.Add(col.gameObject);
		}
		foreach (MPForce force in MPForce.instances)
		{
			force.MPUpdate();
		}
		foreach (MPEmitter emitter in MPEmitter.instances)
		{
			emitter.MPUpdate();
		}

		mprenderer.MPUpdate();
	}

	void ExecuteProcessors()
	{
		if (enableParticleHitHandler && particleProcessor != null)
		{
			particleProcessor(this, MPAPI.mpGetNumParticles(), MPAPI.mpGetParticles());
		}
		if (enableGatheredHitHandler && gatheredHitProcessor != null)
		{
			gatheredHitProcessor(this, MPAPI.mpGetNumHitData(), MPAPI.mpGetHitData());
		}
	}


	public static unsafe void DefaultParticleProcessor(MPWorld world, int numParticles, MPParticle* particles)
	{
		for (int i = 0; i < numParticles; ++i)
		{
			if (particles[i].hit != -1 && particles[i].hit != particles[i].hit_prev)
			{
				GameObject col = world.colliders[particles[i].hit];
				MPColliderAttribute cattr = col.GetComponent<MPColliderAttribute>();
				if (cattr)
				{
					cattr.particleHitHandler(world, col, ref particles[i]);
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
				GameObject col = world.colliders[i];
				MPColliderAttribute cattr = col.GetComponent<MPColliderAttribute>();
				if (cattr)
				{
					cattr.gatheredHitHandler(world, col, ref hits[i]);
				}
			}
		}
	}
}
