local lunajson = require('lunajson')

local json_filename = string.sub(input_filename, 7, -4) .. 'json'
print("Loading json file for level " .. json_filename)

local file = io.open(json_filename, 'r')
local file_contents = file:read('*all')
file:close()

local json_body = lunajson.decode(file_contents)

local script_items = {}
local script_steps = {}


local tutorial_json = json_body.tutorial or {}

local ordered_tutorial = {}

for name, step in pairs(tutorial_json) do
    table.insert(ordered_tutorial, {name = name, step = step})
end

table.sort(ordered_tutorial, function (a, b)
    return a.name < b.name
end)

local tutorial_step_mapping = {}
local tutoral_steps_count = 0

for _, pair in pairs(ordered_tutorial) do
    tutorial_step_mapping[pair.name] = tutoral_steps_count
    tutoral_steps_count = tutoral_steps_count + 1
end

local function tutorial_name_to_index(name)
    if (not name or name == "") then
        return -1
    end

    local result = tutorial_step_mapping[name]

    if (result == nil) then
        error("could not find script step named " .. name)
    end

    return result
end

local function translate_prompt(prompt)
    if (prompt == "pickup") then
        return raw("TutorialPromptTypePickup")
    end

    if (prompt == "drop") then
        return raw("TutorialPromptTypeDrop")
    end
end

local tutorial_data = {}

for _, pair in pairs(ordered_tutorial) do
    local name = pair.name
    local step = pair.step

    local dialog_ref = nil
    local dialog_count = 0

    local text_array = {}

    if (step.dialog) then
        local mapped_dialog = {}

        for _, dialogEntry in pairs(step.dialog) do
            table.insert(mapped_dialog, {
                message = reference_to(text_array, #text_array + 1)
            })

            for i = 1, #dialogEntry.message do
                table.insert(text_array, string.byte(dialogEntry.message, i))
            end

            table.insert(text_array, 0)
        end
        
        add_definition("dialog_" .. name .. "_messages", "char[]", "_geo", text_array)
        add_definition("dialog_" .. name, "struct TutorialDialogStep[]", "_geo", mapped_dialog);
        dialog_ref = reference_to(mapped_dialog, 1)
        dialog_count = #mapped_dialog
    end

    table.insert(tutorial_data, {
        dialog = dialog_ref,
        dialogCount = dialog_count,
        nextState = tutorial_name_to_index(step.next),
        onSuccess = tutorial_name_to_index(step.onSuccess),
        onFail = tutorial_name_to_index(step.onFail),
        onTable = tutorial_name_to_index(step.onTable),
        onPickup = tutorial_name_to_index(step.onPickup),
        prompt = translate_prompt(step.prompt),
        isImmune = step.isImmune and 1 or 0,
    })
end

for _, script_entry in pairs(json_body.script) do
    local current_step = {
        itemPool = reference_to(script_items, #script_items + 1),
        itemPoolSize = #script_entry.item_pool,
        successCount = script_entry.success_count,
        itemTimeout = script_entry.item_timeout or 30,
        itemDelay = script_entry.item_delay or 0,
        itemSpawnDelay = script_entry.item_spawn_delay or 0,
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

level_json_exports.tutorial = reference_to(tutorial_data, 1);
level_json_exports.tutorial_step_count = #tutorial_data
level_json_exports.tutorial_on_start = tutorial_name_to_index(json_body.tutorialOnStart)

add_definition("tutorial", "struct TutorialStep[]", "_geo", tutorial_data);
add_definition("script_step_items", "u8[]", "_geo", script_items);
add_definition("script_steps", "struct ItemScriptStep[]", "_geo", script_steps);
add_definition("script", "struct ItemScript", "_geo", level_json_exports.script);

return level_json_exports