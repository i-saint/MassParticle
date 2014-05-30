using UnityEngine;
using System;
using System.Collections;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Explicit)]
public struct mpParticle
{
    [FieldOffset( 0)] public Vector4 position;
    [FieldOffset(16)] public Vector4 velocity;
    [FieldOffset(32)] public float density;
    [FieldOffset(36)] public uint hash;
    [FieldOffset(40)] public int hit;
    [FieldOffset(44)] public float lifetime;
}

[StructLayout(LayoutKind.Explicit)]
struct mpKernelParams {
	[FieldOffset( 0)] public int SolverType;
	[FieldOffset( 4)] public float LifeTime;
	[FieldOffset( 8)] public float Timestep;
	[FieldOffset(12)] public float Decelerate;
	[FieldOffset(16)] public float SPHPressureStiffness;
	[FieldOffset(20)] public float SPHRestDensity;
	[FieldOffset(24)] public float SPHParticleMass;
	[FieldOffset(28)] public float SPHViscosity;
	[FieldOffset(32)] public float ImpPressureStiffness;
};

public enum mpSolverType
{
	Impulse = 0,
	SPH = 1,
	SPHEstimate = 2,
}

public class mpEmitter : MonoBehaviour {

	[DllImport ("MassParticle")] private static extern void mpSetViewProjectionMatrix(Matrix4x4 view, Matrix4x4 proj);
    [DllImport ("MassParticle")] private static extern void mpSetRenderTargets(IntPtr renderTexture, IntPtr depthTexture);
    [DllImport ("MassParticle")] private static extern void mpClearParticles();
	
	[DllImport ("MassParticle")] private static extern mpKernelParams mpGetKernelParams();
	[DllImport ("MassParticle")] private static extern void mpSetKernelParams(ref mpKernelParams p);


	[DllImport ("MassParticle")] private static extern uint mpGetNumParticles();

	[DllImport ("MassParticle")] private static extern uint mpAddBoxCollider(Matrix4x4 transform, Vector3 size);
    [DllImport ("MassParticle")] private static extern uint mpAddSphereCollider(Vector3 center, float radius);
    [DllImport ("MassParticle")] private static extern bool mpRemoveCollider(uint handle);

	[DllImport ("MassParticle")] private static extern uint mpScatterParticlesSphererical(Vector3 center, float radius, uint num);
	[DllImport ("MassParticle")] private static extern uint mpAddDirectionalForce (Vector3 direction, float strength);
	[DllImport ("MassParticle")] private static extern void mpUpdate (float dt);

	public float particleRadius;
	public float particleLifeTime;
	public float timeStep;
	public float deceleration;
	public mpSolverType solverType;

	public float gravityStrength = 10.0f;
	public Vector3 gravityDirection = new Vector3(0.0f,-1.0f,0.0f);

	mpEmitter()
	{
		mpKernelParams p = mpGetKernelParams();
		particleLifeTime = p.LifeTime;
		timeStep = p.Timestep;
		deceleration = p.Decelerate;
		solverType = (mpSolverType)p.SolverType;
	}

	void Start () {
        mpClearParticles();
	}

	void Update()
	{
		mpScatterParticlesSphererical (transform.position, 0.5f, 32);
		mpAddDirectionalForce (gravityDirection, gravityStrength);

		{
			mpKernelParams p = mpGetKernelParams();
			p.LifeTime = particleLifeTime;
			p.Timestep = timeStep;
			p.Decelerate = deceleration;
			p.SolverType = (int)solverType;
			mpSetKernelParams(ref p);
		}
		mpUpdate (Time.timeSinceLevelLoad);
	}
	
	void OnRenderObject()
	{		
		UnityEngine.Camera cam = UnityEngine.Camera.current;
		if (cam) {
			mpSetViewProjectionMatrix(cam.worldToCameraMatrix, cam.projectionMatrix);
		}

        Collider[] colliders = Physics.OverlapSphere( transform.position, 10.0f );
        for (int i = 0; i < colliders.Length; ++i )
        {
            Collider col = colliders[i];
            SphereCollider sphere = col as SphereCollider;
            BoxCollider box = col as BoxCollider;
            if(sphere) {
                mpAddSphereCollider(sphere.transform.position, sphere.radius);
            }
            else if (box)
            {
                mpAddBoxCollider(box.transform.localToWorldMatrix, box.size);
            }
        }

		// Issue a plugin event with arbitrary integer identifier.
		// The plugin can distinguish between different
		// things it needs to do based on this ID.
		// For our simple plugin, it does not matter which ID we pass here.
		GL.IssuePluginEvent (1);
	}
}
