using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class StageController : MonoBehaviour
{
    public enum State
    {
        OnGoing,
        Cleared,
    }

    public Camera cam;
    public MPGPWorld fluid;
    public GameObject goal;
    public int targetGoaled = 10000;
    public string nextStage;
    State state = State.OnGoing;
    int numGoaled = 0;
    float elapsed = 0.0f;

    void Start()
    {
        fluid.handler = (a, b, c) => { FluidHandler(a, b, c); };
    }
    
    void Update()
    {
        if (state == State.OnGoing && numGoaled >= targetGoaled)
        {
            state = State.Cleared;
        }

        if (state == State.OnGoing)
        {
            elapsed += Time.deltaTime;
        }

        if (Input.GetMouseButton(0))
        {
            const float MaxRotate = 1.0f;
            float ry = Input.GetAxis("Mouse X") * 1.5f;
            float rxz = Input.GetAxis("Mouse Y") * -1.5f;
            float r = ry + rxz;
            r = Mathf.Max(Mathf.Min(r, MaxRotate), -MaxRotate);
            transform.Rotate(new Vector3(0.0f, 0.0f, r));
        }
        {
            float wheel = Input.GetAxis("Mouse ScrollWheel") * 3.0f;
            Vector3 pos = cam.transform.position;
            pos.z += wheel;
            cam.transform.position = pos;
        }
    }

    void OnGUI()
    {
        float lineheight = 22.0f;
        float margin = 0.0f;
        float x = 10.0f;
        float y = 10.0f;
        //GUI.Label(new Rect(x, y, 300, lineheight), "particles: " + fluid.GetNumParticles());
        y += lineheight + margin;
        GUI.Label(new Rect(x, y, 300, lineheight), "goaled: " + numGoaled + " / " + targetGoaled);
        y += lineheight + margin;
        GUI.Label(new Rect(x, y, 300, lineheight), "time: " + elapsed);
        y += lineheight + margin;
        GUI.Label(new Rect(x, y, 300, lineheight), "mouse drag: rotate frame");
        y += lineheight + margin;
        GUI.Label(new Rect(x, y, 300, lineheight), "mouse wheel: zoom in/out");
        y += lineheight + margin;
        y += lineheight + margin;
        if (state == State.Cleared)
        {
            GUI.Label(new Rect(x, y, 300, lineheight), "cleared!");
            y += lineheight + margin;
            if (nextStage != "")
            {
                bool b = GUI.Button(new Rect(x, y, 150, 30), "next stage");
                if (b)
                {
                    Application.LoadLevel(nextStage);
                }
            }
        }
    }


    void FluidHandler(CSParticle[] particles, int num_particles, List<MPGPColliderBase> colliders)
    {
        for (int i = 0; i < num_particles; ++i)
        {
            int hit = particles[i].hit_objid;
            if (particles[i].lifetime != 0.0f && hit != -1 && hit < colliders.Count)
            {
                MPGPColliderBase cscol = colliders[hit];
                if (cscol != null && cscol.m_receive_collision)
                {
                    particles[i].lifetime = 0.0f;
                    ++numGoaled;
                }
            }
        }
    }
}
