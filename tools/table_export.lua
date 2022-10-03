

defaultExports = export_default_mesh()

add_header('"math/box3d.h"')
add_header('"scene/table_type.h"')
add_header('"../build/assets/materials/static.h"')

local collisionNodes = nodes_for_type('@collision')

local collision = collisionNodes[1]

local box3

if (collision) then
    local mesh = collision.node.meshes[1]
    local transformedMesh = mesh:transform(collision.node.full_transformation)

    box3 = {
        transformedMesh.bbMin,
        transformedMesh.bbMax,
    }
else
    box3 = {
        vector3(0, 0, 0),
        vector3(0, 0, 0),
    }
end

local slotPositions = {}

for _, slot in pairs(nodes_for_type('@slot')) do
    local scale, rotation, position = slot.node.full_transformation:decompose()
    table.insert(slotPositions, position)
end

local surface = nodes_for_type('@shadow_caster')[1]

local surfaceVertices

if (surface) then
    local transformedMesh = surface.node.meshes[1]:transform(surface.node.full_transformation)
    surfaceVertices = {table.unpack(transformedMesh.vertices)}
end

add_definition("slots", "struct Vector3[]", "_geo", slotPositions)

add_definition("surface", "struct Vector3[]", "_geo", surfaceVertices);

add_definition("definition", "struct TableType", "_geo", {
    displayList = defaultExports.model,
    boundingBox = box3,
    itemSlots = reference_to(slotPositions[1]),
    itemSlotCount = #slotPositions,
    materialIndex = defaultExports.material,
    surfaceMesh = {
        vertices = reference_to(surfaceVertices[1]),
        vertexCount = #surfaceVertices,
    },
})