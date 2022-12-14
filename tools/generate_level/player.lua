
local playerOutput = {}

local playerNodes = nodes_for_type("@player")

local function sortNode(a, b)
    local aIndex = tonumber(a.arguments[1])
    local bIndex = tonumber(b.arguments[1])

    return aIndex < bIndex
end

table.sort(playerNodes, sortNode)

for i=1,4 do
    local current = playerNodes[i]
    local currentOutput = {}

    if (current) then
        local scale, rotation, position = current.node.full_transformation:decompose()
        currentOutput.position = position
    else 
        currentOutput.position = vector3(0, 0, 0)
    end

    table.insert(playerOutput, currentOutput)
end

return playerOutput