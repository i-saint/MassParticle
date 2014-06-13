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
		[FieldOffset( 88)] public float SPHRestDensity;
		[FieldOffset( 92)] public float SPHParticleMass;
		[FieldOffset( 96)] public float SPHViscosity;
		[FieldOffset(100)] public float ParticleSize;
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

#if MP_PURE_CSHARP

	public static void mpSetViewProjectionMatrix(Matrix4x4 view, Matrix4x4 proj) {}
	public static void mpClearParticles() { mpCS.mpSystem.instance.ClearParticles(); }
	public static void mpReloadShader() { }
	public static mpKernelParams mpGetKernelParams() { return mpCS.mpSystem.instance.kparams;  }
	public static void mpSetKernelParams(ref mpKernelParams p) { mpCS.mpSystem.instance.kparams = p;  }

	public static int mpGetNumParticles() { return mpCS.mpSystem.instance.numActiveParticles; }
	unsafe public static mpParticle[] mpGetParticles() { return mpCS.mpSystem.instance.particles; }
	unsafe public static void mpCopyParticles(mpParticle* dst) { }
	unsafe public static void mpWriteParticles(mpParticle* from) { }


	public static void mpUpdate(float dt) { mpCS.mpSystem.instance.Update(dt); }

	public static uint mpAddSphereCollider(int owner, Vector3 center, float radius) { return mpCS.mpSystem.instance.AddSphereCollider(owner, center, radius); }
	public static uint mpAddBoxCollider(int owner, Matrix4x4 transform, Vector3 size) { return mpCS.mpSystem.instance.AddBoxCollider(owner, transform, size);  }
	//public static uint mpScatterParticlesSphere(Vector3 center, float radius, int num, Vector3 velBase, float velDiffuse) { return 0; }
	//public static uint mpScatterParticlesBox(Vector3 center, Vector3 size, int num, Vector3 velBase, float velDiffuse) { return 0; }
	public static uint mpScatterParticlesSphereTransform(Matrix4x4 trans, int num, Vector3 velBase, float velDiffuse)
	{
		return mpCS.mpSystem.instance.ScatterParticlesSphereTransform(trans, num, velBase, velDiffuse);
	}
	public static uint mpScatterParticlesBoxTransform(Matrix4x4 trans, int num, Vector3 velBase, float velDiffuse)
	{
		return mpCS.mpSystem.instance.ScatterParticlesBoxTransform(trans, num, velBase, velDiffuse);
	}

	public static uint mpAddForce(mpForceShape regionType, Matrix4x4 regionMat, mpForceDirection dirType, mpForceParams p)
	{
		return mpCS.mpSystem.instance.AddForce(regionType, regionMat, dirType, p);
	}

#else

	[DllImport ("MassParticle")] public static extern void mpSetViewProjectionMatrix(Matrix4x4 view, Matrix4x4 proj);
	[DllImport ("MassParticle")] public static extern void mpClearParticles();
	[DllImport ("MassParticle")] public static extern void mpReloadShader ();
	[DllImport ("MassParticle")] public static extern mpKernelParams mpGetKernelParams();
	[DllImport ("MassParticle")] public static extern void mpSetKernelParams(ref mpKernelParams p);

	[DllImport ("MassParticle")] public static extern int mpGetNumParticles();
	[DllImport ("MassParticle")] unsafe public static extern mpParticle* mpGetParticles();
	[DllImport ("MassParticle")] unsafe public static extern void mpCopyParticles (mpParticle *dst);
	[DllImport ("MassParticle")] unsafe public static extern void mpWriteParticles(mpParticle *from);

	[DllImport ("MassParticle")] public static extern void mpUpdate (float dt);

	[DllImport ("MassParticle")] public static extern uint mpAddSphereCollider(int owner, Vector3 center, float radius);
	[DllImport ("MassParticle")] public static extern uint mpAddBoxCollider(int owner, Matrix4x4 transform, Vector3 size);
	[DllImport ("MassParticle")] public static extern uint mpScatterParticlesSphere(Vector3 center, float radius, int num, Vector3 velBase, float velDiffuse);
	[DllImport ("MassParticle")] public static extern uint mpScatterParticlesBox(Vector3 center, Vector3 size, int num, Vector3 velBase, float velDiffuse);
	[DllImport ("MassParticle")] public static extern uint mpScatterParticlesSphereTransform(Matrix4x4 trans, int num, Vector3 velBase, float velDiffuse);
	[DllImport ("MassParticle")] public static extern uint mpScatterParticlesBoxTransform(Matrix4x4 trans, int num, Vector3 velBase, float velDiffuse);
	[DllImport ("MassParticle")] public static extern uint mpAddForce (mpForceShape regionType, Matrix4x4 regionMat, mpForceDirection dirType, mpForceParams p);

#endif // MP_PURE_CSHARP



	// utilities

	public static uint mpAddRadialSphereForce(Vector3 pos, float radius, float strength)
	{
		Matrix4x4 mat = Matrix4x4.TRS(pos, Quaternion.identity, Vector3.one * radius);
		mpForceParams p = new mpForceParams();
		p.strength = strength;
		p.pos = pos;
		return mpAddForce (mp.mpForceShape.Sphere, mat, mp.mpForceDirection.Radial, p);
	}
}

