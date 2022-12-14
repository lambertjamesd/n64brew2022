local tableOutput = {}

local tables = {}

for _, current in pairs(nodes_for_type('@table')) do
    local scale, rotation, position = current.node.full_transformation:decompose()

    table.insert(tables, {
        position = position,
        tableType = raw(current.node.meshes[1].name)
    })
end

add_definition("tables", "struct TableDefinition[]", "_geo", tables)

tableOutput.tables = tables

return tableOutput