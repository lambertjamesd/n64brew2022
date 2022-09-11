local staticContentOutput = require('generate_level/static')
local playerOutput = require('generate_level/player')

add_definition("level", "struct LevelDefinition", "_geo", {
    staticContent = reference_to(staticContentOutput.staticContent[1]),
    staticContentCount = #staticContentOutput.staticContent,

    playerStart = playerOutput,
})