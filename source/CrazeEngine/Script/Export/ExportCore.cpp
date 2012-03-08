#include "CrazeEngine.h"
#include <iostream>

#include "luabind/luabind.hpp"
#include "luabind/operator.hpp"
#include "luabind/adopt_policy.hpp"

#include "V3Math.h"
#include "Resource/FileDataLoader.h"
#include "Resource/ResourceManager.h"

using namespace Craze;
using namespace luabind;

void print(const object& p)
{
	switch (type(p))
	{
	case LUA_TSTRING:
		std::cout << luabind::object_cast<const char*>(p);
		break;
	case LUA_TNUMBER:
		std::cout << luabind::object_cast<float>(p);
		break;
	case LUA_TBOOLEAN:
		std::cout << luabind::object_cast<bool>(p) ? "true" : "false";
		break;
	};
	std::cout << std::endl;
}

void addFileLocation(const char* pLocation)
{
	gFileDataLoader.addLocation(pLocation);
}

void addLoader(const std::string &name)
{
	if (name == "file")
	{
		gResMgr.addDataLoader(&gFileDataLoader);
	}
}

int getNumPending()
{
	return gResMgr.getNumPending();
}

Vec3 makeV3(const object& obj)
{
	Vec3 v = {	object_cast<float>(obj["x"]),
				object_cast<float>(obj["y"]),
				object_cast<float>(obj["z"]) };
	return v;
}

namespace Craze
{
	Vec3 operator+(const Vec3& v0, const Vec3& v1)
	{
		Vec3 v = { v0.x + v1.x, v0.y + v1.y, v0.z + v1.z };
		return v;
	}

	Vec3 operator-(const Vec3& v0, const Vec3& v1)
	{
		Vec3 v = { v0.x - v1.x, v0.y - v1.y, v0.z - v1.z };
		return v;
	}
	Vec3 operator*(const Vec3& v0, float f)
	{
		Vec3 v = { v0.x * f, v0.y * f, v0.z * f };
		return v;
	}
	Vec3 operator*(float f, const Vec3& v0)
	{
		Vec3 v = { v0.x * f, v0.y * f, v0.z * f };
		return v;
	}
	std::ostream& operator<<(std::ostream& stream, Vec3 v)
	{
		return (stream << "( " << v.x << " , " << v.y << " , " << v.z << " )");
	}
}

Vec3 normalize(Vec3& v)
{
	return Normalize(Vector3(v));
}
float length(Vec3& v)
{
	return Length(Vector3(v));
}

void craze_open_core(lua_State* L)
{
	module(L, "resource")
	[
		def("addFileLocation", &addFileLocation),
		def("addLoader", &addLoader),
		def("getNumPending", &getNumPending)
	];

	module(L, "math")
	[
		def("sqrt", &Sqrt),
		def("rsqrt", &RSqrt),
		def("sin", &Sin),
		def("cos", &Cos),
		def("tan", &Tan),
		def("asin", &Asin),
		def("acos", &Acos),
		def("atan", &Atan),
		def("lerp", &Lerp),
		def("min", &Min<float>),
		def("max", &Max<float>),

		def("normalize", &normalize),
		def("length", &length),

		class_<Vec3>("Vector3")
			.def_readwrite("x", &Vec3::x)
			.def_readwrite("y", &Vec3::y)
			.def_readwrite("z", &Vec3::z)
			.def(const_self - other<const Vec3&>())
			.def(const_self + other<const Vec3&>())
			.def(const_self * other<float>())
			.def(other<float>() * const_self)
			.def(tostring(self)),

		class_<Vector2>("Vector2")
			.def_readwrite("x", &Vector2::x)
			.def_readwrite("y", &Vector2::y)
	];

	module(L)
	[
		def("print", &print),
		def("Vector3", &makeV3)
	];
}
