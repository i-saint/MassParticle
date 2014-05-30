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

public class mpEmitter : MonoBehaviour {
	
	// Native plugin rendering events are only called if a plugin is used
	// by some script. This means we have to DllImport at least
	// one function in some active script.
	// For this example, we'll call into plugin's SetTimeFromUnity
	// function and pass the current time so the plugin can animate.
	[DllImport ("MassParticle")]
	private static extern void SetTimeFromUnity (float t);


	// We'll also pass native pointer to a texture in Unity.
	// The plugin will fill texture data from native code.
	[DllImport ("MassParticle")]
	private static extern void SetTextureFromUnity (IntPtr texture);
	
	[DllImport ("MassParticle")]
	private static extern void mpSetViewProjectionMatrix(Matrix4x4 view, Matrix4x4 proj);

    [DllImport("MassParticle")]
    private static extern void mpSetRenderTargets(IntPtr renderTexture, IntPtr depthTexture);

    [DllImport("MassParticle")]
    private static extern void mpClearParticles();

    [DllImport ("MassParticle")] private static extern uint mpGetNumParticles(IntPtr ctx);
    [DllImport ("MassParticle")] private static extern uint mpAddBoxCollider(IntPtr ctx, Matrix4x4 transform, Vector3 size);
    [DllImport ("MassParticle")] private static extern uint mpAddSphereCollider(IntPtr ctx, Vector3 center, float radius);
    [DllImport ("MassParticle")] private static extern bool mpRemoveCollider(IntPtr ctx, uint handle);

	[DllImport ("MassParticle")] private static extern uint mpScatterParticlesSphererical(IntPtr ctx, Vector3 center, float radius, uint num);
	[DllImport ("MassParticle")] private static extern uint mpAddDirectionalForce (IntPtr ctx, Vector3 direction, float strength);
	[DllImport ("MassParticle")] private static extern void mpUpdate (float dt);


	public RenderTexture renderTarget;


	void Start () {
        mpClearParticles();
	}

	void Update()
	{
		mpScatterParticlesSphererical (IntPtr.Zero, transform.position, 0.5f, 32);
		mpAddDirectionalForce (IntPtr.Zero, new Vector3(0.0f,-1.0f,0.0f), 10.0f);
		mpUpdate (Time.timeSinceLevelLoad);
	}

	private void CreateTextureAndPassToPlugin()
	{
		// Create a texture
		Texture2D tex = new Texture2D(256,256,TextureFormat.ARGB32,false);
		// Set point filtering just so we can see the pixels clearly
		tex.filterMode = FilterMode.Point;
		// Call Apply() so it's actually uploaded to the GPU
		tex.Apply();

		// Set texture onto our matrial
		renderer.material.mainTexture = tex;

		// Pass texture pointer to the plugin
		SetTextureFromUnity (tex.GetNativeTexturePtr());
	}
	
	void OnRenderObject()
	{
			
			// Set time for the plugin
			SetTimeFromUnity (Time.timeSinceLevelLoad);

			
			UnityEngine.Camera cam = UnityEngine.Camera.current;
			if (cam) {
				mpSetViewProjectionMatrix(cam.worldToCameraMatrix, cam.projectionMatrix);
                //peSetRenderTargets(
                //    cam.targetTexture.colorBuffer,
                //    cam.depthTextureMode.);
 			}
            {
                Collider[] colliders = Physics.OverlapSphere( transform.position, 10.0f );
                for (int i = 0; i < colliders.Length; ++i )
                {
                    Collider col = colliders[i];
                    SphereCollider sphere = col as SphereCollider;
                    BoxCollider box = col as BoxCollider;
                    if(sphere) {
                        mpAddSphereCollider(IntPtr.Zero,
                            sphere.transform.position,
                            sphere.radius);
                    }
                    else if (box)
                    {
                        mpAddBoxCollider(IntPtr.Zero,
                            box.transform.localToWorldMatrix,
                            box.size);
                    }
                }
            }

			// Issue a plugin event with arbitrary integer identifier.
			// The plugin can distinguish between different
			// things it needs to do based on this ID.
			// For our simple plugin, it does not matter which ID we pass here.
			if(renderTarget) {
				Graphics.SetRenderTarget(renderTarget);
			}

			GL.IssuePluginEvent (1);
	}
}
