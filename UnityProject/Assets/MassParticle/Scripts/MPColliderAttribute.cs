using UnityEngine;
using System.Collections;

public class MPColliderAttribute : MonoBehaviour {

	public bool sendCollision = true;
	public bool receiveCollision = false;
	public uint groupMask = 0xffffffff;
	public float stiffness = 1500.0f;
	public float bounce = 1.0f;
	public float damageOnHit = 0.0f;
	public MPColliderProperties cprops;

	// Use this for initialization
	protected void Start()
	{
	
	}
	
	// Update is called once per frame
	protected void Update()
	{
		cprops.group_mask = groupMask;
		cprops.stiffness = stiffness;
		cprops.bounce = bounce;
		cprops.damage_on_hit = damageOnHit;
	}
}
