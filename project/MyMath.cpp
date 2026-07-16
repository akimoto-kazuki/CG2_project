#include "MyMath.h"
#include <cmath>
#define _USE_MATH_DEFINES
#include <algorithm>

#include <numbers>

namespace MyMath
{
	Vector3 Add(const Vector3& v1, const Vector3& v2)
	{
		Vector3 result;
		result.x = v1.x + v2.x;
		result.y = v1.y + v2.y;
		result.z = v1.z + v2.z;
		return result;
	}

	Matrix4x4 MakeIdentity4x4()
	{
		Matrix4x4 identity;
		identity.m[0][0] = 1.0f;	identity.m[0][1] = 0.0f;	identity.m[0][2] = 0.0f;	identity.m[0][3] = 0.0f;
		identity.m[1][0] = 0.0f;	identity.m[1][1] = 1.0f;	identity.m[1][2] = 0.0f;	identity.m[1][3] = 0.0f;
		identity.m[2][0] = 0.0f;	identity.m[2][1] = 0.0f;	identity.m[2][2] = 1.0f;	identity.m[2][3] = 0.0f;
		identity.m[3][0] = 0.0f;	identity.m[3][1] = 0.0f;	identity.m[3][2] = 0.0f;	identity.m[3][3] = 1.0f;
		return identity;
	}

	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2)
	{
		Matrix4x4 result;

		result.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0];
		result.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1];
		result.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2];
		result.m[0][3] = m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3];

		result.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0];
		result.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1];
		result.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2];
		result.m[1][3] = m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3];

		result.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0];
		result.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1];
		result.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2];
		result.m[2][3] = m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3];

		result.m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0];
		result.m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1];
		result.m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2];
		result.m[3][3] = m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3];

		return result;

	}

	Vector3 MultiplyVector3(float scalar, const Vector3& v)
	{
		Vector3 result;
		result.x = scalar * v.x;
		result.y = scalar * v.y;
		result.z = scalar * v.z;
		return result;
	}

	Matrix4x4 MakeRotateXMatrix(float radian)
	{
		Matrix4x4 result;

		result.m[0][0] = 1.0f; result.m[0][1] = 0.0f; result.m[0][2] = 0.0f; result.m[0][3] = 0.0f;
		result.m[1][0] = 0.0f; result.m[1][1] = std::cos(radian); result.m[1][2] = std::sin(radian); result.m[1][3] = 0.0f;
		result.m[2][0] = 0.0f; result.m[2][1] = -std::sin(radian); result.m[2][2] = std::cos(radian); result.m[2][3] = 0.0f;
		result.m[3][0] = 0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;

		return result;
	}

	Matrix4x4 MakeRotateYMatrix(float radian)
	{
		Matrix4x4 result;

		result.m[0][0] = std::cos(radian); result.m[0][1] = 0.0f; result.m[0][2] = -std::sin(radian); result.m[0][3] = 0.0f;
		result.m[1][0] = 0.0f; result.m[1][1] = 1.0f; result.m[1][2] = 0.0f; result.m[1][3] = 0.0f;
		result.m[2][0] = std::sin(radian); result.m[2][1] = 0.0f; result.m[2][2] = std::cos(radian); result.m[2][3] = 0.0f;
		result.m[3][0] = 0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;

		return result;
	}

	Matrix4x4 MakeRotateZMatrix(float radian)
	{
		Matrix4x4 result;

		result.m[0][0] = std::cos(radian); result.m[0][1] = std::sin(radian); result.m[0][2] = 0.0f; result.m[0][3] = 0.0f;
		result.m[1][0] = -std::sin(radian); result.m[1][1] = std::cos(radian); result.m[1][2] = 0.0f; result.m[1][3] = 0.0f;
		result.m[2][0] = 0.0f; result.m[2][1] = 0.0f; result.m[2][2] = 1.0f; result.m[2][3] = 0.0f;
		result.m[3][0] = 0.0f; result.m[3][1] = 0.0f; result.m[3][2] = 0.0f; result.m[3][3] = 1.0f;

		return result;
	}

	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate)
	{
		Matrix4x4 result;

		Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
		Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
		Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
		Matrix4x4 XYZ = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));

		result.m[0][0] = scale.x * XYZ.m[0][0]; result.m[0][1] = scale.x * XYZ.m[0][1]; result.m[0][2] = scale.x * XYZ.m[0][2]; result.m[0][3] = 0.0f;
		result.m[1][0] = scale.y * XYZ.m[1][0]; result.m[1][1] = scale.y * XYZ.m[1][1]; result.m[1][2] = scale.y * XYZ.m[1][2]; result.m[1][3] = 0.0f;
		result.m[2][0] = scale.z * XYZ.m[2][0]; result.m[2][1] = scale.z * XYZ.m[2][1]; result.m[2][2] = scale.z * XYZ.m[2][2]; result.m[2][3] = 0.0f;
		result.m[3][0] = translate.x; result.m[3][1] = translate.y; result.m[3][2] = translate.z; result.m[3][3] = 1.0f;

		return result;
	}

	Matrix4x4 MakeAffineMatrixQuaternion(const Vector3& scale, const Quaternion& rotate, const Vector3& translate)
	{
		Matrix4x4 result;

		// クォータニオンから回転行列を計算するための要素を準備
		float xx = rotate.x * rotate.x;
		float yy = rotate.y * rotate.y;
		float zz = rotate.z * rotate.z;
		float xy = rotate.x * rotate.y;
		float xz = rotate.x * rotate.z;
		float yz = rotate.y * rotate.z;
		float wx = rotate.w * rotate.x;
		float wy = rotate.w * rotate.y;
		float wz = rotate.w * rotate.z;

		// 回転行列の各要素を計算しつつ、スケール（scale）を掛け算する
		// （既存の MakeAffineMatrix と同じように 1行目にscale.x、2行目にscale.y、3行目にscale.zを掛けています）
		result.m[0][0] = scale.x * (1.0f - 2.0f * (yy + zz));
		result.m[0][1] = scale.x * (2.0f * (xy + wz));
		result.m[0][2] = scale.x * (2.0f * (xz - wy));
		result.m[0][3] = 0.0f;

		result.m[1][0] = scale.y * (2.0f * (xy - wz));
		result.m[1][1] = scale.y * (1.0f - 2.0f * (xx + zz));
		result.m[1][2] = scale.y * (2.0f * (yz + wx));
		result.m[1][3] = 0.0f;

		result.m[2][0] = scale.z * (2.0f * (xz + wy));
		result.m[2][1] = scale.z * (2.0f * (yz - wx));
		result.m[2][2] = scale.z * (1.0f - 2.0f * (xx + yy));
		result.m[2][3] = 0.0f;

		// 平行移動（translate）の設定（既存の MakeAffineMatrix と全く同じです）
		result.m[3][0] = translate.x;
		result.m[3][1] = translate.y;
		result.m[3][2] = translate.z;
		result.m[3][3] = 1.0f;

		return result;
	}

	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip)
	{
		Matrix4x4 result;


		result.m[0][0] = 2 / (right - left); result.m[0][1] = 0.0f; result.m[0][2] = 0.0f; result.m[0][3] = 0.0f;
		result.m[1][0] = 0.0f; result.m[1][1] = 2 / (top - bottom); result.m[1][2] = 0.0f; result.m[1][3] = 0.0f;
		result.m[2][0] = 0.0f; result.m[2][1] = 0.0f; result.m[2][2] = 1 / (farClip - nearClip); result.m[2][3] = 0.0f;
		result.m[3][0] = (left + right) / (left - right); result.m[3][1] = (top + bottom) / (bottom - top); result.m[3][2] = nearClip / (nearClip - farClip); result.m[3][3] = 1.0f;
		return result;
	}

	Matrix4x4 Inverse(const Matrix4x4& m)
	{
		float determinant = +m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3]
			+ m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1]
			+ m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]

			- m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1]
			- m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3]
			- m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]

			- m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3]
			- m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1]
			- m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]

			+ m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1]
			+ m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3]
			+ m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]

			+ m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3]
			+ m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1]
			+ m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]

			- m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1]
			- m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3]
			- m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]

			- m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0]
			- m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0]
			- m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]

			+ m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0]
			+ m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0]
			+ m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];

		Matrix4x4 result;
		float recpDeterminant = 1.0f / determinant;
		result.m[0][0] = (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] +
			m.m[1][3] * m.m[2][1] * m.m[3][2] - m.m[1][3] * m.m[2][2] * m.m[3][1] -
			m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2]) * recpDeterminant;
		result.m[0][1] = (-m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[2][3] * m.m[3][1] -
			m.m[0][3] * m.m[2][1] * m.m[3][2] + m.m[0][3] * m.m[2][2] * m.m[3][1] +
			m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2]) * recpDeterminant;
		result.m[0][2] = (m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] +
			m.m[0][3] * m.m[1][1] * m.m[3][2] - m.m[0][3] * m.m[1][2] * m.m[3][1] -
			m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2]) * recpDeterminant;
		result.m[0][3] = (-m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] -
			m.m[0][3] * m.m[1][1] * m.m[2][2] + m.m[0][3] * m.m[1][2] * m.m[2][1] +
			m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2]) * recpDeterminant;

		result.m[1][0] = (-m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] -
			m.m[1][3] * m.m[2][0] * m.m[3][2] + m.m[1][3] * m.m[2][2] * m.m[3][0] +
			m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2]) * recpDeterminant;
		result.m[1][1] = (m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] +
			m.m[0][3] * m.m[2][0] * m.m[3][2] - m.m[0][3] * m.m[2][2] * m.m[3][0] -
			m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2]) * recpDeterminant;
		result.m[1][2] = (-m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] -
			m.m[0][3] * m.m[1][0] * m.m[3][2] + m.m[0][3] * m.m[1][2] * m.m[3][0] +
			m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2]) * recpDeterminant;
		result.m[1][3] = (m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] +
			m.m[0][3] * m.m[1][0] * m.m[2][2] - m.m[0][3] * m.m[1][2] * m.m[2][0] -
			m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2]) * recpDeterminant;

		result.m[2][0] = (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] +
			m.m[1][3] * m.m[2][0] * m.m[3][1] - m.m[1][3] * m.m[2][1] * m.m[3][0] -
			m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]) * recpDeterminant;
		result.m[2][1] = (-m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] -
			m.m[0][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[2][1] * m.m[3][0] +
			m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1]) * recpDeterminant;
		result.m[2][2] = (m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] +
			m.m[0][3] * m.m[1][0] * m.m[3][1] - m.m[0][3] * m.m[1][1] * m.m[3][0] -
			m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]) * recpDeterminant;
		result.m[2][3] = (-m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] -
			m.m[0][3] * m.m[1][0] * m.m[2][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] +
			m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1]) * recpDeterminant;

		result.m[3][0] = (-m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] -
			m.m[1][2] * m.m[2][0] * m.m[3][1] + m.m[1][2] * m.m[2][1] * m.m[3][0] +
			m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1]) * recpDeterminant;
		result.m[3][1] = (m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] +
			m.m[0][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[2][1] * m.m[3][0] -
			m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]) * recpDeterminant;
		result.m[3][2] = (-m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][2] * m.m[3][0] -
			m.m[0][2] * m.m[1][0] * m.m[3][1] + m.m[0][2] * m.m[1][1] * m.m[3][0] +
			m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1]) * recpDeterminant;
		result.m[3][3] = (m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] +
			m.m[0][2] * m.m[1][0] * m.m[2][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] -
			m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]) * recpDeterminant;

		return result;
	}

	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip)
	{
		float cotHalfFovV = 1.0f / std::tan(fovY / 2.0f);
		return {
			(cotHalfFovV / aspectRatio), 0.0f, 0.0f, 0.0f,
			0.0f, cotHalfFovV, 0.0f, 0.0f,
			0.0f, 0.0f, farClip / (farClip - nearClip), 1.0f,
			0.0f, 0.0f, -(nearClip * farClip) / (farClip - nearClip), 0.0f
		};
	}

	Matrix4x4 MakeTranslateMatrix(const Vector3& translate)
	{
		Matrix4x4 result;
		// 1行目
		result.m[0][0] = 1.0f;  result.m[0][1] = 0.0f;  result.m[0][2] = 0.0f;  result.m[0][3] = 0.0f;
		// 2行目
		result.m[1][0] = 0.0f;  result.m[1][1] = 1.0f;  result.m[1][2] = 0.0f;  result.m[1][3] = 0.0f;
		// 3行目
		result.m[2][0] = 0.0f;  result.m[2][1] = 0.0f;  result.m[2][2] = 1.0f;  result.m[2][3] = 0.0f;
		// 4行目（ここに移動量 X, Y, Z が入ります）
		result.m[3][0] = translate.x;
		result.m[3][1] = translate.y;
		result.m[3][2] = translate.z;
		result.m[3][3] = 1.0f;

		return result;
	}

	Matrix4x4 MakeScaleMatrix(const Vector3& scale)
	{
		Matrix4x4 result;
		result.m[0][0] = scale.x; result.m[0][1] = 0.0f;    result.m[0][2] = 0.0f;    result.m[0][3] = 0.0f;
		result.m[1][0] = 0.0f;    result.m[1][1] = scale.y; result.m[1][2] = 0.0f;    result.m[1][3] = 0.0f;
		result.m[2][0] = 0.0f;    result.m[2][1] = 0.0f;    result.m[2][2] = scale.z; result.m[2][3] = 0.0f;
		result.m[3][0] = 0.0f;    result.m[3][1] = 0.0f;    result.m[3][2] = 0.0f;    result.m[3][3] = 1.0f;
		return result;
	}

	float Length(const Vector3& v)
	{
		float result;
		result = static_cast<float>(sqrt(v.x * v.x + v.y * v.y + v.z * v.z));
		return result;
	}

	Vector3 Normalize(const Vector3& v)
	{
		Vector3 result;
		result.x = v.x / static_cast<float>(sqrt(v.x * v.x + v.y * v.y + v.z * v.z));
		result.y = v.y / static_cast<float>(sqrt(v.x * v.x + v.y * v.y + v.z * v.z));
		result.z = v.z / static_cast<float>(sqrt(v.x * v.x + v.y * v.y + v.z * v.z));
		return result;
	}

	float Dot(const Quaternion& q1, const Quaternion& q2)
	{
		float result;
		result = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
		return result;
	}

	Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t)
	{
		return Add(MultiplyVector3(t, v1), MultiplyVector3((1.0f - t), v2));
	}

	Quaternion Slerp(const Quaternion& q0, const Quaternion& q1, float t)
	{
		Quaternion resalt;
		float dot = Dot(q0, q1);
		Quaternion q2 = q0;
		if (dot < 0)
		{
			q2.x = -q2.x;
			q2.y = -q2.y;
			q2.z = -q2.z;
			q2.w = -q2.w;
			dot = -dot;
		}

		float theta = std::acos(dot);

		float scale0 = sinf((1 - t) * theta) / sinf(theta);
		float scale1 = sinf(t * theta) / sinf(theta);

		resalt.x = scale0 * q2.x + scale1 * q1.x;
		resalt.y = scale0 * q2.y + scale1 * q1.y;
		resalt.z = scale0 * q2.z + scale1 * q1.z;
		resalt.w = scale0 * q2.w + scale1 * q1.w;

		return resalt;

	}

}