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

function onLoaded()
	level:build()
end

function onMouseDown(pos, button, isDown)
	if isDown == false and button == 0 and 1 == 2 then
		local dir = level.scene.camera:unproject(pos)
		local origin = level.scene.camera.pos
	end
end

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
