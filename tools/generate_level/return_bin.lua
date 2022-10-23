local return_bin_output = {}

local return_bins = {}

for _, current in pairs(nodes_for_type('@return_bin')) do
    local scale, rotation, position = current.node.full_transformation:decompose()

    table.insert(return_bins, {
        position = position,
    })
end

add_definition("return_bins", "struct ReturnBinDefinition[]", "_geo", return_bins)

return_bin_output.return_bins = return_bins

return return_bin_output