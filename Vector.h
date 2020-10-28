#pragma once
class Vector3
{
public:
	float x;
	float y;
	float z;
};
typedef class Vector3 Vectro3;
class Vector4
{
public:
	float x;
	float y;
	float z;
	float w;
};
typedef class Vector4 Vector4;
/// <summary>
/// DotProduct or ScalarProduct a dot b=sum(ai*bi)
/// </summary>
/// <param name="pa">Vector a</param>
/// <param name="pb">Vector b</param>
/// <returns>Scalar value</returns>
inline float  DotProduct3(Vectro3 pa, Vector3 pb)
{
	return pa.x * pb.x + pa.y * pb.y + pa.z * pb.z;
}

inline float  DotProduct4(Vector4 pa, Vector4 pb)
{
	return pa.x * pb.x + pa.y * pb.y + pa.z * pb.z+pa.w*pb.w;
}

inline Vector3 CrossProduct(Vector3 pa, Vector3 pb)
{
	Vector3 xtemp;
	xtemp.x = pa.y * pb.z - pa.z * pb.y;
	xtemp.y = pa.z * pb.x - pa.x * pb.z;
	xtemp.z = pa.x * pb.y - pa.y * pb.x;
	return xtemp;
}
