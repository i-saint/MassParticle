using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace mpCS
{
	[StructLayout(LayoutKind.Explicit)]
	public struct mpParticle
	{
		[FieldOffset( 0)] public Vector4 position;	// union
		[FieldOffset( 0)] public Vector3 position3; // 
		[FieldOffset(16)] public Vector4 velocity;	// union
		[FieldOffset(16)] public Vector3 velocity3;	// 
		[FieldOffset(28)] public float speed;		// 
		[FieldOffset(32)] public float density;
		[FieldOffset(36)] public uint hash;		// union
		[FieldOffset(36)] public int hit_prev;	// 
		[FieldOffset(40)] public int hit;
		[FieldOffset(44)] public float lifetime;
	};

	struct mpIMData
	{
		public Vector3 accel;
	}

}