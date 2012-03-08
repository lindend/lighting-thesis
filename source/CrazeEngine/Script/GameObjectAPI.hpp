#pragma once

#include "Memory/MemoryManager.h"

extern "C"
{
#include "lua/lua.h"
#include "lua/lauxlib.h"
}

/*
--This is the target for how a game object should be created in lua

function healthComponent(params)
	--type is default script, so it can be omitted
	--name is mandatory though
	return { type="script", name="health", current=params.current or 20, max = params.max or 20 }
end

obj = createGameObject("object", {	transformComponent{x=0, y=0, z=0},
									meshComponent{mesh = "cubemania4_0.crazemesh"},
									healthComponent{current = 10, max = 20}
									})

--then, to use the properties:
obj.health.max = 25
obj.transform.setPos(1, 2, 3)
x, y, z = obj.transform.getPos()

--How should this be achieved?
obj could be stored as a table
maybe I then could use tolua to export all of the component data to script?

*/
namespace Craze
{
	
}