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

introImgs = { "havokLogo.jpg", "crazeLogo.png" }
timer = nil
currentImage = nil

--keyListener = event.keyboard.listen(function(kc, ks) print("Key " .. kc); timer = nil; currentImage = nil; keyListener = nil; game.setState("mainmenu") end)
keyListener = event.keyboard.Listener(function(kc, ks) game.setState("mainmenu") end)

function showImage(imgName, fadeTime)
	res = game.getResolution()
	currentImage = ui.Image(imgName, 0, 0, res.x, res.y)
	local img = currentImage
	fader(fadeTime, 0.0, 1.0, function (v) img:setColor(1, 1, 1, v) end)
end

function showIntro()
	if #introImgs == 0 then
		game.setState("mainmenu")
		return
	end
	
	local fadeTime = 0.2
	local displayTime = 1.0
	local img = introImgs[#introImgs]
	introImgs[#introImgs] = nil
	
	--print("I am now displaying an awesome image of " .. img)
	showImage(img, fadeTime)
	timer = setTimer(function()
						fader(fadeTime, 1.0, 0.0, function (v) currentImage:setColor(1, 1, 1, v) end)
						timer = setTimer(showIntro, fadeTime)
					end, displayTime)
end

showIntro()