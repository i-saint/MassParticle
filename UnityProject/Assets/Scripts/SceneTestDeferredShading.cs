using UnityEngine;
using System.Collections;

public class SceneTestDeferredShading : MonoBehaviour
{
	public GameObject cam;

	void Start () {
	
	}
	
	void Update ()
	{
		float t = Time.time * 0.15f;
		float r = 10.5f;
		cam.transform.position = new Vector3(Mathf.Cos(t) * r, 7.5f, Mathf.Sin(t) * r);
		cam.transform.LookAt(Vector3.zero);
	}
}
