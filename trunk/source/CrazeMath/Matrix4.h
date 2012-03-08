#pragma once

#include "Export.h"

#include "mmintrin.h"
#include "xmmintrin.h"

#include <memory>

#pragma warning(push)
#pragma warning(disable: 4201)

namespace Craze
{
	struct CRAZEMATH_EXP Mat4
	{
		float m[4][4];
	};

	class Vector3;
	class Vector4;
	class Quaternion;
	class EulerAngle;

	__declspec(align(16)) class CRAZEMATH_EXP Matrix4 
	{	
	public:
		Matrix4() {}
		Matrix4(float _00, float _01, float _02, float _03, float _10, float _11, float _12, float _13, float _20, float _21, float _22, float _23, float _30, float _31, float _32, float _33);

		void SetToIdentity();
		void Translate(const Vector3& pos);
		void SetTranslation(const Vector3& v);

		void Rotate(const EulerAngle& angle);
		void Scale(float factor);

		Vector3 GetTranslation() const;

		Matrix4 GetInverse() const;
			
		Matrix4 GetTranspose() const;

		float Determinant() const;

		Matrix4 operator * (const Matrix4& m) const;

		float* operator[](unsigned int row);
		const float* operator[](unsigned int row) const;

		static const Matrix4 IDENTITY;

		static Matrix4 CreateScale(float x, float y, float z);
		static Matrix4 CreateTranslation(float x, float y, float z);
		static Matrix4 CreateTranslation(const Vector3& v);
		static Matrix4 CreateRotation(const Quaternion& q);

		static Matrix4 CreateFromRightUpForward(const Vector3& right, const Vector3& up, const Vector3& forward);

		static Matrix4 CreatePerspectiveFov(float fovY, float aspect, float znear, float zfar);
		static Matrix4 CreateOrtho(float width, float height, float znear, float zfar);
		static Matrix4 CreateView(const Vector3& eyePosition, const Vector3& lookAt, const Vector3& up);

		operator Mat4() const { Mat4 r; memcpy(r.m[0], m_Matrix[0], sizeof(float) * 16); return r; }

		union
		{
			float m_Matrix[4][4];
			struct
			{
				__m128 m0;
				__m128 m1;
				__m128 m2;
				__m128 m3;
			};
		};
		
	};

	/*
	Multiplies the matrix with the vector. The last component (w)
	of the Vector is 1.0f.
	*/
	CRAZEMATH_EXP Vector3 operator*(const Vector3& vec, const Matrix4& mat);
	CRAZEMATH_EXP Vector4 operator*(const Vector4& v, const Matrix4& m);

	_declspec(align(16)) struct SoAV4
	{
		__m128 xs;
		__m128 ys;
		__m128 zs;
		__m128 ws;
	};

	_declspec(align(16)) struct SoAV3
	{
		__m128 xs;
		__m128 ys;
		__m128 zs;
	};

	CRAZEMATH_EXP void Transform(const Matrix4& m, Vector3* vs, unsigned int num);
	//This function assumes that all inputs are points and thus have their implicit w set to 1
	CRAZEMATH_EXP void Transform(const Matrix4& m, SoAV3* vs, unsigned int num);
	//This function assumes that all inputs are vectors and have their implicit w as 0
	CRAZEMATH_EXP void TransformVecs(const Matrix4& m, SoAV3* vs, unsigned int num);

	Vector3 TransformVec(const Matrix4& m, const Vector3& v);

	CRAZEMATH_EXP void TransformV4(const Matrix4& m, Vector4* vs, unsigned int num);
	CRAZEMATH_EXP void TransformV4(const Matrix4& m, SoAV4* vs, unsigned int num);
}

#pragma warning(pop)