local lunajson = require('lunajson')

local json_filename = string.sub(input_filename, 7, -4) .. 'json'
print("Loading json file for level " .. json_filename)

local file = io.open(json_filename, 'r')
local file_contents = file:read('*all')
file:close()

local json_body = lunajson.decode(file_contents)

local script_items = {}
local script_steps = {}

for _, script_entry in pairs(json_body.script) do
    local current_step = {
        itemPool = reference_to(script_items, #script_items + 1),
        itemPoolSize = #script_entry.item_pool,
        successCount = script_entry.success_count,
    }

    for _, item in pairs(script_entry.item_pool) do
        table.insert(script_items, raw("ItemType" .. item))
    end

    table.insert(script_steps, current_step)
end

local level_json_exports = {}

level_json_exports.script = {
    steps = reference_to(script_steps, 1),
    stepCount = #script_steps,
}

add_definition("script_step_items", "u8[]", "_geo", script_items);
add_definition("script_steps", "struct ItemScriptStep[]", "_geo", script_steps);
add_definition("script", "struct ItemScript", "_geo", level_json_exports.script);

return level_json_exports