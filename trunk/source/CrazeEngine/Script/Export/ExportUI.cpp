#include "CrazeEngine.h"

#include "luabind/luabind.hpp"
#include "luabind/adopt_policy.hpp"

#include "UIWrappers.h"

using namespace Craze;
using namespace luabind;

void setParent(Base* pSelf, Base* pParent)
{
	pSelf->SetParent(pParent);
}

void craze_open_ui(lua_State* L)
{
	module(L, "ui")
	[
		class_<Base>("Base")
			.def("setParent", &setParent, adopt(_1)),

		class_<Image, Base>("Image")
			.def(constructor<const std::string&, int, int, int, int>())
			.def("setColor", &Image::SetColor),

		class_<Button, Base>("Button")
			.def(constructor<const std::string&, int, int, int, int, const object&>()),

		class_<ImageButton, Base>("ImageButton")
			.def(constructor<const std::string&, int, int, int, int, const object&>()),
		
		class_<Label, Base>("Label")
			.def(constructor<const std::string&, int, int, int, int>())
			.def("setText", &Label::setText)
	];
}