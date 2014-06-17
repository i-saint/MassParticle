#if MP_PURE_CSHARP
using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace mpCS
{
	public struct mpIMData
	{
		public Vector3 accel;
		public float affection;
	}


	public struct mpSphere
	{
		public Vector3 center;
		public float radius;
	}

	public struct mpPlane
	{
		public Vector3 normal;
		public float distance;
	}

	public struct mpBox
	{
		public Vector3 center;
		public mpPlane plane0;
		public mpPlane plane1;
		public mpPlane plane2;
		public mpPlane plane3;
		public mpPlane plane4;
		public mpPlane plane5;
	}

	public struct mpBoxCollider
	{
		public int objid;
		public Bounds bb;
		public Vector3 center;
		public mpPlane plane0;
		public mpPlane plane1;
		public mpPlane plane2;
		public mpPlane plane3;
		public mpPlane plane4;
		public mpPlane plane5;

		public unsafe void Build(int ownerid, Matrix4x4 trans, Vector3 size)
		{
			float psize = mpSystem.instance.kparams.ParticleSize;
			Vector3 psize3 = Vector3.one * psize;
			size.x = size.x * 0.5f;
			size.y = size.y * 0.5f;
			size.z = size.z * 0.5f;

			Matrix4x4 st = trans;
			Vector3[] vertices = new Vector3[8];
			vertices[0].Set(size.x, size.y, size.z);
			vertices[1].Set(-size.x, size.y, size.z);
			vertices[2].Set(-size.x, -size.y, size.z);
			vertices[3].Set(size.x, -size.y, size.z);
			vertices[4].Set(size.x, size.y, -size.z);
			vertices[5].Set(-size.x, size.y, -size.z);
			vertices[6].Set(-size.x, -size.y, -size.z);
			vertices[7].Set(size.x, -size.y, -size.z);
			for (int i = 0; i < vertices.Length; ++i) {
				vertices[i] = st * vertices[i];
			}

			Vector3[] normals = new Vector3[6];
			normals[0] = Vector3.Cross(vertices[3]-vertices[0], vertices[4]-vertices[0]).normalized;
			normals[1] = Vector3.Cross(vertices[5]-vertices[1], vertices[2]-vertices[1]).normalized;
			normals[2] = Vector3.Cross(vertices[7]-vertices[3], vertices[2]-vertices[3]).normalized;
			normals[3] = Vector3.Cross(vertices[1]-vertices[0], vertices[4]-vertices[0]).normalized;
			normals[4] = Vector3.Cross(vertices[1]-vertices[0], vertices[3]-vertices[0]).normalized;
			normals[5] = Vector3.Cross(vertices[7]-vertices[4], vertices[5]-vertices[4]).normalized;

			float[] distances = new float[6];
			distances[0] = -Vector3.Dot(vertices[0], normals[0]) - psize;
			distances[1] = -Vector3.Dot(vertices[1], normals[1]) - psize;
			distances[2] = -Vector3.Dot(vertices[0], normals[2]) - psize;
			distances[3] = -Vector3.Dot(vertices[3], normals[3]) - psize;
			distances[4] = -Vector3.Dot(vertices[0], normals[4]) - psize;
			distances[5] = -Vector3.Dot(vertices[4], normals[5]) - psize;

			objid = ownerid;
			center.x = trans.m03;
			center.y = trans.m13;
			center.z = trans.m23;
			bb.SetMinMax(center, center);

			mpPlane[] planes = new mpPlane[6];
			planes[0] = plane0;
			planes[1] = plane1;
			planes[2] = plane2;
			planes[3] = plane3;
			planes[4] = plane4;
			planes[5] = plane5;
			for (int i = 0; i < 6; ++i)
			{
				planes[i].normal = normals[i];
				planes[i].distance = distances[i];
			}
			for (int i = 0; i < vertices.Length; ++i)
			{
				Vector3 pos = center + vertices[i];
				bb.Contains(pos - psize3);
				bb.Contains(pos + psize3);
			}
		}
	}

	public struct mpSphereCollider
	{
		public int objid;
		public Bounds bb;
		public Vector3 center;
		public float radius;

		public void Build(int ownerid, Vector3 center_, float radius_)
		{
			float psize = mpSystem.instance.kparams.ParticleSize;
			float er = radius + psize;
			Vector3 er3 = Vector3.one * er;
			objid = ownerid;
			center = center_;
			radius = radius_;
			bb.SetMinMax(center, center);
			bb.Contains(center - er3);
			bb.Contains(center + er3);
		}
	};


	public struct mpForce
	{
		public mp.mpForceShape		shape_type;
		public mp.mpForceDirection	dir_type;
		public Bounds				bb;
		public mpSphere				shape_sphere;
		public mpBox				shape_box;
		public mp.mpForceParams		fparams;
	}

}
#endif // MP_PURE_CSHARP
