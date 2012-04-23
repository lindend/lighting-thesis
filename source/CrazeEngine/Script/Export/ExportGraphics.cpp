#include "CrazeEngine.h"

#include "luabind/luabind.hpp"
#include "luabind/tag_function.hpp"
#include "luabind/adopt_policy.hpp"
#include "luabind/iterator_policy.hpp"

#include "Scene/Camera.h"
#include "Scene/Scene.h"
#include "CameraController/FreeFlyCamera.h"
#include "Device.h"
#include "Graphics.h"
#include "GPUProfiler/GPUProfiler.h"

using namespace luabind;
using namespace Craze;
using namespace Craze::Graphics2;

void setCamPos(Camera* pCam, const Vec3& pos)
{
	pCam->SetPosition(pos);
}
Vec3 getCamPos(Camera* pCam)
{
	return pCam->GetPosition();
}

void setCamDir(Camera* cam, const Vec3& dir)
{
    cam->SetDirection(dir);
}
Vec3 getCamDir(Camera* cam)
{
    return cam->GetDirection();
}

Vec3 camUnproject(Camera* pCam, const Vector2& v)
{
	Vector2 res = gpDevice->GetViewPort();
	Vector2 pos = Vector2(v.x / res.x, 1.f - v.y / res.y) * 2.f - 1.f;

	return pCam->GetPointerDirection(pos);
}

void useIndirect(bool v)
{
	gUseIndirectLighting = v;
}

void useDirect(bool v)
{
	gUseDirectLighting = v;
}
void drawRays(bool v)
{
	gDrawRays = v;
}
void useShadows(bool v)
{
	gUseShadows = v;
}
void takeScreenshot(const std::string& path)
{
	gSaveScreenShot = true;
	gScreenShotPath = path;
}

const std::vector<TimingBlock>& getProfile()
{
	static auto empty = std::vector<TimingBlock>();
	auto timings = gpGraphics->m_profiler->getTimings();
	if (timings)
	{
		return *timings;
	} else
	{
		return empty;
	}
}

void craze_open_graphics(lua_State* L)
{
	luabind::
	module(L, "graphics")
	[
		class_<Scene, Scene*>("Scene")
			.def_readonly("camera", &Scene::getCamera),

		class_<TimingBlock>("TimingBlock")
			.def_readonly("name", &TimingBlock::name)
			.def_readonly("level", &TimingBlock::level)
			.def_readonly("time", &TimingBlock::time),
			
		class_<Camera, Camera*>("Camera")
			.def("setProjection", &Camera::SetProjection)
			.def("unproject", &camUnproject)
			.property("pos", &getCamPos, &setCamPos)
            .property("direction", &getCamDir, &setCamDir),
			//.property("pos", tag_function<Vec3()>(&Camera::GetPosition), tag_function<void(const Vec3&)>(&Camera::SetPosition))
			//.property("lookAt", &Camera::GetDirection, &Camera::SetDirection)
			//.property("up", &Camera::GetUp, &Camera::SetUp),

		class_<ICameraController>("CameraController"),
		class_<FreeFlyCamera, ICameraController*>("FreeFlyCamera")
			.def(constructor<>())
			.property("speed", &FreeFlyCamera::GetSpeed, &FreeFlyCamera::SetSpeed),
		def("useIndirectLighting", &useIndirect),
		def("useDirectLighting", &useDirect),
		def("drawRays", &drawRays),
		def("useShadows", &useShadows),
		def("captureScreenShot", &takeScreenshot),
		def("getTimings", &getProfile, return_stl_iterator)
	];
}