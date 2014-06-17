#if UNITY_EDITOR
using UnityEngine;
using System.Collections;
using UnityEditor;
using System.Runtime.InteropServices;

[CustomEditor(typeof(mpWorld))]
public class mpWorldEditor : Editor {

	public override void OnInspectorGUI()
	{
		DrawDefaultInspector();

		if(GUILayout.Button("Reload Shader"))
		{
			mp.mpReloadShader();
		}
	}
}
#endif // UNITY_EDITOR
