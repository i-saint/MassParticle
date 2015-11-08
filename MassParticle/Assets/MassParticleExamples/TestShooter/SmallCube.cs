using System.Collections;
using UnityEngine;
using Ist;

public class SmallCube : MonoBehaviour
{
    static MPGPParticle[] particles_to_add;

    void Start()
    {
        GetComponent<TSEntity>().cbDestroyed = () => { CBDestroy(); };
    }

    static float R(float r=1.0f)
    {
        return Random.Range(-r, r);
    }

    void CBDestroy()
    {
        TestShooter ts = TestShooter.instance;
        switch(ts.gameMode) {
            case TestShooter.GameMode.BulletHell: DestroyBH(); break;
            case TestShooter.GameMode.Exception: DestroyEX(); break;
        }
    }

    void DestroyBH()
    {
        TestShooter ts = TestShooter.instance;
        Vector3 pos = transform.position;

        if (particles_to_add == null)
        {
            particles_to_add = new MPGPParticle[512];
        }
        for (int i = 0; i < particles_to_add.Length; ++i)
        {
            particles_to_add[i].position = new Vector3(pos.x, pos.y, 0.0f);
            particles_to_add[i].velocity = new Vector3(Random.Range(-1.0f, 1.0f), Random.Range(-1.0f, 1.0f), 0.0f) * 8.0f;
        }
        ts.fractions.AddParticles(particles_to_add);
    }

    void DestroyEX()
    {
        TestShooter ts = TestShooter.instance;
        Vector3 pos = transform.position;

        if (particles_to_add == null)
        {
            particles_to_add = new MPGPParticle[512];
        }
        for (int i = 0; i < particles_to_add.Length; ++i)
        {
            particles_to_add[i].position = new Vector3(pos.x + R(0.5f), pos.y + R(0.5f), pos.z + R(0.5f));
            particles_to_add[i].velocity = new Vector3(R(), R(), 0.0f) * 2.0f;
        }
        ts.fractions.AddParticles(particles_to_add);
    }
}
