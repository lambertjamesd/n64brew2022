local staticContentOutput = require('generate_level/static')
local playerOutput = require('generate_level/player')
local cameraOutput = require('generate_level/camera')
local itemSlotOutput = require('generate_level/item_slots')
local cameraOutput = require('generate_level/camera')

add_definition("level", "struct LevelDefinition", "_geo", {
    staticContent = reference_to(staticContentOutput.staticContent[1]),
    staticContentCount = #staticContentOutput.staticContent,

    itemSlots = itemSlotOutput.itemSlots[1] and reference_to(itemSlotOutput.itemSlots[1]),
    itemSlotCount = itemSlotOutput.itemSlotCount,

    playerStart = playerOutput,

    cameraDefinition = cameraOutput,
})