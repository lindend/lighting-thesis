#pragma once

#include "Export.h"

namespace Craze
{
	class Vector2;

	/**
	This class corresponds to a 2x2 matrix.
	*/
	class CRAZEMATH_EXP Matrix2
	{
	public:

		/**
		Sets the matrix to a identity matrix (all zero except diagonal numbers which are 1).
		*/
		void SetToIdentity();

		/**
		Multiplies the matrix with another matrix and returns the result.
		@param m The matrix to multiply the current matrix with.
		@return The matrix multiplication result.
		*/
		Matrix2 operator* (const Matrix2& m) const;
		/**
		Gets the row at the specified index. The row is returned as a float
		pointer to to access a certain element in the Matrix just use: Matrix[i][j].
		@param row The row to get a pointer to.
		@return A pointer to the specified row.
		*/
		float* operator[] (unsigned int row) const;
			
	protected:
		/**
		The matrix number array, stored as a 2 dimensional array.
		*/
		float m_Matrix[2][2];
	};

	CRAZEMATH_EXP Vector2 operator* (const Vector2& v, const Matrix2& m);
}