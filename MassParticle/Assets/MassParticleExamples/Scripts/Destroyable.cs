using System.Collections;
using UnityEngine;
using Ist;

public class Destroyable : MonoBehaviour
{
    public float life = 10.0f;
    
    public Vector3 accelDir = Vector3.zero;
    public float accel = 0.02f;
    public float deccel = 0.99f;
    public float maxSpeed = 5.0f;

    public bool scatterFractions = true;

    Rigidbody rigid;
    Transform trans;
    MPCollider m_mpcol;


    void Start()
    {
        rigid = GetComponent<Rigidbody>();
        trans = GetComponent<Transform>();
        m_mpcol = GetComponent<MPCollider>();
        m_mpcol.m_hit_handler = OnHitParticle;
    }
    
    void Update ()
    {
        if(IsDead()) {
            if(scatterFractions) {
                float volume = trans.localScale.x * trans.localScale.y * trans.localScale.z;
                int num = (int)(volume * 500.0f);
                Matrix4x4 mat = trans.localToWorldMatrix;
                MPWorld.s_instances[0].AddOneTimeAction(() => {
                    MPSpawnParams sp = new MPSpawnParams();
                    sp.velocity_random_diffuse = 3.0f;
                    sp.lifetime = 30.0f;
                    MPAPI.mpScatterParticlesBoxTransform(MPWorld.s_instances[0].GetContext(), ref mat, num, ref sp);
                });
            }
            Destroy (gameObject);
        }
        if(Mathf.Abs (trans.position.x)>30.0f ||
           Mathf.Abs (trans.position.z)>30.0f )
        {
            Destroy (gameObject);
        }

        
        if(rigid) {
            Vector3 vel = rigid.velocity;
            vel.x -= accel;
            vel.y = 0.0f;
            rigid.velocity = vel;
            
            Vector3 pos = rigid.transform.position;
            pos.y *= 0.95f;
            rigid.transform.position = pos;
            
            float speed = rigid.velocity.magnitude;
            rigid.velocity = rigid.velocity.normalized * (Mathf.Min (speed, maxSpeed) * deccel);
            
            rigid.angularVelocity = rigid.angularVelocity * 0.98f;
        }
    }

    public void Damage(float v)
    {
        life -= v;
    }

    public bool IsDead()
    {
        return life<=0.0f;
    }

    void OnDestroy()
    {
        if (MPWorld.s_instances.Count == 0) return;

        Vector3 pos = trans.position;
        Vector3 scale = trans.localScale;
        MPWorld.s_instances[0].AddOneTimeAction(() =>
        {
            float radius = (scale.x + scale.y + scale.z) * 0.5f;
            MPUtils.AddRadialSphereForce(MPWorld.s_instances[0], pos, radius, radius * 100.0f);
        });
    }

    void OnHitParticle(ref MPParticle particle)
    {

        m_mpcol.PropagateHit(ref particle);

        const float VelocityThreshold = 12.5f;

        if (particle.speed > VelocityThreshold)
        {
            particle.lifetime = 0.0f;
            Damage(0.25f);
        }
    }
}
