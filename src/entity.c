void UpdateEntityCenteredTransformMatrix(Entity *entity)
{
  Bounds entityBounds = {0};
  Model *model = entity->component.model;
  Transform entityTransform = entity->transform;
  
  if (model)
  {
    entityBounds = model->bounds;
  }
  else
  {
    v3 entityPosition = entityTransform.position;
    v3 entityHalfExtents = HMM_MulV3F(entityTransform.scale, 0.25f);
    
    entityBounds = (Bounds) {
      .min = HMM_SubV3(entityPosition, entityHalfExtents),
      .max = HMM_AddV3(entityPosition, entityHalfExtents)
    };
  }
    
  v3 pivot = HMM_DivV3F(HMM_SubV3(entityBounds.max, entityBounds.min), 2);
  entity->transformMatrix = GetPivotedTransformMatrix(entityTransform, pivot);
}

void UpdateCamera(Entity *entity, GameInputs inputs, f32 deltaTime)
{
  Camera *camera = entity->component.camera;
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
  
  v3 cameraPosition = entity->transform.position;
  m4 translation = HMM_Translate(cameraPosition);
  qt qPitch = HMM_QFromAxisAngle_RH((v3){-1, 0, 0}, pitch);
  qt qYaw = HMM_QFromAxisAngle_RH((v3){0, -1, 0}, yaw);
  m4 rotation = HMM_QToM4(HMM_NormQ(HMM_MulQ(qYaw, qPitch)));  

  if (!isZeroV3(direction))
  {
    direction = HMM_MulV3F(HMM_NormV3(direction), deltaTime * camera->speed);
    entity->transform.position = HMM_AddV3(cameraPosition, direction);
  }
  
  camera->pitch = pitch;
  camera->yaw = yaw;
  camera->view = HMM_InvGeneralM4(HMM_MulM4(translation, rotation));
  camera->projection = HMM_Perspective_RH_NO(camera->fov, camera->aspect, 0.01, 1000);
}

#define AttachComponentCase(entityType, structType)\
case entityType: {\
  ##structType## *entityComponent = (##structType## *)component;\
  entity->component.data = entityComponent;\
  entityComponent->entity = entity;\
} break

void AttachComponent(Entity *entity, void *component)
{
  switch (entity->componentType)
  {
    AttachComponentCase(COMPONENT_MODEL, Model);
    AttachComponentCase(COMPONENT_CAMERA, Camera);
    default: break;
  }
}
