#if UNITY_EDITOR
using UnityEngine;
using System.Collections;
using UnityEditor;
using System.Runtime.InteropServices;

[CustomEditor(typeof(MPWorld))]
public class MPWorldEditor : Editor {

	public override void OnInspectorGUI()
	{
		DrawDefaultInspector();

		if(GUILayout.Button("Reload Shader"))
		{
			MPAPI.mpReloadShader();
		}
	}
}
#endif // UNITY_EDITOR
