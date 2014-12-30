using UnityEngine;
using System.Collections;

public class SceneTestDeferredShading : MonoBehaviour
{
    public GameObject cam;
    public float rotateRadius = 10.0f;
    public float cameraY = 7.5f;
    public float cameraRot = 0.0f;
    public GameObject mpWorldObj;
    public GameObject emitterObj;
    public Material matBloom;
    public Material matReflection;
    public Material matGlowline;

    bool showGUI = true;
    MPWorld world;
    MPEmitter emitter;

    void Start ()
    {
        world = mpWorldObj.GetComponent<MPWorld>();
        emitter = emitterObj.GetComponent<MPEmitter>();
    }
    
    void Update ()
    {
        float wheel = Input.GetAxis("Mouse ScrollWheel");

        if (wheel < 0) { cameraY -= 0.4f;  }
        else if (wheel > 0) { cameraY += 0.4f;  }

        if (Input.GetKey(KeyCode.UpArrow) || Input.GetKey(KeyCode.W)) { rotateRadius -= 0.2f; }
        else if (Input.GetKey(KeyCode.DownArrow) || Input.GetKey(KeyCode.S)) { rotateRadius += 0.2f; }

        if (Input.GetKey(KeyCode.LeftArrow) || Input.GetKey(KeyCode.A)) { cameraRot -= 0.01f; }
        else if (Input.GetKey(KeyCode.RightArrow) || Input.GetKey(KeyCode.D)) { cameraRot += 0.01f; }
        //cameraRot += Time.deltaTime * 0.10f;

        cam.transform.position = new Vector3(
            Mathf.Cos(cameraRot) * rotateRadius,
            cameraY,
            Mathf.Sin(cameraRot) * rotateRadius);
        cam.transform.LookAt(Vector3.zero);

        if (Input.GetKeyUp(KeyCode.Space))
        {
            showGUI = !showGUI;
        }
    }

    
    void OnGUI()
    {
        float lineheight = 22.0f;
        float margin = 0.0f;
        float labelWidth = 130.0f;
        float x = 10.0f;
        float y = 10.0f;
        float tmpf = 0.0f;

        if (!showGUI) { return; }

        GUI.Label(new Rect(x, y, labelWidth, lineheight), "particles: " + world.m_particle_num);
        y += lineheight + margin;

        GUI.Label(new Rect(x, y, labelWidth, lineheight), "timescale:");
        GUI.TextField(new Rect(x + labelWidth, y, 50, lineheight), Time.timeScale.ToString());
        Time.timeScale = GUI.HorizontalSlider(new Rect(x + labelWidth+55, y, 100, lineheight), Time.timeScale, 0.0f, 3.0f);
        y += lineheight + margin;

        GUI.Label(new Rect(x, y, labelWidth, lineheight), "particles par frame:");
        GUI.TextField(new Rect(x + labelWidth, y, 50, lineheight), emitter.m_emit_count.ToString());
        emitter.m_emit_count = (int)GUI.HorizontalSlider(new Rect(x + labelWidth + 55, y, 100, lineheight), emitter.m_emit_count, 0, 500);
        y += lineheight + margin;

        GUI.Label(new Rect(x, y, labelWidth, lineheight), "particles deceleration:");
        GUI.TextField(new Rect(x + labelWidth, y, 50, lineheight), world.m_deceleration.ToString());
        world.m_deceleration = GUI.HorizontalSlider(new Rect(x + labelWidth + 55, y, 100, lineheight), world.m_deceleration, 0.9f, 1.0f);
        y += lineheight + margin;

        y += 10.0f;

        tmpf = matBloom.GetFloat("_Intensity");
        GUI.Label(new Rect(x, y, labelWidth, lineheight), "bloom intensity:");
        GUI.TextField(new Rect(x + labelWidth, y, 50, lineheight), tmpf.ToString());
        matBloom.SetFloat("_Intensity", GUI.HorizontalSlider(new Rect(x + labelWidth + 55, y, 100, lineheight), tmpf, 0.0f, 5.0f));
        y += lineheight + margin;

        tmpf = matReflection.GetFloat("_Intensity");
        GUI.Label(new Rect(x, y, labelWidth, lineheight), "reflection intensity:");
        GUI.TextField(new Rect(x + labelWidth, y, 50, lineheight), tmpf.ToString());
        matReflection.SetFloat("_Intensity", GUI.HorizontalSlider(new Rect(x + labelWidth + 55, y, 100, lineheight), tmpf, 0.0f, 5.0f));
        y += lineheight + margin;

        tmpf = matGlowline.GetFloat("_Intensity");
        GUI.Label(new Rect(x, y, labelWidth, lineheight), "glowline intensity:");
        GUI.TextField(new Rect(x + labelWidth, y, 50, lineheight), tmpf.ToString());
        matGlowline.SetFloat("_Intensity", GUI.HorizontalSlider(new Rect(x + labelWidth + 55, y, 100, lineheight), tmpf, 0.0f, 5.0f));
        y += lineheight + margin;

        y += 10.0f;

        GUI.Label(new Rect(x, y, 300, lineheight), "WASD & MOUSE WHEEL: move camera");
        y += lineheight + margin;

        GUI.Label(new Rect(x, y, 300, lineheight), "SPACE: show / hide GUI");
        y += lineheight + margin;
    }
}
