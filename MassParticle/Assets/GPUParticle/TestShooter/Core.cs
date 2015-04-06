using UnityEngine;
using System.Collections;

public class Core : MonoBehaviour
{
	Vector4 glowColor = new Vector4(0.2f, 0.0f, 0.0f, 0.0f);

	void Start ()
	{
	
	}
	
	void Update()
	{
		MeshRenderer mr = GetComponent<MeshRenderer>();
		mr.material.SetVector("_GlowColor", glowColor);

		Vector3 pos = transform.position;
		pos.x = 9.5f;
		pos.y = Mathf.Cos(Time.time*0.5f) * 3.0f;
		transform.position = pos;

		TestShooter ts = TestShooter.instance;
		{
			//const int num = 52;
			const int num = 16;
			MPGPParticle[] additional = new MPGPParticle[num];
			for (int i = 0; i < additional.Length; ++i)
			{
				additional[i].velocity = new Vector3(Random.Range(-2.0f, -0.5f), Random.Range(-1.0f, 1.0f), 0.0f) * 4.0f;
				additional[i].position = new Vector3(pos.x - 1.4f, pos.y, 0.0f) + additional[i].velocity*0.1f;
			}
			ts.fractions.AddParticles(additional);
		}
	}
}
