#include "Random.h"
#include "../../CrazeMath/Vector3.h"
#include "MersenneTwister.h"

#include <assert.h>

MTRand gRandom;

using namespace Craze;
using namespace Craze::Random;

float Craze::Random::GetFloat(float min, float max)
{
	assert(max > min);
	return (float)gRandom.rand() * (max - min) + min;
}

Vector3 Craze::Random::GetVector3(float min, float max)
{
	assert(max > min);
	return Vector3(GetFloat(min, max), GetFloat(min, max), GetFloat(min, max));
}