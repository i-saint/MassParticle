using UnityEngine;
using System;
using System.Collections;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Explicit)]
public struct mpParticle
{
    [FieldOffset( 0)] Vector4 position;
    [FieldOffset(16)] Vector4 velocity;
    [FieldOffset(32)] float density;
    [FieldOffset(36)] uint hash;
    [FieldOffset(40)] int hit;
    [FieldOffset(44)] float lifetime;
}

public enum mpSolverType
{
	Impulse = 0,
	SPH = 1,
	SPHEstimate = 2,
}

public class mpEmitter : MonoBehaviour {

	[DllImport ("MassParticle")]
	private static extern void mpSetViewProjectionMatrix(Matrix4x4 view, Matrix4x4 proj);

    [DllImport("MassParticle")]
    private static extern void mpSetRenderTargets(IntPtr renderTexture, IntPtr depthTexture);

    [DllImport("MassParticle")]
    private static extern void mpClearParticles();
	
	[DllImport ("MassParticle")] private static extern void mpSetSolverType(mpSolverType t);
	[DllImport ("MassParticle")] private static extern float mpGetParticleLifeTime();
	[DllImport ("MassParticle")] private static extern void mpSetParticleLifeTime(float lifetime);
	[DllImport ("MassParticle")] private static extern uint mpGetNumParticles();

	[DllImport ("MassParticle")] private static extern uint mpAddBoxCollider(Matrix4x4 transform, Vector3 size);
    [DllImport ("MassParticle")] private static extern uint mpAddSphereCollider(Vector3 center, float radius);
    [DllImport ("MassParticle")] private static extern bool mpRemoveCollider(uint handle);

	[DllImport ("MassParticle")] private static extern uint mpScatterParticlesSphererical(Vector3 center, float radius, uint num);
	[DllImport ("MassParticle")] private static extern uint mpAddDirectionalForce (Vector3 direction, float strength);
	[DllImport ("MassParticle")] private static extern void mpUpdate (float dt);

	public float particleRadius;
	public float particleLifeTime;
	public mpSolverType solverType;

	public float gravityStrength = 10.0f;
	public Vector3 gravityDirection = new Vector3(0.0f,-1.0f,0.0f);


	void Start () {
        mpClearParticles();
		particleLifeTime = mpGetParticleLifeTime();
	}

	void Update()
	{
		mpScatterParticlesSphererical (transform.position, 0.5f, 32);
		mpAddDirectionalForce (gravityDirection, gravityStrength);

		mpSetSolverType(solverType);
		mpSetParticleLifeTime(particleLifeTime);
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
