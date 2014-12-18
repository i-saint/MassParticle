using UnityEngine;
using System.Collections;
using System.Collections.Generic;

[RequireComponent(typeof(Light))]
public class DSLight : MonoBehaviour
{
    public static List<DSLight> instances = new List<DSLight>();

    void OnEnable()
    {
        instances.Add(this);
    }

    void OnDisable()
    {
        instances.Remove(this);
    }

    static public void RenderLights(DSRenderer dsr)
    {
        foreach(DSLight l in instances) {
            Vector4 c = l.lit.color * l.lit.intensity;
            Vector4 shadow = Vector4.zero;
            shadow.x = l.castShadow ? 1.0f : 0.0f;
            shadow.y = (float)l.shadowSteps;

            l.mat.SetTexture("_NormalBuffer", dsr.rtNormalBuffer);
            l.mat.SetTexture("_PositionBuffer", dsr.rtPositionBuffer);
            l.mat.SetTexture("_ColorBuffer", dsr.rtColorBuffer);
            l.mat.SetTexture("_GlowBuffer", dsr.rtGlowBuffer);
            l.mat.SetVector("_LightColor", c);
            l.mat.SetVector("_ShadowParams", shadow);

            if (l.lit.type == LightType.Point)
            {
                Matrix4x4 trans = Matrix4x4.TRS(l.transform.position, Quaternion.identity, Vector3.one);
                Vector4 range = Vector4.zero;
                range.x = l.lit.range;
                range.y = 1.0f / range.x;
                l.mat.SetVector("_LightPosition", l.transform.position);
                l.mat.SetVector("_LightRange", range);
                l.mat.SetPass(0);
                Graphics.DrawMeshNow(l.mesh, trans);
            }
            else if (l.lit.type == LightType.Directional)
            {
                l.mat.SetVector("_LightDir", l.lit.transform.forward);
                l.mat.SetPass(0);
                DSRenderer.DrawFullscreenQuad();
            }
        }
    }


    public bool castShadow = true;
    public int shadowSteps = 10;
    public Mesh mesh;
    public Material mat;
    public Light lit;

    void Start ()
    {
        lit = GetComponent<Light>();
    }
    
    void Update ()
    {
    
    }
}
