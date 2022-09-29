local itemRequesterOutput = {}

local itemRequesterNodes = nodes_for_type("@item_requester")

itemRequesterOutput.itemRequesters = {}

for _, itemRequesterNode in pairs(itemRequesterNodes) do
    local scale, rotation, position = itemRequesterNode.node.full_transformation:decompose()


    table.insert(itemRequesterOutput.itemRequesters, {
        position = position,
        rotation = rotation,
    })
end

add_definition("item_requesters", "struct ItemRequesterDefinition[]", "_geo", itemRequesterOutput.itemRequesters);

return itemRequesterOutput