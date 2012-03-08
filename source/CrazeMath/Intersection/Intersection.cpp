#include "Intersection.h"
#include <emmintrin.h>

#include "../CrazeMath.h"

using namespace Craze;

RayTestResult Intersection::Test(const Ray& r, Plane* p)
{
	__m128 minusOne = _mm_set_ps1(-1.0f);

	__m128 epsilon = _mm_set_ps1(0.00001f);
	
    __m128 nx = _mm_setr_ps(p[0].m_Plane->x, p[1].m_Plane->x, p[2].m_Plane->x, p[3].m_Plane->x);
    __m128 ny = _mm_setr_ps(p[0].m_Plane->y, p[1].m_Plane->y, p[2].m_Plane->y, p[3].m_Plane->y);
    __m128 nz = _mm_setr_ps(p[0].m_Plane->z, p[1].m_Plane->z, p[2].m_Plane->z, p[3].m_Plane->z);
					   
    __m128 negd = _mm_setr_ps(p[0].m_Plane->w, p[1].m_Plane->w, p[2].m_Plane->w, p[3].m_Plane->w);
	negd = _mm_sub_ps(_mm_setzero_ps(), negd);

    __m128 rx = _mm_load_ps1(&r.m_Origin->x);
    __m128 ry = _mm_load_ps1(&r.m_Origin->y);
    __m128 rz = _mm_load_ps1(&r.m_Origin->z);
						
    __m128 dx = _mm_load_ps1(&r.m_Dir->x);
    __m128 dy = _mm_load_ps1(&r.m_Dir->y);
    __m128 dz = _mm_load_ps1(&r.m_Dir->z);
	
	//Solve the equation: t = (-d - ray.o dot plane.n) / (ray.d dot plane.n)
	
	//Calculate the dot product in the demoninator, the result is stored in dx
    dx = _mm_mul_ps(dx, nx);
    dy = _mm_mul_ps(dy, ny);
    dz = _mm_mul_ps(dz, nz);

    dx = _mm_add_ps(dx, dy);
    dx = _mm_add_ps(dx, dz);

	//Perform the dot product in the numerator, the result is stored in nx
    nx = _mm_mul_ps(rx, nx);
    ny = _mm_mul_ps(ry, ny);
    nz = _mm_mul_ps(rz, nz);

    nx = _mm_add_ps(nx, ny);
    nx = _mm_add_ps(nx, nz);

	nx = _mm_sub_ps(negd, nx);
	
	//Mask the components which are too small (would create an error if dividing by zero), store this mask in dy
	dy = _mm_mul_ps(dx, dx);
	dy = _mm_cmplt_ps(dy, epsilon);

	//Perform the division
    nx = _mm_div_ps(nx, dx);

	//Use the mask to create the final output
	ny = _mm_and_ps(minusOne, dy);
	nx = _mm_andnot_ps(dy, nx);
	RayTestResult rtr;
	rtr.res = _mm_or_ps(nx, ny);

     return rtr;
}


bool Intersection::IsInside(const Matrix4& m, const BoundingBox& bb, float* pDepth)
{
	SoAV4 pts[2];
	pts[0].xs = _mm_set_ps1(bb.m_Min->x);
	pts[0].ys = _mm_shuffle_ps(bb.m_Min.vec, bb.m_Max.vec, _MM_SHUFFLE(1, 1, 1, 1));//_mm_setr_ps(bb.m_Min.xyz[1], bb.m_Min.xyz[1], bb.m_Max.xyz[1], bb.m_Max.xyz[1]);
	pts[0].zs = _mm_shuffle_ps(bb.m_Min.vec, bb.m_Max.vec, _MM_SHUFFLE(2, 2, 2, 2));
	pts[0].zs = _mm_shuffle_ps(pts[0].zs, pts[0].zs, _MM_SHUFFLE(0, 2, 0, 2));//_mm_setr_ps(bb.m_Min.z, bb.m_Max.z, bb.m_Min.z, bb.m_Max.z);
	pts[0].ws = _mm_set_ps1(1.0f);
	pts[1].xs = _mm_set_ps1(bb.m_Max->x);
	pts[1].ys = pts[0].ys;
	pts[1].zs = pts[0].zs;
	pts[1].ws = pts[0].ws;

	TransformV4(m, pts, 2);
	
	__m128 dmin = _mm_min_ps(pts[0].ws, pts[1].ws);
	*pDepth = Min(Min(dmin.m128_f32[0], dmin.m128_f32[1]), Min(dmin.m128_f32[2], dmin.m128_f32[3]));

	__m128 p0negw = _mm_sub_ps(_mm_setzero_ps(), pts[0].ws);
	__m128 p1negw = _mm_sub_ps(_mm_setzero_ps(), pts[1].ws);

	/*
	x >= -w, x <= w: left/right planes
	y >= -w, y <= w: top/bottom planes
	z >= 0, z <= w: near/far planes
*/
	__m128 outLeft = _mm_and_ps(_mm_cmplt_ps(pts[0].xs, p0negw), _mm_cmplt_ps(pts[1].xs, p1negw));
	__m128 outRight = _mm_and_ps(_mm_cmpgt_ps(pts[0].xs, pts[0].ws), _mm_cmpgt_ps(pts[1].xs, pts[1].ws));
																		 
	__m128 outTop = _mm_and_ps(_mm_cmplt_ps(pts[0].ys, p0negw), _mm_cmplt_ps(pts[1].ys, p1negw));
	__m128 outDown = _mm_and_ps(_mm_cmpgt_ps(pts[0].ys, pts[0].ws), _mm_cmpgt_ps(pts[1].ys, pts[1].ws));
																		 
	__m128 outNear = _mm_and_ps(_mm_cmplt_ps(pts[0].zs, _mm_setzero_ps()), _mm_cmplt_ps(pts[1].zs, _mm_setzero_ps()));
	__m128 outFar = _mm_and_ps(_mm_cmpgt_ps(pts[0].zs, pts[0].ws), _mm_cmpgt_ps(pts[1].zs, pts[1].ws));

	int ol = outLeft.m128_i32[0] & outLeft.m128_i32[1] & outLeft.m128_i32[2] & outLeft.m128_i32[3];
	int or = outRight.m128_i32[0] & outRight.m128_i32[1] & outRight.m128_i32[2] & outRight.m128_i32[3];
	int ot = outTop.m128_i32[0] & outTop.m128_i32[1] & outTop.m128_i32[2] & outTop.m128_i32[3];
	int od = outDown.m128_i32[0] & outDown.m128_i32[1] & outDown.m128_i32[2] & outDown.m128_i32[3];
	int on = outNear.m128_i32[0] & outNear.m128_i32[1] & outNear.m128_i32[2] & outNear.m128_i32[3];
	int of = outFar.m128_i32[0] & outFar.m128_i32[1] & outFar.m128_i32[2] & outFar.m128_i32[3];

	return !(ol | or | ot | od | on | of);
}

//Note that this code is very much mathematically unstable, for example when b.m_Min[K] == r.m_Origin[K] and r.m_Dir[K] == 0 (K denotes dimension - x, y or z)
RayBoxResult Intersection::Test(const Ray& r, const BoundingBox& b)
{
	Vector3 l1 = (b.m_Min - r.m_Origin) / r.m_Dir;
	Vector3 l2 = (b.m_Max - r.m_Origin) / r.m_Dir;

	Vector3 lmin = Vector3::Min(l1, l2);
	Vector3 lmax = Vector3::Max(l1, l2);
	
	RayBoxResult res;
	res.tMin = Max(0.f, Max(lmin.GetX(), Max(lmin.GetY(), lmin.GetZ())));
	res.tMax = Min(lmax.GetX(), Min(lmax.GetY(), lmax.GetZ()));
	res.intersects = (res.tMax >= res.tMin);
	return res;
}
#define INFINITY 1000000000000.f
float Intersection::Test(const Ray& r, const Triangle& t)
{
	Vector3 edge1 = (Vector3)t.v2 - t.v0;
	Vector3 edge0 = (Vector3)t.v1 - t.v0;

	Vector3 p = r.m_Dir.Cross(edge1);

	Vector3 tv = r.m_Origin - t.v0;

	float det = edge0.Dot(p);

	if (det < 0.00001f && det > -0.00001f)
	{
		return INFINITY;
	}

	float invDet = 1.f / det;

	Vector3 q = tv.Cross(edge0);

	float u = tv.Dot(p) * invDet;

	float v = r.m_Dir.Dot(q) * invDet;

	if (u >= 0.f && v >= 0.f && u + v <= 1.f)
	{
		float value = edge1.Dot(q) * invDet;
		if (value <= 0.f)
		{
			return INFINITY;
		}
		return value;
	}

	return INFINITY;
}