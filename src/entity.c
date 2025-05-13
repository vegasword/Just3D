void UpdateEntityCenteredModelMatrix(Entity *entity)
{
  Boundings boundings = {0};
  if (entity->model)
  {
    boundings = entity->model->boundings;
  }
  else
  {
    v3 position = entity->transform->position;
    v3 halfExtents = HMM_MulV3F(entity->transform->scale, 0.25f);

    boundings = (Boundings) {
      .min = HMM_SubV3(position, halfExtents),
      .max = HMM_AddV3(position, halfExtents)
    };
  }
    
  *entity->modelMatrix = GetPivotedTransformMatrix(entity->transform, HMM_DivV3F(HMM_SubV3(boundings.max, boundings.min), 2));
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
  
  if (pitch < -0.25)
  {
    pitch = -0.25;
  }
  else if (pitch >  0.25)
  {
    pitch = 0.25;
  }
  
  if (yaw < 0)
  {
    yaw = 1;
  }
  else if (yaw > 1)
  {
    yaw = 0;
  }
  
  if (buttons.moveForward) 
  {
    direction.X += dX;
    direction.Z -= dZ;
  }

  if (buttons.moveBackward)
  {
    direction.X -= dX;
    direction.Z += dZ;
  }

  if (buttons.moveLeft)
  {
    direction.X -= dZ;
    direction.Z -= dX;
  }

  if (buttons.moveRight)
  {
    direction.X += dZ;
    direction.Z += dX;
  }  
  
  if (buttons.moveUp)
  {
    direction.Y++;
  }

  if (buttons.moveDown)
  {
    direction.Y--;
  }
  
  Transform transform = *camera->transform;
  m4 translation = HMM_Translate(transform.position);
  qt qPitch = HMM_QFromAxisAngle_RH((v3){-1, 0, 0}, pitch);
  qt qYaw = HMM_QFromAxisAngle_RH((v3){0, -1, 0}, yaw);
  m4 rotation = HMM_QToM4(HMM_NormQ(HMM_MulQ(qYaw, qPitch)));  

  if (!isZeroV3(direction))
  {
    direction = HMM_MulV3F(HMM_NormV3(direction), deltaTime * 0.003f);
    transform.position = HMM_AddV3(transform.position, direction);
  }
  
  camera->pitch = pitch;
  camera->yaw = yaw;
  *camera->transform = transform;
  *camera->view = HMM_InvGeneralM4(HMM_MulM4(translation, rotation));
  *camera->projection = HMM_Perspective_RH_NO(camera->fov, camera->aspect, 0.01, 1000);
}
