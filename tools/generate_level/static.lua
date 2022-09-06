local staticContentOutput = {}

local staticNodes = nodes_for_type("@static")

local staticContent = {}

for k, v in pairs(staticNodes) do
    local renderChunks = generate_render_chunks(v.node)
    
    for _, chunkV in pairs(renderChunks) do
        local gfxName = generate_mesh({chunkV}, "_geo", {defaultMaterial = chunkV.material})

        table.insert(staticContent, {displayList = raw(gfxName), materialIndex = raw(chunkV.material.macro_name)})
    end
end

add_header('"level/level_definition.h"')
add_header('"../build/assets/materials/static.h"')
add_definition("static", "struct StaticContentElement[]", "_geo", staticContent);

staticContentOutput.staticContent = staticContent

return staticContentOutput