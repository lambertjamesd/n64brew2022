-- set_paths.lua
local version = _VERSION:match("%d+%.%d+")
package.path = 'lua_modules/share/lua/' .. version .. '/?.lua;lua_modules/share/lua/' .. version .. '/?/init.lua;' .. package.path
package.cpath = 'lua_modules/lib/lua/' .. version .. '/?.so;' .. package.cpath

local staticContentOutput = require('tools.generate_level.static')
local playerOutput = require('tools.generate_level.player')
local cameraOutput = require('tools.generate_level.camera')
local cameraOutput = require('tools.generate_level.camera')
local spotLightOutput = require('tools.generate_level.spot_light')
local conveyorOutput = require('tools.generate_level.conveyor')
local tableOutput = require('tools.generate_level.table')
local itemRequesterOutput = require('tools.generate_level.item_requester')
local boundaryOutput = require('tools.generate_level.boundary')

local levelJson = require('tools.generate_level.level_json')

local return_bin_output = require('tools.generate_level.return_bin')

add_definition("level", "struct LevelDefinition", "_geo", {
    staticContent = reference_to(staticContentOutput.staticContent[1]),
    staticContentCount = #staticContentOutput.staticContent,

    groundContent = reference_to(staticContentOutput.groundContent[1]),
    groundContentCount = #staticContentOutput.groundContent,

    spotLights = reference_to(spotLightOutput.lanterns[1]),
    spotLightCount = #spotLightOutput.lanterns,

    conveyors = reference_to(conveyorOutput.conveyors[1]),
    conveyorCount = #conveyorOutput.conveyors,

    tables = reference_to(tableOutput.tables[1]),
    tableCount = #tableOutput.tables,

    itemRequesters = reference_to(itemRequesterOutput.itemRequesters[1]),
    itemRequesterCount = #itemRequesterOutput.itemRequesters,

    playerStart = playerOutput,

    cameraDefinition = cameraOutput,

    script = reference_to(levelJson.script),

    boundary = reference_to(boundaryOutput.boundaryEntries, 1),
    boundaryCount = #boundaryOutput.boundaryEntries,

    tutorial = levelJson.tutorial,
    tutorialStepCount = levelJson.tutorial_step_count,
    tutorialOnStart = levelJson.tutorial_on_start,

    returnBins = reference_to(return_bin_output.return_bins, 1),
    returnBinCount = #return_bin_output.return_bins,
})