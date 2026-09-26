-- What the preset offers under FUNCTIONS, one entry per knob.
--
-- The name goes on the button, call is what running it does, and close asks
-- for the window to be put away afterwards.

local function toPage(pageId)
    return function()
        commands.run("switchPage", pageId)
    end
end

preset.userFunctions = {
    pot1  = { name = "DEFAULTS", call = function()
                  commands.run("setAllDefaultValues")
              end },
    pot2  = { name = "RANGES",   call = toPage(1), close = true },
    pot3  = { name = "DETENTS",  call = toPage(2), close = true },
    pot5  = { name = "SET 1",    call = function()
                  commands.run("switchControlSet", 1)
              end },
    pot6  = { name = "SET 2",    call = function()
                  commands.run("switchControlSet", 2)
              end },
    pot7  = { name = "SET 3",    call = function()
                  commands.run("switchControlSet", 3)
              end },
    pot11 = { name = "SNAPSHOTS", call = function()
                  commands.run("openSnapshots")
              end, close = true },
    pot12 = { name = "PRESETS",  call = function()
                  commands.run("openPresetsMenu")
              end, close = true }
}

-- Reports what every value on the page thinks its knob rate is.
function reportSensitivity()
    for id = 1, 42 do
        local control = controls.get(id)
        if control ~= nil then
            local value = control:getValue(0)
            if value ~= nil then
                print(string.format("SENS control %d (%s) = %d/1000",
                                    id, control:getName(),
                                    math.floor(value:getSensitivity() * 1000 + 0.5)))
            end
        end
    end
end

-- Prints every command this instrument knows.
function reportCommands()
    for i, name in ipairs(commands.getNames()) do
        print(string.format("CMD %d %s", i, name))
    end
end
