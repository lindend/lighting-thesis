#pragma once

#include "Export.h"

namespace Craze
{
	class Vector3;

	class CRAZEMATH_EXP Quaternion
	{
	public:
		float x, y, z, w;

		void Normalize();
		Quaternion Normalized() const;
		Vector3 Transform(const Vector3& v) const;
		Quaternion Conjugate() const;
		float GetLength() const;
		float GetRLength() const;
		float GetSquaredLength() const;

		float Real() const;
		Vector3 Im() const;

		/**
		Initializes an identity quaternion
		*/
		Quaternion();
		Quaternion(float _x, float _y, float _z, float _w);
		Quaternion(float yaw, float pitch, float roll);
		Quaternion(const Vector3& axis, float angle);
		Quaternion(const Vector3& dirFrom, const Vector3& dirTo);
		Quaternion operator*(const Quaternion& o) const;
		Quaternion operator/(float f) const;
		Quaternion operator*(float f) const;

		static const Quaternion IDENTITY;
			
	};

}
