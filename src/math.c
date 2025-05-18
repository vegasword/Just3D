bool isZeroV3(v3 value)
{
  f32 tolerance = 0.000001f;
  return fabs(value.X) < tolerance && fabs(value.Y) < tolerance && fabs(value.Z) < tolerance;
}

v3 ClampV3(v3 min, v3 value, v3 max)
{
  return (v3) { .X = HMM_Clamp(min.X, value.X, max.X), .Y = HMM_Clamp(min.Y, value.Y, max.Y), .Z = HMM_Clamp(min.Z, value.Z, max.Z) };
}

qt GetOrientation(v3 rotation)
{
  qt pitch = HMM_QFromAxisAngle_RH((v3){1, 0, 0}, rotation.X);
  qt yaw = HMM_QFromAxisAngle_RH((v3){0, 1, 0}, rotation.Y);
  qt roll = HMM_QFromAxisAngle_RH((v3){0, 0, 1}, rotation.Z);
  return HMM_NormQ(HMM_MulQ(roll, HMM_MulQ(yaw, pitch)));
}

m4 GetRotationMatrix(v3 rotation)
{
  return HMM_QToM4(GetOrientation(rotation));
}

m4 GetTransformMatrix(Transform transform)
{
  m4 translate = HMM_Translate(transform.position);
  m4 rotation = GetRotationMatrix(transform.rotation);
  m4 scale = HMM_Scale(transform.scale);
  return HMM_MulM4(HMM_MulM4(translate, rotation), scale);
}

m4 GetPivotedTransformMatrix(Transform transform, v3 pivot)
{  
  m4 translateToPivot = HMM_Translate(pivot);
  m4 translateFromPivot = HMM_Translate(HMM_MulV3F(pivot, -1));
  
  m4 translate = HMM_Translate(transform.position);
  m4 rotation = GetRotationMatrix(transform.rotation);
  m4 scale = HMM_Scale(HMM_MulV3F(transform.scale, 2));
  
  return HMM_MulM4(translateToPivot, HMM_MulM4(translate, HMM_MulM4(rotation, HMM_MulM4(scale, translateFromPivot))));
}

m4 ComputeModelViewProjectionMatrix(m4 modelMatrix, Camera *camera)
{
  return HMM_MulM4(camera->projection, HMM_MulM4(camera->view, modelMatrix));
}

m3 ComputeNormalMatrix(m4 modelMatrix)
{
  m3 modelMatrix3 = (m3) {
    .Columns = {
      [0] = modelMatrix.Columns[0].XYZ,
      [1] = modelMatrix.Columns[1].XYZ,
      [2] = modelMatrix.Columns[2].XYZ
    }
  };
  return HMM_TransposeM3(HMM_InvGeneralM3(modelMatrix3));
}
