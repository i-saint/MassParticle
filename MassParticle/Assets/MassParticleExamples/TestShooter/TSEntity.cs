using System.Collections.Generic;
using UnityEngine;
using Ist;

public class TSEntity : MonoBehaviour
{
    public enum EntityType {
        Player,
        Enemy,
        Ground,
        PlayerBullet,
        Other,
    }

    public delegate void Callback();
    public static Vector4 damageFlashColor = new Vector4(0.10f, 0.025f, 0.02f, 0.0f);
    Material matBase;
    public EntityType entityType;
    public Rigidbody rigid;
    public Transform trans;
    public int frame = 0;
    public float life = 100.0f;
    float deltaDamage;
    public Vector3 accel;
    public float deccel = 0.99f;
    public float maxSpeed = 5.0f;
    public Callback cbDestroyed;

    void Start()
    {
        rigid = GetComponent<Rigidbody>();
        trans = GetComponent<Transform>();
        MeshRenderer mr = GetComponent<MeshRenderer>();
        matBase = new Material(mr.material.shader);
        mr.material = matBase;
        accel = TestShooter.instance.globalAccel;
    }
    
    void Update()
    {
        ++frame;

        if (rigid)
        {
            Vector3 vel = rigid.velocity;
            vel += accel;
            vel.z = 0.0f;
            rigid.velocity = vel;

            Vector3 pos = rigid.transform.position;
            pos.z *= 0.95f;
            rigid.transform.position = pos;

            float speed = rigid.velocity.magnitude;
            rigid.velocity = rigid.velocity.normalized * (Mathf.Min(speed, maxSpeed) * deccel);

            rigid.angularVelocity *= 0.95f;
        }
        
        if (deltaDamage > 0.0f && frame % 4 < 2)
        {
            matBase.SetVector("_GlowColor", damageFlashColor);
        }
        else
        {
            matBase.SetVector("_GlowColor", Vector4.zero);
        }

        life -= deltaDamage;
        deltaDamage = 0.0f;
        if (life <= 0.0f)
        {
            Destroy(gameObject);
            if (cbDestroyed != null) { cbDestroyed.Invoke(); }
        }
        if (Mathf.Abs(trans.position.x) > 30.0f || Mathf.Abs(trans.position.z) > 30.0f)
        {
            Destroy(gameObject);
        }
    }

    public void OnDamage(float damage, int damageFrom)
    {
        deltaDamage += damage;
    }

    public void OnHitParticle(ref MPGPParticle particle)
    {
        if (particle.speed > 12.5f)
        {
            //OnDamage(1.0f, particle.owner_objid);
            OnDamage(1.0f, 0);
            particle.lifetime = 0.0f;
        }
    }

    void OnTriggerEnter(Collider col)
    {
        PlayerBullet bul = col.GetComponent<PlayerBullet>();
        if (bul!=null && entityType == EntityType.Enemy)
        {
            OnDamage(bul.power, 0);
            Destroy(col.gameObject);
        }
    }
}
