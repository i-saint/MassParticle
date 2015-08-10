using UnityEngine;
using System.Collections;

public class MediumCube : MonoBehaviour
{
	public GameObject enemySmallCube;

	void Start()
	{
		GetComponent<TSEntity>().cbDestroyed = () => { CBDestroy(); };
	}

	void CBDestroy()
	{
		TestShooter ts = TestShooter.instance;
		if (!ts) { return; }

		Vector3 pos = transform.position;
		for (int i = 0; i < 6; ++i)
		{
			Vector3 r = transform.rotation * new Vector3(Random.Range(-1.0f, 1.0f), Random.Range(-1.0f, 1.0f), Random.Range(-1.0f, 1.0f));
			Instantiate(enemySmallCube, pos + r, transform.rotation);
		}
	}
}
