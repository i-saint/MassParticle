using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class mp {
	
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


	[StructLayout(LayoutKind.Explicit)]
	public struct mpKernelParams {
		[FieldOffset(  0)] public Vector3  WorldCenter;
		[FieldOffset( 16)] public Vector3  WorldSize;
		[FieldOffset( 32)] public int  WorldDiv_x;
		[FieldOffset( 36)] public int  WorldDiv_y;
		[FieldOffset( 40)] public int  WorldDiv_z;
		[FieldOffset( 48)] public Vector3  Scaler;
		[FieldOffset( 64)] public int SolverType;
		[FieldOffset( 68)] public float LifeTime;
		[FieldOffset( 72)] public float Timestep;
		[FieldOffset( 76)] public float Decelerate;
		[FieldOffset( 80)] public float PressureStiffness;
		[FieldOffset( 84)] public float WallStiffness;
		[FieldOffset( 88)] public int MaxParticles;
		[FieldOffset( 92)] public float ParticleSize;
		[FieldOffset( 96)] public float SPHRestDensity;
		[FieldOffset(100)] public float SPHParticleMass;
		[FieldOffset(104)] public float SPHViscosity;
	};
	
	public enum mpSolverType
	{
		Impulse = 0,
		SPH = 1,
		SPHEstimate = 2,
		NoInteraction = 3,
	}


	public enum mpForceShape {
		All,
		Sphere,
		Box
	}

	public enum mpForceDirection {
		Directional,
		Radial,
	}

	[StructLayout(LayoutKind.Explicit)]
	public struct mpForceParams
	{
		[FieldOffset( 0)] public Vector3 pos;
		[FieldOffset( 0)] public Vector3 dir;
		[FieldOffset(12)] public float strength;
		[FieldOffset(16)] public float strengthRandomDiffuse;
	}


	[DllImport ("MassParticle")] public static extern void mpUpdate (float dt);
	[DllImport ("MassParticle")] public static extern void mpClearParticles();
	[DllImport ("MassParticle")] public static extern void mpClearCollidersAndForces();
	[DllImport ("MassParticle")] public static extern void mpSetViewProjectionMatrix(Matrix4x4 view, Matrix4x4 proj, Vector3 cameraPos);
	[DllImport ("MassParticle")] public static extern void mpReloadShader ();
	[DllImport ("MassParticle")] public static extern mpKernelParams mpGetKernelParams();
	[DllImport ("MassParticle")] public static extern void mpSetKernelParams(ref mpKernelParams p);

	[DllImport ("MassParticle")] public static extern int mpGetNumParticles();
	[DllImport ("MassParticle")] unsafe public static extern mpParticle* mpGetParticles();
	[DllImport ("MassParticle")] unsafe public static extern void mpCopyParticles (mpParticle *dst);
	[DllImport ("MassParticle")] unsafe public static extern void mpWriteParticles(mpParticle *from);


	[DllImport ("MassParticle")] public static extern void mpAddSphereCollider(int owner, Vector3 center, float radius);
	[DllImport ("MassParticle")] public static extern void mpAddBoxCollider(int owner, Matrix4x4 transform, Vector3 size);
	[DllImport ("MassParticle")] public static extern void mpScatterParticlesSphere(Vector3 center, float radius, int num, Vector3 velBase, float velDiffuse);
	[DllImport ("MassParticle")] public static extern void mpScatterParticlesBox(Vector3 center, Vector3 size, int num, Vector3 velBase, float velDiffuse);
	[DllImport ("MassParticle")] public static extern void mpScatterParticlesSphereTransform(Matrix4x4 trans, int num, Vector3 velBase, float velDiffuse);
	[DllImport ("MassParticle")] public static extern void mpScatterParticlesBoxTransform(Matrix4x4 trans, int num, Vector3 velBase, float velDiffuse);
	[DllImport ("MassParticle")] public static extern void mpAddForce (mpForceShape regionType, Matrix4x4 regionMat, mpForceDirection dirType, mpForceParams p);


	// utilities

	public static void mpAddRadialSphereForce(Vector3 pos, float radius, float strength)
	{
		Matrix4x4 mat = Matrix4x4.TRS(pos, Quaternion.identity, Vector3.one * radius);
		mpForceParams p = new mpForceParams();
		p.strength = strength;
		p.pos = pos;
		mpAddForce (mp.mpForceShape.Sphere, mat, mp.mpForceDirection.Radial, p);
	}
}

