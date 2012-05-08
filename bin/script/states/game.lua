level = game.newLevel()
game.setLevel(level)
game.setCameraController(graphics.FreeFlyCamera())

level.scene.camera:setProjection(3.1415 / 4, 1920/1080, 10, 10000)

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
function v3(x, y, z)
	return Vector3{x=x, y=y, z=z}
end

cmPath = { v3(1000, 70, 0), v3(-1000, 70, 0) }
cmPath.numNodes = 2
function cmPath:getNode(node)
	return cmPath[node]
end

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
										currentTarget = 1
									end
								else
									local newPos = dir * travelLen + currentPos
									obj.transform.pos = newPos
								end
								
								return true
							end)
end

cubeman = nil
dirlight = nil
spotlight = nil
lightAnimator = nil

function onLoaded()
	dirlight = level.scene:addDirectionalLight(v3(0.2, -1, 0.2), v3(1, 1, 1))
	spotlight = level.scene:addSpotLight(v3(-258, 132, 518), v3(-1, 0, 0), 0.3, 100000, v3(1, 1, 1))
	lightAnimator = game.beginUpdate(animLights)

	cubeman = level:add("cubeman", {component.transform{x=200, y=70, z=0},
									component.mesh{file="cubeman.crm"}})
	level:build()
	--followPath(cubeman, cmPath, 100)
end

rx = 0.5
rz = 0.2
rspot = 0
function animLights(delta)
	--dirlight.direction = math.normalize(v3(math.sin(rx) * 0.3, -1, math.sin(rz) * 0.3))
	spotlight.direction = math.normalize(v3(math.sin(rspot), 0, math.cos(rspot)))
	rx = rx + 0.943247785 * delta
	rz = rz + 0.549334545 * delta
	rspot = rspot + 0.37 * delta
	return true
end

function onMouseDown(pos, button, isDown)
	if isDown == false and button == 0 and 1 == 2 then
		local dir = level.scene.camera:unproject(pos)
		local origin = level.scene.camera.pos
	end
end

function round(num, idp)
  local mult = 10^(idp or 0)
  return math.floor(num * mult + 0.5) / mult
end

game.loadFile("cameraValues.lua")
currentPosition = 1

tmr = {}

function startReferenceGrab()
	--spotlight:remove()
	game.stopUpdate(lightAnimator)
	dirlight.direction = math.normalize(v3(60, -325, 87))
	if currentPosition > #benchmarkCameraData then
		return
	end

	level.scene.camera.pos = Vector3(benchmarkCameraData[currentPosition].pos)
	level.scene.camera.direction = Vector3(benchmarkCameraData[currentPosition].dir)

	tmr = setTimer(takeScreenshot, 10)
end

function takeScreenshot()
	print("Capturing screenshot at " .. currentPosition)
	graphics.captureScreenShot("screenshot" .. currentPosition .. ".png")
	currentPosition = currentPosition + 1
	
	tmr = setTimer(startReferenceGrab, 0.5)
end


fpsLabel = ui.Label("FPS: ", 580, 10, 60, 30)
fpsUpd = game.beginUpdate(	function(delta)
								fpsLabel:setText(string.format("FPS: %.1f", game.getFps()))
								return true
							end)

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

showUI = true
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
		[112] = function() currentPosition = 1; startReferenceGrab() end,
		[113] = function() graphics.captureScreenShot("screenshot.png") end,
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


--[[
##############################################
			GPU PROFILING GUI
#############################################
]]--

profGUIWnd = ui.Window("GPU timings", 10, 200, 200, 250)

profGUI = {}
function updateProfGUI(delta)
	timings = graphics.getTimings()
	y = 5
	for b in timings do
		lbl = profGUI[b.name]
		if lbl == nil then
			lbl =  ui.Label("", 10 + b.level * 10, y, 500, 20)
			lbl:setParent(profGUIWnd)
			profGUI[b.name] = lbl
		end

		lbl:setText(string.format("%s - %.2f ms", b.name, b.time))

		y = y + 20
	end
	return true
end

guiUpd = game.beginUpdate(updateProfGUI)


--[[
############################################
			DEBUG POSITION TOOL
############################################
]]--

storedPositions = {}

function addPosition()
	print("Debug location added")
	storedPositions[#storedPositions + 1] = {pos=level.scene.camera.pos, dir=level.scene.camera.direction}
end

function printPositions()
	print("Printing positions")

	luaPositions = io.open("positions.lua", "w+")
	maxPositions = io.open("positions.txt", "w+")

	maxPositions:write(string.format("%i\n", #storedPositions))
	for i = 1,#storedPositions do
		position = storedPositions[i].pos
		direction = storedPositions[i].dir
		luaPositions:write(string.format("  {pos = {x= %f , y= %f , z = %f }, dir = {x= %f , y= %f , z= %f }},\n", position.x, position.y, position.z, direction.x, direction.y, direction.z))
		maxPositions:write(string.format("%i %i %i %i %i %i\n", position.x, position.y, position.z, direction.x * 10000, direction.y * 10000, direction.z * 10000))
	end
	luaPositions:close()
	maxPositions:close()
end

posDbgKeyListener = event.keyboard.Listener(function(kc, ks)
												if ks == 0 then
													if kc == 102 then
														addPosition()
													elseif kc == 103 then
														printPositions()
													end
												end
											end)