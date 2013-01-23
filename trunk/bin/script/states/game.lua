level = game.newLevel()
game.setLevel(level)
game.setCameraController(graphics.FreeFlyCamera())

level.scene.camera:setProjection(3.1415 / 4, 1920/1080, 10, 10000)

--[[
#############################################
			    UTILITIES
#############################################
]]--

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

function round(num, idp)
  local mult = 10^(idp or 0)
  return math.floor(num * mult + 0.5) / mult
end

game.loadFile("level.lua")

--CubeMan path
cmPath = { v3(1000, 70, 0), v3(-1000, 70, 0) }
cmPath.numNodes = 2
function cmPath:getNode(node)
	return cmPath[node]
end


cubeman = nil
dirlight = nil
spotlight = nil
lightAnimator = nil

activeShape = nil
inactiveShape = nil

function onLoaded()
	dirlight = level.scene:addDirectionalLight(math.normalize(v3(60, -325, 87)), v3(1, 1, 1))
	--dirlight = level.scene:addDirectionalLight(math.normalize(v3(0.2, -1, 0.2)), v3(1, 1, 1))
	
	--spotlight = level.scene:addSpotLight(v3(-258, 132, 518), v3(-1, 0, 0), 0.2, 100000, v3(0.2, 0.2, 0.2))

	cubeman = level:add("cubeman", {component.transform{x=200, y=70, z=0},
									component.mesh{file="cubeman.crm"}})
	activeShape =  level:add("sphere", {component.transform{x=0, y=0, z=0},
									component.mesh{file="sphere.crm"}})
	inactiveShape =  level:add("cube", {component.transform{x=-100000000000, y=-100000000, z=-10000000000},
									component.mesh{file="cube.crm"}})

	activeShape.transform.pos = activeShape.transform.pos + v3(1,1,1)*(211.17372 *0.25)

	level:build()
	--followPath(cubeman, cmPath, 100)

	--setTimer(cameraFollowPath, 1)
	setTimer(function() lightAnimator = game.beginUpdate(animLights) end, 1.15)
end


rx = 0.5
rz = 0.2
rspot = 0
function animLights(delta)
	--dirlight.direction = math.normalize(v3(math.sin(rx) * 0.3, -1, math.sin(rz) * 0.3))
	if spotlight ~= nil then
		--spotlight.direction = math.normalize(v3(math.sin(rspot), 0, math.cos(rspot)))
		spotlight.position = level.scene.camera.pos
		spotlight.direction = level.scene.camera.direction
	end
	rx = rx + 0.943247785 * delta
	rz = rz + 0.549334545 * delta
	rspot = rspot + 10.37 * delta
	return true
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


game.loadFile("cameraValues.lua")
currentPosition = 1

--[[
#############################################
			    REFERENCE SCREENSHOTS
#############################################
]]--

function takeScreenshot()
	print("Capturing screenshot at " .. currentPosition)
	graphics.captureScreenShot("screenshot" .. currentPosition .. ".png")
	currentPosition = currentPosition + 1
	
	tmr = setTimer(startReferenceGrab, 0.5)
end

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

--[[
#############################################
			    UI
#############################################
]]--

fpsLabel = ui.Label("FPS: ", 580, 10, 60, 30)
fpsUpd = game.beginUpdate(	function(delta)
								fpsLabel:setText(string.format("FPS: %.1f", game.getFps()))
								return true
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

--[[
#############################################
			    MOVABLE SHAPE
#############################################
]]--
function moveCube(delta)
	activeShape.transform.pos = activeShape.transform.pos + (delta * 0.5 * 211.17372)
end

function switchShape()
	temp = activeShape
	activeShape = inactiveShape
	inactiveShape = temp
	activeShape.transform.pos = inactiveShape.transform.pos
	inactiveShape.transform.pos = v3(-1000000000, -10000000000, -10000000000)
end

--[[
#############################################
			    KEY BINDINGS
#############################################
]]--



useShadows = true
keyActions = { 
		[108] = function() useIndirect = not useIndirect; graphics.useIndirectLighting(useIndirect) end,
		[107] = function() useDirect = not useDirect; graphics.useDirectLighting(useDirect) end,
		[111] = function() drawRays = not drawRays; graphics.drawRays(drawRays) end,
		[112] = function() currentPosition = 1; startReferenceGrab() end,
		[113] = function() graphics.captureScreenShot("screenshot.png") end,
		[105] = function() useShadows = not useShadows; graphics.useShadows(useShadows) end,

		[16] = function() moveCube(v3(1, 0, 0)) end,
		[17] = function() moveCube(v3(-1, 0, 0)) end,
		[18] = function() moveCube(v3(0, 0, 1)) end,
		[19] = function() moveCube(v3(0, 0, -1)) end,
		[32] = function() moveCube(v3(0, 1, 0)) end,
		[33] = function() moveCube(v3(0, -1, 0)) end,
		[109] = switchShape
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
	local idx = 0
	for b in timings do
		lbl = profGUI[idx]
		if lbl == nil then
			lbl =  ui.Label("", 10 + b.level * 10, y, 500, 20)
			lbl:setParent(profGUIWnd)
			profGUI[idx] = lbl
		end
		idx = idx + 1
		lbl:setText(string.format("%s - %.2f ms", b.name, b.time))
		lbl:setPosition(10 + b.level * 10, y, 500, 20)

		y = y + 20
	end
	return true
end

function printProfiling()
	timings = graphics.getTimings()
	profilingOutput = io.open("gpu_profiling.txt", "w+")
	for b in timings do
		print(b.time)
		profilingOutput:write(b.time, "\n")
	end
	profilingOutput:close()
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
													elseif kc == 104 then
														printProfiling()
													end
												end
											end)

--[[
############################################
			BENCHMARK TOOLS
############################################
]]--

timeMeasurements = {}

math.lerp = function(a, b, p)
	return b * p + a * (1 - p)
end

math.cspline = function(p0, p1, m0, m1, p, pdelta)
	local pcube = p * p * p
	local psq = p * p
	return	(2 * pcube - 3 * psq + 1) * p0 + 
			(pcube - 2 * psq + p) * pdelta * m0 + 
			(-2 * pcube + 3 * psq) * p1 +
			(pcube - psq) * pdelta * m1
end

math.min = function(a, b)
	if a < b then
		return a
	else
		return b
	end
end

math.max = function(a, b)
	if a > b then
		return a
	else
		return b
	end
end

cameraPath1 = 
{	
  {pos = v3(1153.880493, 634.536377, 548.073608), dir = v3(-0.982372, -0.088742, -0.164457), speed=100},
  {pos = v3(-847.113770, 587.971436, 546.413208), dir = v3(-0.137021, -0.105755, -0.984731), speed=100},
  {pos = v3(-878.775635, 535.113281, 188.442398), dir = v3(0.614129, -0.359333, -0.702461), speed=100},
  {pos = v3(-813.555603, 269.021149, 119.249512), dir = v3(0.957916, -0.151735, -0.243634), speed=100},
  {pos = v3(-152.540527, 160.923187, -44.296154), dir = v3(0.876104, -0.163454, 0.453452), speed=100},
  {pos = v3(982.577271, 97.685928, -110.684875), dir = v3(0.256811, -0.025016, 0.965945), speed=100},
  {pos = v3(1084.903687, 173.212631, 619.538208), dir = v3(-0.798506, -0.098240, -0.593717), speed=100},
  {pos = v3(-121.860031, 142.057938, 657.498413), dir = v3(0.028547, -0.011988, -0.999511), speed=100},
  {pos = v3(-1076.027832, 338.307312, 638.832092), dir = v3(0.677017, -0.407224, -0.612713), speed=100},
  {pos = v3(-1340.013550, 254.455933, 564.173157), dir = v3(0.014996, -0.167838, -0.985499), speed=100},
  {pos = v3(-1372.824341, 162.759369, -282.363617), dir = v3(0.961778, -0.115630, -0.247888), speed=100},
  {pos = v3(-746.626160, 232.787582, -593.299805), dir = v3(-0.062728, -0.347588, 0.935479), speed=100},
  {pos = v3(-370.550293, 171.397980, -522.080566), dir = v3(0.929210, -0.089359, 0.358003), speed=100},
  {pos = v3(514.545776, 201.102600, -428.581299), dir = v3(0.975580, -0.109166, -0.189901), speed=100},
  {pos = v3(1043.010742, 244.672134, -395.964203), dir = v3(0.904802, 0.090084, 0.416147), speed=100},
  {pos = v3(1036.592651, 172.404953, 23.643270), dir = v3(-0.916005, -0.184304, 0.355823), speed=100},
  {pos = v3(484.488800, 217.036118, 157.988663), dir = v3(-0.854038, -0.384090, -0.350548), speed=100},

}
cameraPath2 = 
{
  {pos = v3(1186.195435, 621.331421, -138.398621), dir = v3(-0.971042, 0.086857, 0.221509),		speed=100},
  {pos = v3(1101.012573, 634.775757, 168.166275), dir = v3(-0.995539, 0.090184, -0.018974),		speed=100},
  {pos = v3(978.082825, 634.743530, 409.338562), dir = v3(-0.955313, -0.019737, -0.294532),		speed=100},
  {pos = v3(-219.519196, 678.390320, 614.651733), dir = v3(-0.218273, -0.310359, -0.925003),	speed=100},
  {pos = v3(-753.717896, 710.265869, 602.345215), dir = v3(0.177320, -0.364522, -0.914107),		speed=100},
  {pos = v3(-913.067688, 642.417847, 453.584564), dir = v3(0.120704, -0.330416, -0.935858),		speed=100},
  {pos = v3(-883.904602, 571.345947, 192.375671), dir = v3(0.380565, -0.655401, -0.652107),		speed=100},
  {pos = v3(-398.041779, 146.901093, 50.174656), dir = v3(0.939225, -0.332812, -0.083997),		speed=100},
  {pos = v3(-84.982208, 95.212952, 105.460106), dir = v3(0.968984, -0.180823, 0.167136),		speed=100},
  {pos = v3(264.379242, 60.387955, 130.831131), dir = v3(0.993359, 0.050247, -0.102206),		speed=100},
  {pos = v3(804.544800, 116.750679, 60.605621), dir = v3(0.969702, 0.226482, -0.089983),		speed=100},
  {pos = v3(906.597412, 135.504517, 117.536674), dir = v3(0.829565, -0.045656, 0.556407),		speed=100},
  {pos = v3(1213.107910, 184.367874, 258.599762), dir = v3(-0.820521, 0.036227, 0.570372),		speed=100},
  {pos = v3(1010.407410, 301.897217, 599.884216), dir = v3(-0.678321, -0.301090, -0.669983),	speed=100},
  {pos = v3(389.584015, 270.144592, 576.754700), dir = v3(0.164289, -0.450091, -0.877481),		speed=100},
  {pos = v3(-46.504925, 307.611633, 591.628601), dir = v3(0.067804, -0.557274, -0.827543),		speed=100},
  {pos = v3(-675.562195, 335.197418, 567.680054), dir = v3(0.243188, -0.547982, -0.800071),		speed=100},
  {pos = v3(-961.291138, 276.331085, 568.058960), dir = v3(-0.588991, -0.219588, -0.777602),	speed=100},
  {pos = v3(-1258.668213, 212.324310, 354.276337), dir = v3(0.218684, -0.106356, -0.969947),	speed=100},
  {pos = v3(-1212.220459, 202.237274, -42.593948), dir = v3(0.989209, -0.053421, -0.135078),	speed=100},
  {pos = v3(-1264.126343, 202.237274, -262.062378), dir = v3(0.796156, -0.076722, -0.599875),	speed=100},
  {pos = v3(-731.447876, 212.172577, -467.158661), dir = v3(-0.129285, -0.378754, 0.916387),	speed=100},
  {pos = v3(-456.551270, 92.503006, -503.982910), dir = v3(0.848285, -0.066496, 0.525104),		speed=100},
  {pos = v3(282.817047, 91.600670, -544.715027), dir = v3(0.739336, -0.043440, 0.671859),		speed=100},
  {pos = v3(790.431519, 86.478561, -486.604706), dir = v3(0.761873, 0.013155, 0.647320),		speed=100},
  {pos = v3(905.107300, 75.740662, -500.582489), dir = v3(0.986114, -0.113022, -0.121287),		speed=100},
  {pos = v3(1195.062744, 53.960922, -560.650879), dir = v3(0.298415, 0.026431, 0.953956),		speed=100},
  {pos = v3(1240.490112, 59.867294, -296.597961), dir = v3(-0.026813, -0.016868, 0.999294),		speed=100},
  {pos = v3(986.697632, 144.412354, 72.020454), dir = v3(-0.733380, -0.051717, -0.677794),		speed=100},
  {pos = v3(547.419739, 290.219849, 180.086151), dir = v3(0.039921, -0.301972, -0.952230),		speed=100},
  {pos = v3(420.114990, 215.916733, 132.201813), dir = v3(-0.806208, -0.451299, -0.382204),		speed=100},
}

cameraPath3 = 
{
  {pos = v3(1134.235352, 667.533264, -140.141510), dir = v3(-0.986512, -0.114875, 0.116560), speed = 100},
  {pos = v3(1144.740601, 667.533264, 166.996002), dir = v3(-0.990390, 0.091179, -0.101764), speed = 100},
  {pos = v3(979.925720, 620.093933, 475.411652), dir = v3(-0.872087, -0.047368, -0.486736), speed = 100},
  {pos = v3(558.945374, 736.668396, 611.735901), dir = v3(-0.470710, -0.312197, -0.825172), speed = 100},
  {pos = v3(10.572622, 742.460449, 630.973450), dir = v3(-0.313197, -0.518298, -0.795512), speed = 100},
  {pos = v3(-759.839050, 639.941162, 596.675537), dir = v3(0.125124, -0.366846, -0.921677), speed = 100},
  {pos = v3(-905.693176, 597.322571, 424.240753), dir = v3(0.057298, -0.177309, -0.982288), speed = 100},
  {pos = v3(-893.988159, 542.568909, 234.240555), dir = v3(0.096206, -0.679920, -0.726829), speed = 100},
  {pos = v3(-843.712891, 275.895355, 107.442833), dir = v3(0.669896, -0.277013, -0.688644), speed = 100},
  {pos = v3(-678.295044, 223.820221, 46.092735), dir = v3(0.900273, -0.192344, -0.390429), speed = 100},
  {pos = v3(-418.103088, 129.369919, 76.985435), dir = v3(0.997341, 0.070483, 0.004123), speed = 100},
  {pos = v3(113.749374, 56.203529, 138.548233), dir = v3(0.999871, 0.001531, 0.007467), speed = 100},
  {pos = v3(569.037659, 250.318726, 84.302071), dir = v3(0.990490, -0.119069, -0.065334), speed = 100},
  {pos = v3(1200.221069, 226.546341, -17.788990), dir = v3(-0.306103, -0.116770, 0.944725), speed = 100},
  {pos = v3(1110.729370, 255.686874, 527.688904), dir = v3(-0.856970, -0.246492, -0.452567), speed = 100},
  {pos = v3(325.033875, 302.229401, 577.103333), dir = v3(0.009695, -0.564367, -0.825328), speed = 100},
  {pos = v3(123.248276, 277.307861, 583.681580), dir = v3(-0.525825, -0.369824, -0.765985), speed = 100},
  {pos = v3(-640.593689, 252.050018, 548.574280), dir = v3(0.549459, -0.447459, -0.705426), speed = 100},
  {pos = v3(-807.800842, 234.995804, 611.573181), dir = v3(-0.712145, -0.305244, -0.632039), speed = 100},
  {pos = v3(-1285.714355, 175.185318, 238.303711), dir = v3(0.926745, -0.067355, -0.369134), speed = 100},
  {pos = v3(-1241.939575, 164.889679, -8.487463), dir = v3(0.931694, -0.083934, -0.353231), speed = 100},
  {pos = v3(-1281.969116, 231.595901, -395.925903), dir = v3(0.910711, -0.228888, 0.343783), speed = 100},
  {pos = v3(-870.989319, 202.543976, -454.371552), dir = v3(0.997698, -0.063105, 0.014251), speed = 100},
  {pos = v3(-539.859985, 169.447037, -471.410461), dir = v3(0.529064, -0.194779, 0.825699), speed = 100},
  {pos = v3(59.569054, 155.952972, -491.118927), dir = v3(0.660224, 0.138438, 0.737980), speed = 100},
  {pos = v3(423.098328, 180.736893, -549.078491), dir = v3(0.590772, 0.056024, 0.804872), speed = 100},
  {pos = v3(1045.414917, 202.189331, -390.302277), dir = v3(-0.042736, -0.182178, 0.982126), speed = 100},
  {pos = v3(1096.264282, 210.552017, 154.074585), dir = v3(-0.710353, -0.216129, -0.669799), speed = 100},
  {pos = v3(862.252686, 197.768005, 119.427177), dir = v3(-0.518722, -0.093142, -0.849664), speed = 100},
  {pos = v3(474.798035, 257.984863, 168.383240), dir = v3(-0.737536, -0.510856, -0.441457), speed = 100},

}

cameraPath4 = 
{
  {pos = v3(1086.898804, 619.941833, -161.737061), dir = v3(-0.980025, 0.069954, 0.186051), speed = 100},
  {pos = v3(1090.959595, 619.941833, 210.175186), dir = v3(-0.984168, -0.043224, -0.171361), speed = 100},
  {pos = v3(598.382568, 778.996399, 624.960144), dir = v3(-0.010438, -0.457267, -0.889017), speed = 100},
  {pos = v3(-780.852417, 608.113586, 521.484192), dir = v3(0.110295, -0.273312, -0.955481), speed = 100},
  {pos = v3(-897.618103, 612.795532, 429.035919), dir = v3(0.071263, -0.305916, -0.949335), speed = 100},
  {pos = v3(-874.089600, 521.788147, 154.988556), dir = v3(0.062661, -0.703071, -0.708120), speed = 100},
  {pos = v3(-848.701965, 194.510864, 108.027519), dir = v3(0.026106, -0.382319, -0.923563), speed = 100},
  {pos = v3(-1267.431396, 160.110275, 53.331509), dir = v3(0.484973, -0.136303, -0.863786), speed = 100},
  {pos = v3(-993.140198, 191.945099, -506.868652), dir = v3(0.532633, -0.344011, 0.773004), speed = 100},
  {pos = v3(-643.803467, 202.585419, -483.517181), dir = v3(0.602388, -0.207016, 0.770850), speed = 100},
  {pos = v3(167.921371, 224.164413, -458.965363), dir = v3(-0.668202, -0.056557, 0.741673), speed = 100},
  {pos = v3(690.426208, 176.576675, -421.252625), dir = v3(-0.862326, 0.060742, 0.502318), speed = 100},
  {pos = v3(980.518799, 174.861328, -432.167419), dir = v3(-0.785931, 0.094326, 0.610970), speed = 100},
  {pos = v3(1199.300903, 174.870071, -70.185410), dir = v3(-0.971097, -0.118593, -0.206173), speed = 100},
  {pos = v3(1069.364258, 174.870071, 203.346268), dir = v3(-0.777704, -0.029311, -0.627876), speed = 100},
  {pos = v3(780.471008, 168.860962, 234.970291), dir = v3(-0.756415, -0.022648, -0.653507), speed = 100},
  {pos = v3(491.877380, 256.033142, 179.669647), dir = v3(-0.784809, -0.423445, -0.452001), speed = 100},

}

cameraPath = cameraPath3

function findTangents(path, dists)
	tangents = {}

	for i = 1,#path do
		local prevNode = path[math.max(1, i - 1)]
		local node = path[i]  
		local nextNode = path[math.min(#path, i + 1)]
		print(math.length(node.pos - prevNode.pos))
		print(math.length(nextNode.pos - node.pos))
		local dist = dists[math.min(#path, i + 1)] - dists[math.max(1, i - 1)]

		--Catmull-Rom tangents for spline interpolation
		local posTangent = (nextNode.pos - prevNode.pos) / dist
		local dirTangent = (nextNode.dir - prevNode.dir) / dist
		
		tangents[i] = {pos = posTangent, dir = dirTangent}
	end
	return tangents
end

function findDists(path)
	dists = {}

	local totalDist = 0
	for i = 1,#path do
		local prevNode = path[math.max(1, i - 1)]
		local node = path[i]
		totalDist = totalDist + math.length(node.pos - prevNode.pos)
		dists[i] = totalDist
	end
	return dists
end

function cameraFollowPath(camPath)
	camPath = camPath or cameraPath
	local target = 1
	local camera = level.scene.camera
	local progress = 0

	local dists = findDists(camPath)
	local tangents = findTangents(camPath, dists)

	return game.beginUpdate(function(delta)
								print(delta)
								local nextNode = camPath[target + 1]
								local nextTangent = tangents[target + 1]
								local targetNode = camPath[target]
								local targetTangent = tangents[target]
								local t_delta_nodes = dists[target + 1] - dists[target]

								local distance = math.length(targetNode.pos - targetNode.pos)
								local travelSpeed = math.lerp(targetNode.speed, nextNode.speed, (progress - dists[target]) / t_delta_nodes)
								progress = progress + travelSpeed * delta

								local t_delta = progress - dists[target]

								camera.pos = math.cspline(targetNode.pos, nextNode.pos, targetTangent.pos, nextTangent.pos, t_delta / t_delta_nodes, t_delta_nodes)
								camera.direction = math.normalize(math.cspline(targetNode.dir, nextNode.dir, targetTangent.dir, nextTangent.dir, t_delta / t_delta_nodes, t_delta_nodes))

								if progress >= dists[target + 1] then
									if target + 1 == #camPath then
										setTimer(cameraFollowPath, 1)
										return false
									else
										target = target + 1
									end
								end
								return true
							end)
end