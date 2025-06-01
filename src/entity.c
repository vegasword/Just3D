void UpdateCenteredModelTransformMatrix(Model *model)
{
  v3 pivot = HMM_DivV3F(HMM_SubV3(model->bounds.max, model->bounds.min), 2);
  model->transformMatrix = GetPivotedTransformMatrix(model->transform, pivot);
}

void UpdateCamera(Camera *camera, GameInputs inputs, f32 deltaTime)
{
  GameButtons buttons = inputs.buttons;
  
  f32 mouseSpeed = deltaTime * 0.01f;
  f32 pitch = camera->pitch;
  f32 yaw = camera->yaw;
  f32 dX = HMM_SinF(yaw);
  f32 dZ = HMM_CosF(yaw);
  v3 direction = {0};
  
  pitch += inputs.mousePosition.Y * mouseSpeed;
  yaw += inputs.mousePosition.X * mouseSpeed;
  
  if (pitch < -0.25) pitch = -0.25; else if (pitch >  0.25) pitch = 0.25;  
  if (yaw < 0) yaw = 1; else if (yaw > 1) yaw = 0;
  
  if (buttons.moveForward)  { direction.X += dX; direction.Z -= dZ; }
  if (buttons.moveBackward) { direction.X -= dX; direction.Z += dZ; }
  
  if (buttons.moveLeft)  { direction.X -= dZ; direction.Z -= dX; }
  if (buttons.moveRight) { direction.X += dZ; direction.Z += dX; }  
  
  if (buttons.moveUp)   { direction.Y++; }
  if (buttons.moveDown) { direction.Y--; }
  
  v3 cameraPosition = camera->transform.position;
  m4 translation = HMM_Translate(cameraPosition);
  qt qPitch = HMM_QFromAxisAngle_RH((v3){-1, 0, 0}, pitch);
  qt qYaw = HMM_QFromAxisAngle_RH((v3){0, -1, 0}, yaw);
  m4 rotation = HMM_QToM4(HMM_NormQ(HMM_MulQ(qYaw, qPitch)));  

  if (!isZeroV3(direction))
  {
    direction = HMM_MulV3F(HMM_NormV3(direction), deltaTime * camera->speed);
    camera->transform.position = HMM_AddV3(cameraPosition, direction);
  }
  
  camera->pitch = pitch;
  camera->yaw = yaw;
  camera->viewMatrix = HMM_InvGeneralM4(HMM_MulM4(translation, rotation));
  camera->projectionMatrix = HMM_Perspective_RH_NO(camera->fov, camera->aspect, 0.01, 1000);
}
