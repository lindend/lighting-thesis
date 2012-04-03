level = game.newLevel()
game.setLevel(level)
game.setCameraController(graphics.FreeFlyCamera())

level.scene.camera:setProjection(3.1415 / 4, 4 / 3, 10, 10000)

--Load in a level, set a couple of callbacks. This code could preferably be put in a lua library for the game engine
counter = 0
function Static(pos, mesh)
	print("Adding static " .. mesh .. " at " .. pos.x .. " " .. pos.y .. " " .. pos.z)
	counter = counter + 1
	return level:add("static" .. counter, {	component.transform(pos),
											component.mesh{file = mesh}})
end
function Object(name, params)
	print("Adding object " .. name)
	return level:add(name, params)
end

game.loadFile("level.lua")

function waitFor(condition, onTrue)
	return game.beginUpdate(function(_)
								if condition() then
									onTrue()
									return false
								end
								return true
							end)
end

function fader(duration, start, stop, setter)
	local p = 0.0
	return game.beginUpdate(function(delta)
								p = p + delta / duration
								if p >= 1.0 then
									setter(stop)
									return false
								end
								setter(p * stop + (1.0 - p) * start)
								return true
							end)
end

function setTimer(funct, timeout)
	return game.beginUpdate(function(delta)
								timeout = timeout - delta
								if timeout <= 0 then
									funct()
									return false
								end
								return true
							end)
end

function stopTimer(tmr)
	game.stopUpdate(tmr)
end

function v3str(v)
	return "( " .. v.x .. " , " .. v.y .. " , " .. v.z .. " )"
end

UP = Vector3{x=0,y=1,z=0}

function followPath(obj, path, speed)
	local currentTarget = 2
	return game.beginUpdate(function(delta)
								local currentPos = obj.transform.pos
								local dVec = path:getNode(currentTarget) - currentPos
								local dir = math.normalize(dVec)
								local travelLen = delta * speed
								local leftLen = math.length(dVec)
								obj.transform:setOrientation(dir, UP)
								if travelLen > leftLen then
									obj.transform.pos = dir * leftLen + currentPos
									currentTarget = currentTarget + 1
									print("New target! : " .. currentTarget)
									if currentTarget > path.numNodes then
										return false
									end
								else
									local newPos = dir * travelLen + currentPos
									obj.transform.pos = newPos
								end
								
								return true
							end)
end

function showDialog(str)
	local lblBg = ui.Image("Library/UI/dialogbg.png", 100, 100, 300, 100)
	local lbl = ui.Label(str, 10, 10, 100, 50)
	lbl:setParent(lblBg)
	setTimer(function() lblBg = nil end, 3)
end

function onLoaded()
	level:build()
end

follower = nil

function onMouseDown(pos, button, isDown)
	if isDown == false and button == 0 and 1 == 2 then
		local dir = level.scene.camera:unproject(pos)
		local origin = level.scene.camera.pos
		
		local obj,hit = level:rayCast(origin, dir, 100)
		
		if obj then
			if obj.onClick then
				obj.onClick.trigger()
			else
				local begin = cubeman.transform.pos
				if follower then
					game.stopUpdate(follower)
				end
				local p = level:findPath(begin, hit)
				follower = followPath(cubeman, p, 1.5)
			end
		end
	end
end
--[[
exitGame = ui.Button("Exit game", 100, 350, 100, 30, function() game.shutDown() end)

ui_x = 20
ui_y = 0
ui_w = 50
ui_h = 15

s = {"display gui", " ", "show indirect illumination" , "show simplified geo", "build frequency", "RSM size", "number of rays", "bounces per ray", " ", "ray trace time", "final gather time", "total time"}
lbls = {}

for i = 1, 12, 1 do
	lbls[i] =  ui.Label(s[i], ui_x, ui_y+(ui_h*i), ui_w, ui_h)
end

btn1 =  ui.Button("ON", ui_x + 150, ui_y+(ui_h*1), ui_w, ui_h, function() game.shutdown() end)
btn2 = ui.Button("ON", ui_x + 150, ui_y+(ui_h*3), ui_w, ui_h, function() game.shutdown() end)
btn3 = ui.Button("ON", ui_x + 150, ui_y+(ui_h*4), ui_w, ui_h, function() game.shutdown() end)

]]--
print("Pending: " .. resource.getNumPending())
waitFor(function() return resource.getNumPending() == 0 end, onLoaded)

btnListener = event.mouseButton.Listener(onMouseDown)
listener = event.mouseMove.Listener(function (pos) 
										dir = level.scene.camera:unproject(pos)
										origin = level.scene.camera.pos
										
										obj,hit = level:rayCast(origin, dir, 100)
										if obj then
											marker.transform.pos = hit
										end
									end)

showUI = true;
useDirect = true
useIndirect = true
drawRays = false

if showUI then
	toggleDirectLabel = ui.Label("K - toggle direct " .. toString(useDirect), 20, 20, 50, 15)
	toggleIndirectLabel = ui.Label("L - toggle indirect " .. toString(useIndirect), 20, 35, 50, 15)
	toggleRaysLabel = ui.Label("O - toggle rays " .. toString(drawRays), 20, 50, 50, 15)

	startDirectionalLightTravel = ui.Label("T - start directional light travel", 20, 80, 50, 15)
	stopDirectionalLightTravel = ui.Label("Y - stop directional light travel", 20, 95, 50, 15)
	resetDirectionalLightLabel = ui.Label("R - reset directional light", 20, 110, 50, 15)
end


useShadows = true
keyActions = { 
		[108] = function() useIndirect = not useIndirect; graphics.useIndirectLighting(useIndirect) end,
		[107] = function() useDirect = not useDirect; graphics.useDirectLighting(useDirect) end,
		[111] = function() drawRays = not drawRays; graphics.drawRays(drawRays) end,
		[105] = function() useShadows = not useShadows; graphics.useShadows(useShadows) end
	 }
function onKey(kc, ks)
	if ks == 0 then
		if keyActions[kc] ~= nil then
			keyActions[kc]()
		end
	end
end
keyListener = event.keyboard.Listener(onKey)



--event.mouseMove.stopListen(listener)
--[[
bouncer = level:add("bouncer", {component.transform{x=-3,y=4.3,z=0.8}, 
								component.mesh{file="Library/Models/cube_0.crazemesh"},
								{type="script", name="test", update=function(delta) print("Pos: " .. bouncer.transform:getPos().x) end}})]]--

--This builds the level. Building a level means that (at least) the following is calculated:
--Navigation mesh
--kd-trees for lighting
--possibly sky occlusion, but it would best to compute it in the editor
--If more static objects are added later, level:rebuild CAN be called. Preferably it should not as it's a very expensive operation.
--level:build()

--obj = level:add("object", {	component.transform{x=10, y=10, z=10},
--							component.mesh{file = "Library/cubemania4_0.crazemesh"} })



--Event listeners and such...
--Lua: listner = event.mouseClick.listen(function(pos, button, isPressed) print("Mouse button pressed at " .. pos.x .. " " .. pos.y) end)
--listener:close()

--C++: def("
--