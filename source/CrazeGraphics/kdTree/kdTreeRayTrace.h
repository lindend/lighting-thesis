#pragma once

#include "../../CrazeMath/Intersection/Ray.h"
#include "kdTree.h"

namespace Craze
{
	namespace Graphics2
	{
		float kdTreeRayTrace(const kdTree<>* pKdTree, const Ray& r);
	}
}