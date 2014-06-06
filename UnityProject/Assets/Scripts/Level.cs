using UnityEngine;
using System.Collections;

public class Level : MonoBehaviour {

	public GameObject enemy;

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		if (Time.frameCount % 150==0) {
			Vector3 pos = new Vector3(Random.Range (-3.0f, 4.0f), 0.0f, Random.Range (-3.0f, 6.0f));
			Instantiate(enemy, pos, Quaternion.identity);
		}
	}
}
