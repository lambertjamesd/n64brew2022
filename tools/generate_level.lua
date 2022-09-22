local staticContentOutput = require('tools.generate_level/static')
local playerOutput = require('tools.generate_level/player')
local cameraOutput = require('tools.generate_level/camera')
local itemSlotOutput = require('tools.generate_level/item_slots')
local cameraOutput = require('tools.generate_level/camera')
local spotLightOutput = require('tools.generate_level/spot_light')
local conveyorOutput = require('tools.generate_level.conveyor')

add_definition("level", "struct LevelDefinition", "_geo", {
    staticContent = reference_to(staticContentOutput.staticContent[1]),
    staticContentCount = #staticContentOutput.staticContent,

    groundContent = reference_to(staticContentOutput.groundContent[1]),
    groundContentCount = #staticContentOutput.groundContent,

    itemSlots = reference_to(itemSlotOutput.itemSlots[1]),
    itemSlotCount = itemSlotOutput.itemSlotCount,

    spotLights = reference_to(spotLightOutput.lanterns[1]),
    spotLightCount = #spotLightOutput.lanterns,

    conveyors = reference_to(conveyorOutput.conveyors[1]),
    conveyorCount = #conveyorOutput.conveyors,

    playerStart = playerOutput,

    cameraDefinition = cameraOutput,
})