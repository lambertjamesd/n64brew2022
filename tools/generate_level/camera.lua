local cameraOutput = {}

if (#scene.cameras > 0) then
    local node = node_with_name(scene.cameras[1].name)

    if (not node) then
        error("could not find node for camera " .. scene.cameras[1].name)
    end

    local cameraTransform = settings.model_transform * node.full_transformation

    scale, rotation, position = cameraTransform:decompose();

    print(scale)
    print(rotation)
    print(position)
else
    cameraOutput.position = vector3(0, 0, 0)
    cameraOutput.lookAt = vector3(0, 0, 0)
end
print(node_with_name(scene.cameras[1].name))

return cameraOutput