using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class mp {
	
	[StructLayout(LayoutKind.Explicit)]
	public struct mpParticle
	{
		[FieldOffset( 0)] public Vector4 position;
		[FieldOffset(16)] public Vector4 velocity;
		[FieldOffset(32)] public float density;
		[FieldOffset(36)] public int hit_prev;
		[FieldOffset(40)] public int hit;
		[FieldOffset(44)] public float lifetime;
	}
	
	[StructLayout(LayoutKind.Explicit)]
	public struct mpKernelParams {
		[FieldOffset( 0)] public Vector3  WorldCenter;
		[FieldOffset(16)] public Vector3  WorldSize;
		[FieldOffset(32)] public Vector3  Scaler;
		[FieldOffset(48)] public int SolverType;
		[FieldOffset(52)] public float LifeTime;
		[FieldOffset(56)] public float Timestep;
		[FieldOffset(60)] public float Decelerate;
		[FieldOffset(64)] public float PressureStiffness;
		[FieldOffset(68)] public float WallStiffness;
		[FieldOffset(72)] public float SPHRestDensity;
		[FieldOffset(76)] public float SPHParticleMass;
		[FieldOffset(80)] public float SPHViscosity;
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

	
	[DllImport ("MassParticle")] public static extern void mpSetViewProjectionMatrix(Matrix4x4 view, Matrix4x4 proj);
	[DllImport ("MassParticle")] public static extern void mpSetRenderTargets(IntPtr renderTexture, IntPtr depthTexture);
	[DllImport ("MassParticle")] public static extern void mpClearParticles();
	[DllImport ("MassParticle")] public static extern void mpReloadShader ();
	[DllImport ("MassParticle")] public static extern mpKernelParams mpGetKernelParams();
	[DllImport ("MassParticle")] public static extern void mpSetKernelParams(ref mpKernelParams p);

	[DllImport ("MassParticle")] public static extern uint mpGetNumParticles();
	[DllImport ("MassParticle")] unsafe public static extern mpParticle* mpGetParticles();
	[DllImport ("MassParticle")] unsafe public static extern void mpCopyParticles (mpParticle *dst);
	[DllImport ("MassParticle")] unsafe public static extern void mpWriteParticles(mpParticle *from);

	[DllImport ("MassParticle")] public static extern uint mpAddBoxCollider(int owner, Matrix4x4 transform, Vector3 size);
	[DllImport ("MassParticle")] public static extern uint mpAddSphereCollider(int owner, Vector3 center, float radius);
	
	[DllImport ("MassParticle")] public static extern void mpUpdate (float dt);


	[DllImport ("MassParticle")] public static extern uint mpScatterParticlesSphere(Vector3 center, float radius, int num, Vector3 velBase, float velDiffuse);
	[DllImport ("MassParticle")] public static extern uint mpScatterParticlesBox(Vector3 center, Vector3 size, int num, Vector3 velBase, float velDiffuse);
	[DllImport ("MassParticle")] public static extern uint mpScatterParticlesSphereTransform(Matrix4x4 trans, int num, Vector3 velBase, float velDiffuse);
	[DllImport ("MassParticle")] public static extern uint mpScatterParticlesBoxTransform(Matrix4x4 trans, int num, Vector3 velBase, float velDiffuse);
	
	[DllImport ("MassParticle")] public static extern uint mpAddDirectionalForce (Vector3 direction, float strength);
	[DllImport ("MassParticle")] public static extern uint mpAddForce (mpForceShape regionType, Matrix4x4 regionMat, mpForceDirection dirType, mpForceParams p);


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

