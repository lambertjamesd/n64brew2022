local staticContentOutput = require('generate_level/static')
local playerOutput = require('generate_level/player')
local cameraOutput = require('generate_level/camera')
local itemSlotOutput = require('generate_level/item_slots')
local cameraOutput = require('generate_level/camera')
local spotLightOutput = require('generate_level/spot_light')

add_definition("level", "struct LevelDefinition", "_geo", {
    staticContent = reference_to(staticContentOutput.staticContent[1]),
    staticContentCount = #staticContentOutput.staticContent,

    groundContent = reference_to(staticContentOutput.groundContent[1]),
    groundContentCount = #staticContentOutput.groundContent,

    itemSlots = reference_to(itemSlotOutput.itemSlots[1]),
    itemSlotCount = itemSlotOutput.itemSlotCount,

    spotLights = reference_to(spotLightOutput.lanterns[1]),
    spotLightCount = #spotLightOutput.lanterns,

    playerStart = playerOutput,

    cameraDefinition = cameraOutput,
})