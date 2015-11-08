using System.Collections;
using UnityEngine;
using Ist;

public class MPGPPlayerBullet : MonoBehaviour
{
    Vector4 glowColor = new Vector4(0.3f, 0.3f, 1.5f, 0.0f);

    public float speed = 1.0f;
    public float power = 30.0f;

    void Start()
    {
        MeshRenderer mr = GetComponent<MeshRenderer>();
        mr.material.SetVector("_GlowColor", glowColor);
    }

    void Update()
    {
        if (Mathf.Abs(transform.position.x) > 20.0f ||
           Mathf.Abs(transform.position.z) > 20.0f)
        {
            Destroy(gameObject);
        }
        Vector3 pos = transform.position;
        pos += transform.forward.normalized * speed;
        pos.z = -0.1f;
        transform.position = pos;
    }
}
