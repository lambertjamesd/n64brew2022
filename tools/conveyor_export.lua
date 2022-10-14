

defaultExports = export_default_mesh()

add_header('"math/box3d.h"')
add_header('"scene/conveyor_type.h"')
add_header('"../build/assets/materials/static.h"')

local collisionNodes = nodes_for_type('@collision')

local collision = collisionNodes[1]

local box3

if (collision) then
    local mesh = collision.node.meshes[1]
    local transformedMesh = mesh:transform(collision.node.full_transformation)

    print(mesh.bbMin)
    print(mesh.bbMax)

    print(transformedMesh.bbMin)
    print(transformedMesh.bbMax)

    box3 = {
        mesh.bbMin,
        mesh.bbMax,
    }
else
    box3 = {
        vector3(0, 0, 0),
        vector3(0, 0, 0),
    }
end

add_definition("definition", "struct ConveyorType", "_geo", {
    displayList = defaultExports.model,
    boundingBox = box3,
    materialIndex = defaultExports.material,
})