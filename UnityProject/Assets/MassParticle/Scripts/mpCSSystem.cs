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

	unsafe public class mpSystem
	{
		public const int DEFAULT_MAX_PARTICLE = 65536;

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

		public mp.mpParticle[]		particles;
		public mpIMData[]			imdata;
		public int					numActiveParticles;
		public mp.mpKernelParams	kparams;
		public List<mpForce>		forces;
		public List<mpSphereCollider>	col_spheres;
		public List<mpBoxCollider>	col_boxes;
		public Mesh mesh;

		mpSystem()
		{
			kparams.MaxParticles = DEFAULT_MAX_PARTICLE;
			kparams.ParticleSize = 0.08f;

			particles = new mp.mpParticle[kparams.MaxParticles];
			imdata = new mpIMData[kparams.MaxParticles];
			forces = new List<mpForce>();
			col_spheres = new List<mpSphereCollider>();
			col_boxes = new List<mpBoxCollider>();
		}

		void BuildMesh()
		{
			float s = 0.08f;
			mesh = new Mesh();
			Vector3 p0 = new Vector3(0, 0, 0);
			Vector3 p1 = new Vector3(s, 0, 0);
			Vector3 p2 = new Vector3(s, s, 0);
			Vector3 p3 = new Vector3(0, s, 0);
			Vector3 p4 = new Vector3(s, 0, 0);
			Vector3 p5 = new Vector3(s, 0, s);
			Vector3 p6 = new Vector3(s, s, s);
			Vector3 p7 = new Vector3(s, s, 0);
			mesh.vertices = new Vector3[]{
					p0,p1,p2,
					p0,p2,p3,
					p2,p1,p3,
					p0,p3,p1,
					p4,p5,p6,
					p4,p6,p5,
					p4,p6,p7,
					p4,p7,p6
				};
			mesh.triangles = new int[]{
					0,1,2,
					3,4,5,
					6,7,8,
					9,10,11,
					12,13,14,
					15,16,17,
					18,19,20,
					21,22,23
				};
			mesh.RecalculateNormals();
			mesh.RecalculateBounds();
			mesh.Optimize();
		}

		public void Start()
		{
		}

		public void Update(float dt)
		{
			if (particles.Length != kparams.MaxParticles)
			{
				particles = new mp.mpParticle[kparams.MaxParticles];
				imdata = new mpIMData[kparams.MaxParticles];
				numActiveParticles = Math.Min(numActiveParticles, kparams.MaxParticles);
			}
			fixed (mp.mpParticle* ps = particles)
			{
				fixed (mpIMData* imd = imdata)
				{
					GenHash(numActiveParticles, ps, imd);
					MakehashGrid(numActiveParticles, ps, imd);
					SortParticles(numActiveParticles, ps, imd);
					ProcessParticleCollision(numActiveParticles, ps, imd);
					ProcessExternalForces(numActiveParticles, ps, imd);
					ProcessColliders(numActiveParticles, ps, imd);
					Integrate(numActiveParticles, ps, imd);
				}
			}
			ClearCollisionsAndForces();
		}

		public void Draw()
		{
			if (!mesh) { return; }

			Matrix4x4 mat = Matrix4x4.identity;
			mat.m00 = kparams.ParticleSize;
			mat.m11 = kparams.ParticleSize;
			mat.m22 = kparams.ParticleSize;
			for (int i = 0; i < numActiveParticles; ++i)
			{
				mat.m03 = particles[i].position3.x;
				mat.m13 = particles[i].position3.y;
				mat.m23 = particles[i].position3.z;
				//Graphics.DrawMeshNow(mesh, particles[i].position3, Quaternion.identity);
				Graphics.DrawMeshNow(mesh, mat);
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

		public int AddSphereCollider(int owner, Vector3 center, float radius)
		{
			mpSphereCollider col = new mpSphereCollider();
			col.Build(owner, center, radius);
			col_spheres.Add(col);
			return 0;
		}

		public int AddBoxCollider(int owner, Matrix4x4 trans, Vector3 size)
		{
			mpBoxCollider col = new mpBoxCollider();
			col.Build(owner, trans, size);
			col_boxes.Add(col);
			return 0;
		}

		public int AddForce(mp.mpForceShape regionType, Matrix4x4 trans, mp.mpForceDirection dirType, mp.mpForceParams p)
		{
			mpForce force = new mpForce();
			force.shape_type = regionType;
			force.dir_type = dirType;
			force.fparams = p;

			switch (force.shape_type) {
			case mp.mpForceShape.Box:
			{
				mpBoxCollider col = new mpBoxCollider();
				col.Build(0, trans, Vector3.one);
				force.bb = col.bb;
				force.shape_box.center = col.center;

				force.shape_box.plane0 = col.plane0;
				force.shape_box.plane1 = col.plane1;
				force.shape_box.plane2 = col.plane2;
				force.shape_box.plane3 = col.plane3;
				force.shape_box.plane4 = col.plane4;
				force.shape_box.plane5 = col.plane5;
			}
				break;

			case mp.mpForceShape.Sphere:
			{
				mpSphereCollider col = new mpSphereCollider();
				col.Build(0, new Vector3(trans.m30, trans.m31, trans.m32), (trans.m00 + trans.m11 + trans.m22)*0.33333333f*0.5f);
				force.bb = col.bb;
				force.shape_sphere.center = col.center;
				force.shape_sphere.radius = col.radius;
			}
				break;
			}
			mpCS.mpSystem.instance.forces.Add(force);
			return mpCS.mpSystem.instance.forces.Count - 1;
		}


		float RandHalf()
		{
			return UnityEngine.Random.Range(-0.5f, 0.5f);
		}
		float RandPositive()
		{
			return UnityEngine.Random.Range(0.0f, 1.0f);
		}


		public int ScatterParticlesSphereTransform(Matrix4x4 trans, int num, Vector3 velBase, float velDiffuse)
		{
			if (num <= 0) { return 0; }

			mp.mpParticle tmp = new mp.mpParticle();
			tmp.lifetime = kparams.LifeTime;
			tmp.hit = tmp.hit_prev = -1;
			Vector3 dir = Vector3.zero;
			for (int i = 0; i < num; ++i)
			{
				if (numActiveParticles < particles.Length)
				{
					dir.Set(RandHalf(), RandHalf(), RandHalf());
					dir.Normalize();
					float l = RandPositive();
					tmp.position3 = dir * l;
					tmp.position.w = 1.0f;
					tmp.position = trans * tmp.position;
					tmp.velocity3 = Vector3.zero;
					particles[numActiveParticles++] = tmp;
				}
				else
				{
					break;
				}
			}
			return 0;
		}

		public int ScatterParticlesBoxTransform(Matrix4x4 trans, int num, Vector3 velBase, float velDiffuse)
		{
			if (num <= 0) { return 0; }

			mp.mpParticle tmp = new mp.mpParticle();
			tmp.lifetime = kparams.LifeTime;
			tmp.hit = tmp.hit_prev = -1;
			for (int i = 0; i < num; ++i)
			{
				if (numActiveParticles < particles.Length)
				{
					tmp.position.Set(RandHalf(), RandHalf(), RandHalf(), 1.0f);
					tmp.position = trans * tmp.position;
					tmp.velocity3 = Vector3.zero;
					particles[numActiveParticles++] = tmp;
				}
				else
				{
					break;
				}
			}
			return 0;
		}



		unsafe static void UnsafeQuickSortRecursive(mp.mpParticle* data, int left, int right)
		{
			int i = left - 1;
			int j = right;

			while (true)
			{
				float d = data[left].lifetime;
				do i++; while (data[i].lifetime > d);
				do j--; while (data[j].lifetime < d);

				if (i < j)
				{
					mp.mpParticle tmp = data[i];
					data[i] = data[j];
					data[j] = tmp;
				}
				else
				{
					if (left < j) UnsafeQuickSortRecursive(data, left, j);
					if (++j < right) UnsafeQuickSortRecursive(data, j, right);
					return;
				}
			}
		}

		void GenHash(int n, mp.mpParticle* ps, mpIMData* imd)
		{
			UnsafeQuickSortRecursive(ps, 0, n - 1);
			int i = 0;
			for (; i < numActiveParticles; ++i )
			{
				ps[i].lifetime = Mathf.Max(0.0f, ps[i].lifetime - 1.0f);
				imd[i].accel = Vector3.zero;
				if (ps[i].lifetime == 0.0f) { break; }
			}
			numActiveParticles = i;

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

				if(force.shape_type==mp.mpForceShape.All) {
					for(int i=0; i<n; ++i) {
						imd[i].affection = 1.0f;
					}
				}
				else if(force.shape_type==mp.mpForceShape.Sphere) {
					//if(!IsGridOverrapedAABB(xi,0,zi, force.bb)) { continue; }

					mpSphere sphere = force.shape_sphere;
					float radius_sq = sphere.radius * sphere.radius;
					Vector3 force_center = sphere.center;
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
						Vector3 dir = force.fparams.dir;
						Vector3 accel = imd[i].accel;
						accel += dir * imd[i].affection * force.fparams.strength;
						imd[i].accel = accel;
					}
				}
				else if(force.dir_type==mp.mpForceDirection.Radial) {
					Vector3 force_center = force.fparams.pos;
					for(int i=0; i<n; ++i) {
						Vector3 particles_pos = ps[i].position3;
						Vector3 diff = particles_pos - force_center;
						Vector3 dir = diff.normalized;
						Vector3 accel = imd[i].accel;
						accel += dir * imd[i].affection * force.fparams.strength;
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
					Vector3 diff = particles_pos - sphere_pos;
					float len = Vector3.Dot(diff, diff);
					float distance = len - sphere_radius;
					if(distance < 0.0f) {
						Vector3 dir = diff / len;
						particles[i].hit = shape.objid;
						imd[i].accel += dir * (distance * kparams.WallStiffness);
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
					Vector3 closest_normal = Vector3.zero;
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
						particles[i].hit = shape.objid;
						imd[i].accel += closest_normal * (-closest_distance * kparams.WallStiffness);
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
				pos = Vector3.Scale(pos, kparams.Scaler);

				ps[i].position3 = pos;
				ps[i].velocity3 = vel;
				ps[i].speed = Vector3.Magnitude(vel);
			}
		}
	};
}
#endif // MP_PURE_CSHARP
