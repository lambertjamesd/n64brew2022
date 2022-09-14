local spotLightOutput = {}

local lanterns = {}

for _, current in pairs(nodes_for_type('@lantern')) do
    local scale, rotation, position = current.node.full_transformation:decompose()

    table.insert(lanterns, {
        position = position,
        rotation = rotation,
        angle = math.pi / 3
    })
end

add_definition("lanterns", "struct SpotLightDefinition[]", "_geo", lanterns)

spotLightOutput.lanterns = lanterns

return spotLightOutput