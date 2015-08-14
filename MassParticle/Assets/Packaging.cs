#if UNITY_EDITOR
using System;
using System.Collections;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEditor;


public class Packaging
{
    [MenuItem("Assets/MassParticle/MakePackages")]
    public static void MakePackages()
    {
        string[] files = new string[]
        {
"Assets/Ist",
"Assets/smcs.rsp",
        };
        AssetDatabase.ExportPackage(files, "MassParticle.unitypackage", ExportPackageOptions.Recurse);
    }

}
#endif
