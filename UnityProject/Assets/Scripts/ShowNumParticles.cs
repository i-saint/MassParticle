using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

public class ShowNumParticles : MonoBehaviour {
	
	[DllImport ("MassParticle")] private static extern uint mpGetNumParticles();

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		guiText.text = "particles: " + mpGetNumParticles ();
	}
}
