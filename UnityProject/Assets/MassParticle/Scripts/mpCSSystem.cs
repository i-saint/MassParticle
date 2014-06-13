#if MP_PURE_CSHARP
using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;
//using System.Collections.Concurrent;

namespace mpCS
{

	//public class mpTaskScheduler
	//{
	//	static mpTaskScheduler _instance;
	//	public static mpTaskScheduler instance
	//	{
	//		get
	//		{
	//			if (_instance == null)
	//			{
	//				_instance = new mpTaskScheduler();
	//			}
	//			return _instance;
	//		}
	//	}

	//	delegate void Task();
	//	List<Thread>	threads;
	//	List<Task>		tasks;
	//	Monitor			mutex_tasks;
	//	Semaphore		sema;
	//	bool			endFlag;

	//	mpTaskScheduler()
	//	{
	//		threads = new List<Thread>();
	//		tasks = new List<Task>();
	//		mutex_tasks = new Monitor();
	//		sema = new Semaphore(0, 128);
	//		endFlag = false;

	//		for (int i = 0; i < Environment.ProcessorCount; ++i )
	//		{
	//			Thread t = new Thread(() => WorkerThreadMain());
	//			threads.Add(t);
	//		}
	//	}

	//	void Execute(List<Task> t)
	//	{
	//		tasks = t;
	//		mutex_tasks.ReleaseMutex();
	//	}

	//	void WorkerThreadMain()
	//	{
	//		while (!endFlag)
	//		{
	//			mutex_tasks.Lo
	//		}
	//	}
	//}

	unsafe public class mpSystem
	{
		public const int MAX_PARTICLE = 65536;

		static mpSystem _instance;
		public static mpSystem instance
		{
			get
			{
				if (_instance == null)
				{
					_instance = new mpSystem();
				}
				return _instance;
			}
		}

		public mp.mpParticle[]			particles;
		public mpIMData[]			imdata;
		public int					numActiveParticles;
		public mp.mpKernelParams	kparams;
		public List<mpForce>		forces;
		public List<mpSphereCollider>	col_spheres;
		public List<mpBoxCollider>	col_boxes;
		public Mesh mesh;

		mpSystem()
		{
			particles = new mp.mpParticle[MAX_PARTICLE];
			imdata = new mpIMData[MAX_PARTICLE];
			kparams.ParticleSize = 0.08f;
			forces = new List<mpForce>();
			col_spheres = new List<mpSphereCollider>();
			col_boxes = new List<mpBoxCollider>();
		}

		public void Update(float dt)
		{
			fixed (mp.mpParticle* ps = particles)
			{
				fixed (mpIMData* imd = imdata)
				{
					//GenHash(numActiveParticles, ps, imd);
					//MakehashGrid(numActiveParticles, ps, imd);
					//SortParticles(numActiveParticles, ps, imd);
					ProcessParticleCollision(numActiveParticles, ps, imd);
					ProcessExternalForces(numActiveParticles, ps, imd);
					ProcessColliders(numActiveParticles, ps, imd);
					Integrate(numActiveParticles, ps, imd);
				}
			}
			ClearCollisionsAndForces();
		}

		public void Render()
		{
			fixed (mp.mpParticle* ps = particles)
			{
				for (int i = 0; i < numActiveParticles; ++i)
				{
					Graphics.DrawMesh(mesh, ps[i].position3, Quaternion.identity);
				}
			}
		}

		public void ClearParticles()
		{
			fixed (mp.mpParticle* ps = particles)
			{
				for (int i = 0; i < numActiveParticles; ++i)
				{
					ps[i].lifetime = 0.0f;
				}
			}
		}

		public void ClearCollisionsAndForces()
		{
			forces.Clear();
			col_spheres.Clear();
			col_boxes.Clear();
		}

		public uint AddSphereCollider(int owner, Vector3 center, float radius)
		{
			mpSphereCollider col = new mpSphereCollider();
			col.id = owner;
			col.center = center;
			col.radius = radius;
			col.bb = new Bounds(col.center, new Vector3(col.radius, col.radius, col.radius));
			col_spheres.Add(col);
			return 0;
		}

		public uint AddBoxCollider(int owner, Matrix4x4 transform, Vector3 size)
		{
			mpBoxCollider col = new mpBoxCollider();
			col.id = owner;
			col.center = new Vector3(transform.m30, transform.m31, transform.m32);
			col.size = size;
			col.bb = new Bounds(col.center, Vector3.zero);
			col_spheres.Add(col);
			return 0;
		}

		public uint AddForce(mp.mpForceShape regionType, Matrix4x4 regionMat, mp.mpForceDirection dirType, mp.mpForceParams p)
		{
			return 0;
		}

		public uint ScatterParticlesSphereTransform(Matrix4x4 trans, int num, Vector3 velBase, float velDiffuse)
		{
			return 0;
		}

		public uint ScatterParticlesBoxTransform(Matrix4x4 trans, int num, Vector3 velBase, float velDiffuse)
		{
			return 0;
		}


		void GenHash(int n, mp.mpParticle* ps, mpIMData* imd)
		{
			// todo
		}

		void MakehashGrid(int n, mp.mpParticle* ps, mpIMData* imd)
		{
			// todo
		}

		void SortParticles(int n, mp.mpParticle* ps, mpIMData* imd)
		{
			// todo
		}

		void ProcessParticleCollision(int n, mp.mpParticle* ps, mpIMData* imd)
		{
			for (int i = 0; i < n; ++i)
			{
				Vector3 pos1 = ps[i].position3;
				for (int j = 0; j < n; ++j)
				{
					if (i == j) { continue; }
					Vector3 pos2 = ps[j].position3;

					float mpRcpParticleSize2 = 1.0f / (kparams.ParticleSize * 2.0f);
					Vector3 diff = pos2 - pos1;    
					float d = Vector3.Magnitude(diff);
					Vector3 dir = diff * mpRcpParticleSize2; // Vector3 dir = diff / d;
					imd[i].accel += dir * (Mathf.Min(0.0f, d - (kparams.ParticleSize * 2.0f)) * kparams.PressureStiffness);
				}
			}
		}

		void ProcessExternalForces(int n, mp.mpParticle* ps, mpIMData* imd)
		{
			int num_forces = forces.Count;
			for(int fi=0; fi<num_forces; ++fi) {
				mpForce force = forces[fi];

				// todo: ‹…ó radial force ‚Í‚æ‚­Žg‚í‚ê‚é‚Æ—\‘z‚³‚ê‚é‚Ì‚Å“ÁŽê‰»‚É‚æ‚éÅ“K‰»
				if(force.shape_type==mp.mpForceShape.All) {
					for(int i=0; i<n; ++i) {
						imd[i].affection = 1.0f;
					}
				}
				else if(force.shape_type==mp.mpForceShape.Sphere) {
					//if(!IsGridOverrapedAABB(xi,0,zi, force.bb)) { continue; }

					mpSphere sphere = force.shape_sphere;
					float radius_sq = sphere.radius * sphere.radius;
					Vector3 force_center = sphere.pos;
					for(int i=0; i<n; ++i) {
						Vector3 particles_pos = ps[i].position3;
						Vector3 diff = particles_pos - force_center;
						float dist_sq = Vector3.Dot(diff, diff);
						imd[i].affection = dist_sq<=radius_sq ? 1.0f : 0.0f;
					}
				}
				else if(force.shape_type==mp.mpForceShape.Box) {
					//if(!IsGridOverrapedAABB(xi,0,zi, force.bb)) { continue; }

					mpBox box = force.shape_box;
					Vector3 box_pos = box.center;
					for(int i=0; i<n; ++i) {
						Vector3 particles_pos = ps[i].position3;
						int inside = 0;
						Vector3 ppos = particles_pos - box_pos;
						mpPlane* planes = &box.plane0;
						for(int p=0; p<6; ++p) {
							Vector3 plane_normal = planes[p].normal;
							float plane_distance = planes[p].distance;
							float distance = Vector3.Dot(ppos, plane_normal) + plane_distance;
							if(distance < 0.0f) {
								inside++;
							}
						}
						imd[i].affection = inside==6 ? 1.0f : 0.0f;
					}
				}

				if(force.dir_type==mp.mpForceDirection.Directional) {
					for(int i=0; i<n; ++i) {
						Vector3 dir = force.pos;
						Vector3 accel = imd[i].accel;
						accel += dir * imd[i].affection * force.strength;
						imd[i].accel = accel;
					}
				}
				else if(force.dir_type==mp.mpForceDirection.Radial) {
					Vector3 force_center = force.pos;
					for(int i=0; i<n; ++i) {
						Vector3 particles_pos = ps[i].position3;
						Vector3 diff = particles_pos - force_center;
						Vector3 dir = diff.normalized;
						Vector3 accel = imd[i].accel;
						accel += dir * imd[i].affection * force.strength;
						imd[i].accel = accel;
					}
				}
			}
		}

		void ProcessColliders(int n, mp.mpParticle* ps, mpIMData* imd)
		{
			// Sphere
			for(int s=0; s<col_spheres.Count; ++s) {
				mpSphereCollider shape = col_spheres[s];
				//if(!IsGridOverrapedAABB(xi,0,zi, shape.bb)) { continue; }

				Vector3 sphere_pos = shape.center;
				float sphere_radius = shape.radius;
				for(int i=0; i<n; ++i) {
					Vector3 particles_pos = ps[i].position3;
					Vector3 diff = sphere_pos - particles_pos;
					float distance = Vector3.Dot(diff, diff) - sphere_radius;
					if(distance < 0.0f) {
						Vector3 dir = diff / distance;
						//repulse(particles[i], imd[i], dir, distance, shape);
					}
				}
			}

			// Box
			for(int s=0; s<col_boxes.Count; ++s) {
				mpBoxCollider shape = col_boxes[s];
				//if(!IsGridOverrapedAABB(xi,0,zi, shape.bb)) { continue; }

				Vector3 box_pos = shape.center;
				for(int i=0; i<n; ++i) {
					Vector3 particles_pos = ps[i].position3;

					int inside = 0;
					float closest_distance = -9999.0f;
					Vector3 closest_normal;
					Vector3 ppos = particles_pos - box_pos;
					mpPlane* planes = &shape.plane0;
					for(int p=0; p<6; ++p) {
						Vector3 plane_normal = planes[p].normal;
						float plane_distance = planes[p].distance;
						float distance = Vector3.Dot(ppos, plane_normal) + plane_distance;
						if(distance < 0.0f) {
							inside++;
							if(distance > closest_distance) {
								closest_distance = distance;
								closest_normal = plane_normal;
							}
						}
					}
					if(inside==6) {
						repulse(particles[i], imd[i], closest_normal, closest_distance, shape);
					}
				}
			}
		}

		void Integrate(int n, mp.mpParticle* ps, mpIMData* imd)
		{
			for (int i = 0; i < n; ++i)
			{
				Vector3 pos = ps[i].position3;
				Vector3 vel = ps[i].velocity3;
				Vector3 accel = imd[i].accel;

				vel += accel * kparams.Timestep;
				vel *= kparams.Decelerate;
				vel = Vector3.Scale(vel, kparams.Scaler);

				pos += vel * kparams.Timestep;
				pos = Vector3.Scale(vel, kparams.Scaler);

				ps[i].position3 = pos;
				ps[i].velocity3 = vel;
				ps[i].speed = Vector3.Magnitude(vel);
			}
		}
	};
}
#endif // MP_PURE_CSHARP
