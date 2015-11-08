using System.Collections;
using System.Runtime.InteropServices;
using UnityEngine;
using Ist;

public class ShowNumParticles : MonoBehaviour
{
    GUIText text;

    void Start ()
    {
        text = GetComponent<GUIText>();
    }
    
    // Update is called once per frame
    void Update () {
        text.text = "particles: " + MPWorld.s_instances[0].m_particle_num;
    }
}
