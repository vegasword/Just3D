void UpdateCenteredModelTransformMatrix(Model *model)
{
  v3 pivot = HMM_DivV3F(HMM_SubV3(model->bounds.max, model->bounds.min), 2);
  model->transformMatrix = GetPivotedTransformMatrix(model->transform, pivot);
}

void UpdateCamera(Camera *camera, GameInputs *inputs, f32 deltaTime)
{
  GameButtons buttons = inputs->buttons;

  f32 mouseSensitivity = inputs->mouseSensitivity;
  f32 pitch = camera->pitch + inputs->mouseDelta.Y * mouseSensitivity;
  f32 yaw = camera->yaw + inputs->mouseDelta.X * mouseSensitivity;
  
  if (pitch < -0.25) pitch = -0.25; else if (pitch > 0.25) pitch = 0.25;  
  if (yaw < 0) yaw = 1; else if (yaw > 1) yaw = 0;
  
  camera->pitch = pitch;
  camera->yaw = yaw;
  inputs->mouseDelta = (v2){0};

  v3 direction = {0};
  f32 dX = HMM_SinF(yaw);
  f32 dZ = HMM_CosF(yaw);

  if (buttons.moveForward)  { direction.X += dX; direction.Z -= dZ; }
  if (buttons.moveBackward) { direction.X -= dX; direction.Z += dZ; }
  if (buttons.moveLeft)     { direction.X -= dZ; direction.Z -= dX; }
  if (buttons.moveRight)    { direction.X += dZ; direction.Z += dX; }
  if (buttons.moveUp)       { direction.Y += 1.0f; }
  if (buttons.moveDown)     { direction.Y -= 1.0f; }

  if (!isZeroV3(direction))
  {
    direction = HMM_MulV3F(HMM_NormV3(direction), deltaTime * camera->speed);
    camera->transform.position = HMM_AddV3(camera->transform.position, direction);
  }

  f32 cosPitch = HMM_CosF(pitch);
  f32 sinPitch = HMM_SinF(pitch);
  f32 cosYaw = HMM_CosF(yaw);
  f32 sinYaw = HMM_SinF(yaw);

  m4 rotationPitch = (m4) {
    1,        0,         0, 0,
    0, cosPitch, -sinPitch, 0,
    0, sinPitch,  cosPitch, 0,
    0,        0,         0, 1
  };

  m4 rotationYaw = (m4) {
     cosYaw, 0, sinYaw, 0,
          0, 1,      0, 0,
    -sinYaw, 0, cosYaw, 0,
          0, 0,      0, 1
  };

  m4 rotation = HMM_MulM4(rotationYaw, rotationPitch);
  m4 translation = HMM_Translate(camera->transform.position);

  camera->viewMatrix = HMM_InvGeneralM4(HMM_MulM4(translation, rotation));
  camera->projectionMatrix = HMM_Perspective_RH_NO(camera->fov, camera->aspect, 0.01f, 1000.0f);
}

DirectionalLight CreateDirectionalLight(v3 direction, v3 color, f32 intensity)
{
  return (DirectionalLight) {
    .direction = (v4){ .XYZ = direction, .W = 1.0f },
    .color = color,
    .intensity = intensity,
  };
}

PunctualLight CreatePointLight(v3 position, v3 direction, v3 color, f32 intensity, f32 falloffRadius)
{
  return (PunctualLight) {
    .position = (v4){ .XYZ = position },
    .direction = (v4){ .XYZ = direction },
    .color = color,
    .intensity = intensity,
    .inverseFalloffRadius = 1.0f / falloffRadius,
    .isSpotlight = 0,
  };
}

PunctualLight CreateSpotLight(v3 position, v3 direction, v3 color, f32 intensity, f32 falloffRadius, f32 innerAngle, f32 outerAngle)
{
  f32 cosInnerAngle = HMM_CosF(innerAngle);
  f32 cosOuterAngle = HMM_CosF(outerAngle);
  return (PunctualLight) {
    .position = (v4){ .XYZ = position },
    .direction = (v4){ .XYZ = direction },
    .color = color,
    .intensity = intensity,
    .inverseFalloffRadius = 1.0f / falloffRadius,
    .isSpotlight = 1,
    .cosOuterAngle = cosOuterAngle,
    .spotScale = 1.0f / fmaxf(cosInnerAngle - cosOuterAngle, 0.001f),
  };
}
