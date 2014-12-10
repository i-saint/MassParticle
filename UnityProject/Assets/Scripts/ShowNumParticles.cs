using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

public class ShowNumParticles : MonoBehaviour
{
	GUIText text;

	void Start ()
	{
		text = GetComponent<GUIText>();
	}
	
	// Update is called once per frame
	void Update () {
		text.text = "particles: " + MPWorld.instances[0].particleNum;
	}
}
