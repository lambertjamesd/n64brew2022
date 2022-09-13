local itemSlotOutput = {}

local itemSlotNodes = nodes_for_type("@item_slot")

itemSlotOutput.itemSlots = {}

for _, itemSlotNode in pairs(itemSlotNodes) do
    local scale, rotation, position = itemSlotNode.node.full_transformation:decompose()


    table.insert(itemSlotOutput.itemSlotCount, {
        position = position
    })
end

itemSlotOutput.itemSlotCount = #itemSlotOutput.itemSlots


add_definition("item_slots", "struct ItemSlotDefinition[]", "_geo", itemSlotOutput.itemSlots);

return itemSlotOutput