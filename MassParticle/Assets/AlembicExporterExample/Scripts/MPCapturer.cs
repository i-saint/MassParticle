using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Ist
{

    [RequireComponent(typeof(MPWorld))]
    public class MPCapturer : AlembicCustomComponentCapturer
    {
        aeAPI.aeObject m_abc;

        public override void CreateAbcObject(aeAPI.aeObject parent)
        {
            m_abc = aeAPI.aeNewPoints(parent, gameObject.name);
        }

        public override unsafe void Capture()
        {
            var target = GetComponent<MPWorld>();
            var ctx = target.GetContext();
            var num = MPAPI.mpGetNumParticles(ctx);
            var particles = MPAPI.mpGetParticles(ctx);

            var positions = new Vector3[num];
            var t = GetComponent<Transform>().worldToLocalMatrix;
            for (int i = 0; i< num; ++i)
            {
                positions[i] = t * new Vector4(particles[i].position.x, particles[i].position.y, particles[i].position.z, 1.0f);
            }

            var data = new aeAPI.aePointsSampleData();
            data.count = num;
            data.positions = Marshal.UnsafeAddrOfPinnedArrayElement(positions, 0);
            aeAPI.aePointsWriteSample(m_abc, ref data);
        }

    }
}
