local cameraOutput = require('tools.generate_level/camera')

export_default_mesh()

add_header('"level/level_definition.h"')

add_definition("camera", "struct CameraDefinition", "_geo", cameraOutput)

local light = nodes_for_type('@light')[1]

if (light) then
    local scale, rotation, position = light.node.full_transformation:decompose()

    add_definition("light", "struct Vector3", "_geo", rotation * vector3(0, 1, 0))
end