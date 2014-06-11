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
		public Vector3 pos;
		public float radius;
	}

	public struct mpPlane
	{
		public Vector3 normal;
		public float distance;
	}

	public unsafe struct mpBox
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
		public int id;
		public Bounds bb;
		public Vector3 center;
		public mpPlane plane0;
		public mpPlane plane1;
		public mpPlane plane2;
		public mpPlane plane3;
		public mpPlane plane4;
		public mpPlane plane5;
	}

	public struct mpSphereCollider
	{
		public int id;
		public Bounds bb;
		public Vector3 center;
		public float radius;
	};


	public struct mpForce
	{
		public mp.mpForceShape		shape_type;
		public mp.mpForceDirection	dir_type;
		public Bounds				bb;
		public mpSphere				shape_sphere;
		public mpBox				shape_box;
		public Vector3 pos;
		public float strength;
	}

}