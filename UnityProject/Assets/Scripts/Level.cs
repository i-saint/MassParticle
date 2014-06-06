using UnityEngine;
using System.Collections;

public class Level : MonoBehaviour {
	
	public GameObject enemyLargeCube;
	public GameObject enemySmallCube;

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		if (Time.frameCount % 200==0) {
			Vector3 pos = new Vector3(Random.Range (-3.0f, 3.0f), 0.0f, Random.Range (5.0f, 10.0f));
			Instantiate(enemyLargeCube, pos, Quaternion.identity);
		}
		if (Time.frameCount % 50==0) {
			Vector3 pos = new Vector3(Random.Range (-3.0f, 3.0f), 0.0f, Random.Range (5.0f, 10.0f));
			Instantiate(enemySmallCube, pos, Quaternion.identity);
		}
	}
}
