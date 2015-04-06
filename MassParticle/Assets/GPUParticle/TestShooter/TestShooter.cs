using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class TestShooter : MonoBehaviour
{
    public enum GameMode
    {
        BulletHell,
        Exception,
    }
    public static TestShooter instance;

    public MPGPWorld fractions;
    //public ParticleSet playerBullets;
    //public ParticleSet effectParticles;

    public GameMode gameMode;
    public Vector3 globalAccel;
    public GameObject cam;
    public GameObject bgCube;
    public GameObject enemySmallCube;
    public GameObject enemyMediumCube;
    public GameObject enemyLargeCube;
    public GameObject enemyCore;

    int frame = 0;

    void OnEnable()
    {
        instance = this;
    }

    void OnDisable()
    {
        instance = null;
    }

    void Start()
    {
        if (gameMode==GameMode.BulletHell)
        {
            fractions.handler = (a, b, c) => { EnemyBulletHandler(a, b, c); };
        }
        else if(gameMode==GameMode.Exception)
        {
            fractions.handler = (a, b, c) => { FractionHandler(a, b, c); };
        }
        
        cam.transform.position = new Vector3(-0.5f, -0.5f, -10.0f);
        cam.transform.LookAt(Vector3.zero);

        for (int yi = 0; yi < 15; ++yi)
        {
            for (int xi = 0; xi < 15; ++xi)
            {
                GameObject c = (GameObject)Instantiate(bgCube,
                    new Vector3(2.2f * xi - 15.4f, 2.2f * yi - 15.4f, Random.Range(3.0f, 5.0f) - 1.0f), Quaternion.identity);
                c.transform.localScale = Vector3.one * 2.0f;
            }
        }
    }
    
    void Update()
    {
        ++frame;

        TestShooter ts = TestShooter.instance;
        switch (ts.gameMode)
        {
            case TestShooter.GameMode.BulletHell: UpdateBH(); break;
            case TestShooter.GameMode.Exception: UpdateEX(); break;
        }
    }
    void UpdateBH()
    {
        if (frame % 30 == 0)
        {
            Vector3 pos = new Vector3(Random.Range(-18.0f, -29.0f), Random.Range(-6.0f, 6.0f), 0.0f);
            Instantiate(enemySmallCube, pos, Quaternion.identity);
        }
        if (frame % 200 == 0)
        {
            Vector3 pos = new Vector3(Random.Range(-18.0f, -29.0f), Random.Range(-6.0f, 6.0f), 0.0f);
            Instantiate(enemyMediumCube, pos, Quaternion.identity);
        }
        if (frame % 500 == 0)
        {
            Vector3 pos = new Vector3(Random.Range(-15.0f, -29.0f), Random.Range(-5.0f, 5.0f), 0.0f);
            Instantiate(enemyLargeCube, pos, Quaternion.identity);
        }
    }
    void UpdateEX()
    {
        if (frame % 30 == 0)
        {
            Vector3 pos = new Vector3(Random.Range(18.0f, 29.0f), Random.Range(-6.0f, 6.0f), 0.0f);
            Instantiate(enemySmallCube, pos, Quaternion.identity);
        }
        if (frame % 200 == 0)
        {
            Vector3 pos = new Vector3(Random.Range(18.0f, 29.0f), Random.Range(-6.0f, 6.0f), 0.0f);
            Instantiate(enemyMediumCube, pos, Quaternion.identity);
        }
        if (frame % 500 == 0)
        {
            Vector3 pos = new Vector3(Random.Range(15.0f, 29.0f), Random.Range(-5.0f, 5.0f), 0.0f);
            Instantiate(enemyLargeCube, pos, Quaternion.identity);
        }
    }

    void EnemyBulletHandler(CSParticle[] particles, int num_particles, List<MPGPColliderBase> colliders)
    {
        for (int i = 0; i < num_particles; ++i)
        {
            int hit = particles[i].hit_objid;
            if (particles[i].lifetime != 0.0f && hit != -1 && hit < colliders.Count)
            {
                MPGPColliderBase cscol = colliders[hit];
                if (cscol != null && cscol.m_receive_collision)
                {
                    TSEntity tge = cscol.GetComponent<TSEntity>();
                    if (tge)
                    {
                        tge.OnDamage(0.1f, 0);
                        particles[i].lifetime = 0.0f;
                    }
                }
            }
        }
    }

    void FractionHandler(CSParticle[] particles, int num_particles, List<MPGPColliderBase> colliders)
    {
        for (int i = 0; i < num_particles; ++i)
        {
            int hit = particles[i].hit_objid;
            if (particles[i].lifetime != 0.0f && hit != -1 && hit < colliders.Count)
            {
                MPGPColliderBase cscol = colliders[hit];
                if (cscol != null && cscol.m_receive_collision)
                {
                    TSEntity tge = cscol.GetComponent<TSEntity>();
                    if (tge)
                    {
                        tge.OnHitParticle(ref particles[i]);
                    }
                }
            }
        }
    }

    void OnGUI()
    {
        float lineheight = 22.0f;
        float margin = 0.0f;
        float x = 10.0f;
        float y = 10.0f;
        //GUI.Label(new Rect(x, y, 300, lineheight), "particles: " + fractions.GetNumParticles());
        y += lineheight + margin;
        GUI.Label(new Rect(x, y, 300, lineheight), "left click: shot");
        y += lineheight + margin;

        if (gameMode == GameMode.Exception)
        {
            GUI.Label(new Rect(x, y, 300, lineheight), "middle click: blow");
            y += lineheight + margin;
        }
    }
}
