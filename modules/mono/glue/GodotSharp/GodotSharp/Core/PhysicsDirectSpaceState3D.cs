using System;
using System.Runtime.InteropServices;

#nullable enable

namespace Godot
{
    [Serializable]
    [StructLayout(LayoutKind.Sequential)]
    public struct ShapeRestInfo
    {
        public Vector3 Point;
        public Vector3 Normal;
        public Rid Rid;
        public int ColliderId;
        public int Shape = 0;
        public Vector3 LinearVelocity; // Velocity at contact point.

        public ShapeRestInfo()
        {
        }
    };
}
