
local boundaryOutput = {}

boundaryOutput.boundaryEntries = {}

local firstNode = nodes_for_type("@boundary")[1]
local firstMesh = firstNode and firstNode.node.meshes[1]:transform(firstNode.node.full_transformation)

local function edgeKey(a, b)
    if (a < b) then
        return a .. ',' .. b
    else
        return b .. ',' .. a
    end
end

local function markEdge(cache, key)
    cache[key] = (cache[key] or 0) + 1
end

local function checkForEdge(mesh, edgeUseCount, a, b)
    if (edgeUseCount[edgeKey(a, b)] == 1) then
        table.insert(boundaryOutput.boundaryEntries, {
            {x = mesh.vertices[a].x, y = mesh.vertices[a].z},
            {x = mesh.vertices[b].x, y = mesh.vertices[b].z},
        })
    end
end

if (firstMesh) then
    local edgeUseCount = {}

    for _, face in pairs(firstMesh.faces) do
        markEdge(edgeUseCount, edgeKey(face[1], face[2]))
        markEdge(edgeUseCount, edgeKey(face[2], face[3]))
        markEdge(edgeUseCount, edgeKey(face[3], face[1]))
    end

    for _, face in pairs(firstMesh.faces) do
        checkForEdge(firstMesh, edgeUseCount, face[1], face[2]);
        checkForEdge(firstMesh, edgeUseCount, face[2], face[3]);
        checkForEdge(firstMesh, edgeUseCount, face[3], face[1]);
    end
end

add_definition("boundary", "struct BoundarySegment[]", "_geo", boundaryOutput.boundaryEntries)

return boundaryOutput