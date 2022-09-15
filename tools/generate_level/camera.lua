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

    cameraOutput.position = position
    cameraOutput.rotation = rotation
    cameraOutput.verticalFov = vericalFov * 180 / math.pi
    cameraOutput.nearPlane = camera.near_plane * settings.model_scale
    cameraOutput.farPlane = camera.far_plane * settings.model_scale
else
    cameraOutput.position = vector3(0, 0, 0)
    cameraOutput.rotation = quaternion(0, 0, 0, 1)
    cameraOutput.verticalFov = 90
    cameraOutput.nearPlane = 5
    cameraOutput.farPlane = 20
end

return cameraOutput