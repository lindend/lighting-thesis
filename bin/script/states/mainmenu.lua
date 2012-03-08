res = game.getResolution()
bg = ui.Image("Menu/bg.png", 0, 0, res.x, res.y)
newGame = ui.Button("New game", 100, 300, 100, 30, function() game.setState("game") end)
exitGame = ui.Button("Exit game", 100, 350, 100, 30, function() game.shutDown() end)