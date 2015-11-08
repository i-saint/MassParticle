using UnityEngine;
using System.Collections;

public class Level : MonoBehaviour
{
    public GameObject enemyLargeCube;
    public GameObject enemySmallCube;

    // Use this for initialization
    void Start () {
    
    }
    
    // Update is called once per frame
    void Update () {
        if (Time.frameCount % 300==0) {
            Vector3 pos = new Vector3(Random.Range (15.0f, 29.0f), 0.0f, Random.Range (-5.0f, 5.0f));
            Instantiate(enemyLargeCube, pos, Quaternion.identity);
        }
        if (Time.frameCount % 30==0) {
            Vector3 pos = new Vector3(Random.Range (18.0f, 29.0f), 0.0f, Random.Range (-6.0f, 6.0f));
            Instantiate(enemySmallCube, pos, Quaternion.identity);
        }
    }
}
