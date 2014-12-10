using UnityEngine;
using System.Collections;

[RequireComponent(typeof(DSRenderer))]
public class DSPFog : MonoBehaviour
{
	public Material matFog;
	public Vector4 color = new Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	public float near = 5.0f;
	public float far = 10.0f;
	DSRenderer dscam;

	void Start()
	{
		dscam = GetComponent<DSRenderer>();
		dscam.AddCallbackPostEffect(() => { Render(); }, 1100);
	}

	void Render()
	{
		if (!enabled || matFog == null) { return; }

		matFog.SetTexture("_PositionBuffer", dscam.rtPositionBuffer);
		matFog.SetVector("_Color", color);
		matFog.SetFloat("_Near", near);
		matFog.SetFloat("_Far", far);
		matFog.SetPass(0);
		DSRenderer.DrawFullscreenQuad();
	}
}
