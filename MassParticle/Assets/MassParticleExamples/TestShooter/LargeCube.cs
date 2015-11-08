using System.Collections;
using UnityEngine;
using Ist;

public class LargeCube : MonoBehaviour
{
    public GameObject enemyMediumCube;

    void Start()
    {
        GetComponent<TSEntity>().cbDestroyed = () => { CBDestroy(); };
    }

    void CBDestroy()
    {
        TestShooter ts = TestShooter.instance;
        if (!ts) { return; }

        Vector3 pos = transform.position;
        for (int i = 0; i < 6; ++i )
        {
            Vector3 r = transform.rotation * new Vector3(Random.Range(-2.0f, 2.0f), Random.Range(-2.0f, 2.0f), Random.Range(-2.0f, 2.0f));
            Instantiate(enemyMediumCube, pos + r, transform.rotation);
        }
    }
}
