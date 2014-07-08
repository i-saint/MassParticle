using UnityEngine;
using System.Collections;
using System.Collections.Generic;

[RequireComponent(typeof(Light))]
public class DSLight : MonoBehaviour
{
	static HashSet<DSLight> _instances;
	public static HashSet<DSLight> instances
	{
		get
		{
			if (_instances == null) { _instances = new HashSet<DSLight>(); }
			return _instances;
		}
	}

	void OnEnable()
	{
		instances.Add(this);
	}

	void OnDisable()
	{
		instances.Remove(this);
	}

	static public Mesh sphereMesh;
	static public Material matPointLight;
	static public Material matDirectionalLight;

	static public void RenderLights(DSCamera cam)
	{
		foreach(DSLight l in instances) {
			Vector4 c = l.lit.color * l.lit.intensity;
			Vector4 shadow = Vector4.zero;
			shadow.x = l.castShadow ? 1.0f : 0.0f;
			shadow.y = (float)l.shadowSteps;

			if (l.lit.type == LightType.Point)
			{
				Matrix4x4 trans = Matrix4x4.TRS(l.transform.position, Quaternion.identity, Vector3.one);
				Vector4 range = Vector4.zero;
				Vector4 fs = Vector4.zero;
				range.x = l.lit.range;
				range.y = 1.0f / range.x;
				matPointLight.SetVector("_LightColor", c);
				matPointLight.SetVector("_ShadowParams", shadow);
				matPointLight.SetVector("_LightPosition", l.transform.position);
				matPointLight.SetVector("_LightRange", range);
				if (Vector3.Magnitude(cam.transform.position - l.transform.position) <= range.x*1.1f)
				{
					fs.x = 1.0f;
					matPointLight.SetVector("_Fullscreen", fs);
					matPointLight.SetPass(0);
					DSCamera.DrawFullscreenQuad();
				}
				else
				{
					matPointLight.SetVector("_Fullscreen", fs);
					matPointLight.SetPass(0);
					Graphics.DrawMeshNow(sphereMesh, trans);
				}
			}
			else if (l.lit.type == LightType.Directional)
			{
				matDirectionalLight.SetVector("_LightColor", c);
				matDirectionalLight.SetVector("_LightDir", l.lit.transform.forward);
				matDirectionalLight.SetVector("_ShadowParams", shadow);
				matDirectionalLight.SetPass(0);
				DSCamera.DrawFullscreenQuad();
			}
		}
	}

	public bool castShadow = true;
	public int shadowSteps = 10;
	public Light lit;

	void Start ()
	{
		lit = GetComponent<Light>();
	}
	
	void Update ()
	{
	
	}
}
