local staticContentOutput = {}

local function proccessStaticNodes(nodes)
    local result = {}

    for k, v in pairs(nodes) do
        local renderChunks = generate_render_chunks(v.node)
        
        for _, chunkV in pairs(renderChunks) do
            local gfxName = generate_mesh({chunkV}, "_geo", {defaultMaterial = chunkV.material})
    
            table.insert(result, {displayList = raw(gfxName), materialIndex = raw(chunkV.material.macro_name)})
        end
    end

    return result;
end

local staticContent = proccessStaticNodes({ table.unpack(nodes_for_type("@static")), table.unpack(nodes_for_type("")) })

add_header('"level/level_definition.h"')
add_header('"../build/assets/materials/static.h"')
add_definition("static", "struct StaticContentElement[]", "_geo", staticContent);

local groundContent = proccessStaticNodes(nodes_for_type("@ground"))

add_definition("ground", "struct StaticContentElement[]", "_geo", groundContent);

staticContentOutput.staticContent = staticContent
staticContentOutput.groundContent = groundContent

return staticContentOutput