local cameraOutput = {}

if (#scene.cameras > 0) then
    local camera = scene.cameras[1]

    local node = node_with_name(camera.name)

    if (not node) then
        error("could not find node for camera " .. camera.name)
    end

    local cameraTransform = node.full_transformation * camera.local_transform

    scale, rotation, position = cameraTransform:decompose();

    local tanAngle = math.tan(camera.horizontal_fov * 0.5)

    local vericalFov = math.atan(tanAngle / camera.aspect_ratio) * 2

    print(rotation)

    cameraOutput.position = position
    cameraOutput.rotation = rotation
    cameraOutput.verticalFov = vericalFov * 180 / math.pi
else
    cameraOutput.position = vector3(0, 0, 0)
    cameraOutput.rotation = quaternion(0, 0, 0, 1)
    cameraOutput.verticalFov = 90
end

return cameraOutput