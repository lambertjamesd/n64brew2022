local conveyorOutput = {}

local conveyors = {}

for _, current in pairs(nodes_for_type('@conveyor')) do
    local scale, rotation, position = current.node.full_transformation:decompose()

    table.insert(conveyors, {
        position = position,
        rotation = rotation
    })
end

add_definition("conveyors", "struct ConveyorDefinition[]", "_geo", conveyors)

conveyorOutput.conveyors = conveyors

return conveyorOutput