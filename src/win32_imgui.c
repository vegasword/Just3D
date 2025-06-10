typedef struct  {
  ImGuiContext *ctx;
  ImGuiIO *io;
} ImGui;

typedef struct {  
  Arena *arena;
  Win32Context *win32;
  f32 frameDelay;
  u32 modelsCount;
  Camera *camera;
  Model *models;
  DirectLights *directLights;
} ImGuiDebugData;

void InitImGui(HWND window)
{
  ImGui imgui = { 0 };
  imgui.ctx = igCreateContext(NULL);
  imgui.ctx->Style.ItemSpacing = (ImVec2){0};
  imgui.ctx->Style.ItemInnerSpacing = (ImVec2){0};
  imgui.io = igGetIO();
  imgui.io->IniFilename = NULL;
  ImGui_ImplWin32_InitForOpenGL(window);
  ImGui_ImplOpenGL3_Init(NULL);
  igStyleColorsDark(NULL);
}

void UpdateImGui(ImGuiDebugData *data)
{
  Win32Context *win32 = data->win32;
  GameInputs *inputs = win32->inputs;
  
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplWin32_NewFrame();
  igNewFrame();
  
  if (igGetKeyPressedAmount(ImGuiKey_F1, 0.5f, 0.1f))
  {
    win32->imguiDebugging = !win32->imguiDebugging;
    if (win32->imguiDebugging)
    {
      ClipCursor(NULL);
      ShowCursor(true);
    }
    else
    {
      POINT cliCenter = GetClientCenter(WindowFromDC(win32->dc));
      RECT cursorClipBounds = (RECT) { .left = cliCenter.x, .top = cliCenter.y, .right = cliCenter.x, .bottom = cliCenter.y };
      ClipCursor(&cursorClipBounds);
      ShowCursor(false);
    }
  }
  
  if (win32->imguiDebugging)
  {
    if (igBegin("DEBUG MODE", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
    {
      igSetWindowPos_Vec2((ImVec2){0, 0}, ImGuiCond_Always);
      igSetWindowSize_Vec2((ImVec2){win32->viewport.Width / 4, win32->viewport.Height}, ImGuiCond_Once);
      
      igText("%.1fms", data->frameDelay);
    
      ImGuiTabBarFlags tab_bar_flags = 0;
      if (igBeginTabBar("DebugTabBar", tab_bar_flags))
      {      
        if (igBeginTabItem("Entities", NULL, 0))
        {
          Camera* camera = data->camera;
          igPushID_Ptr(camera);
          if (igTreeNodeEx_StrStr("##", 0, "%lx Camera", camera))
          {              
            igText("Position: "V3_FORMAT, V3_ARGS(camera->transform.position));            
            igText("Pitch: %.2f", camera->pitch);
            igText("Yaw: %.2f", camera->yaw);
            
            i32 fovDegree = (i32)(camera->fov * HMM_TurnToDeg);
            if (igDragInt("FOV", &fovDegree, 1, 30, 120, "%d", ImGuiInputTextFlags_None))
            {
              camera->fov = (f32)(fovDegree * HMM_DegToTurn);
            }
            
            igTreePop();
          }
          igPopID();
          
          for (u32 i = 0; i < data->modelsCount; ++i)
          {
            Model *model = &data->models[i];
            ModelData modelData = model->data;
            
            igPushID_Ptr(model);
                        
            if (igTreeNodeEx_StrStr("##", 0, "%lx Model", model))
            {              
              igSeparatorText("Transform");

              Transform *transform = &model->transform;              
              if (igDragFloat3("Position", (f32 *)&transform->position, 0.01f, 0, 0, "%f", ImGuiInputTextFlags_CharsDecimal))
              {
                UpdateCenteredModelTransformMatrix(model);
              }
      
              if (igDragFloat3("Rotation", (f32 *)&transform->rotation, 0.01f, 0, 0, "%f", ImGuiInputTextFlags_CharsDecimal))
              {
                UpdateCenteredModelTransformMatrix(model);
              }
      
              if (igDragFloat3("Scale", (f32 *)&transform->scale, 0.01f, 0.001f, FLT_MAX, "%f", ImGuiInputTextFlags_CharsDecimal))
              {
                UpdateCenteredModelTransformMatrix(model);
              }

              if (igButton("Copy transform", (ImVec2){0}))
              {
                char transformString[512];
                
                v3 position = transform->position;
                v3 rotation = transform->rotation;
                v3 scale = transform->scale;
                
                igImFormatString(transformString, sizeof(transformString),
                                 "(Transform){ (v3){%.3f,%.3f,%.3f}, (v3){%.3f,%.3f,%.3f}, (v3){%.3f,%.3f,%.3f} }",
                                 position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, scale.X, scale.Y, scale.Z);
                
                igSetClipboardText(transformString);
              }
              
              igSeparatorText("Material properties");
              
              igText("Base color: "V4_FORMAT, V4_ARGS(modelData.baseColor));
              igText("Metallic factor: %.2f ", modelData.metallicFactor);
              igText("Roughness factor: %.2f ", modelData.roughnessFactor);
              
              igSeparatorText("Textures properties");
              
              igText("UV Offset: "V2_FORMAT, V2_ARGS(modelData.uvOffset));
              igText("UV Scale: "V2_FORMAT, V2_ARGS(modelData.uvScale));

              igTreePop();
            }
            igPopID();
          }
          
          DirectLights *directLights = data->directLights;
          u32 punctualAlignedSize = ALIGN_UP(sizeof(PunctualLight), 16);
          for (u32 i = 0; i < directLights->punctualsCount; ++i)
          {
            PunctualLight *punctual = &directLights->punctuals[i];
            u32 punctualOffset = directLights->punctualsBaseOffset + (i * punctualAlignedSize);
            bool isSpotlight = punctual->isSpotlight;
            
            igPushID_Ptr(punctual);
            
            if (igTreeNodeEx_StrStr("##", 0, "%lx %s", punctual, isSpotlight ? "Spotlight" : "Point Light"))
            {              
              igSeparatorText("Transform");


              if (igDragFloat3("Position", (f32 *)&punctual->position, 0.01f, 0, 0, "%f", ImGuiInputTextFlags_CharsDecimal))
              {
                glNamedBufferSubData(directLights->bufferObject, punctualOffset, punctualAlignedSize, punctual);
              }
      
              if (igDragFloat3("Direction", (f32 *)&punctual->direction, 0.01f, 0, 0, "%f", ImGuiInputTextFlags_CharsDecimal))
              {
                glNamedBufferSubData(directLights->bufferObject, punctualOffset, punctualAlignedSize, punctual);
              }
      
              if (igColorEdit3("Color", (f32 *)&punctual->color, 0))
              {
                glNamedBufferSubData(directLights->bufferObject, punctualOffset, punctualAlignedSize, punctual);
              }
              
              igSeparatorText("Common properties");
              
              if (igDragFloat("Intensity", &punctual->intensity, 0.01f, 0.001f, 100, "%f", ImGuiInputTextFlags_CharsDecimal))
              {
                glNamedBufferSubData(directLights->bufferObject, punctualOffset, punctualAlignedSize, punctual);
              }

              f32 falloffRadius = 1.f / punctual->inverseFalloffRadius;
              if (igDragFloat("Falloff radius", &falloffRadius, 0.01f, 0.001f, 100, "%f", ImGuiInputTextFlags_CharsDecimal))
              {
                punctual->inverseFalloffRadius = 1.f / falloffRadius;
                glNamedBufferSubData(directLights->bufferObject, punctualOffset, punctualAlignedSize, punctual);
              }
              
              igSeparatorText("Spotlight properties");
              if (igCheckbox("Spotlight", (bool *)&isSpotlight))
              {
                punctual->isSpotlight = isSpotlight ? 1.f : 0.f;
                glNamedBufferSubData(directLights->bufferObject, punctualOffset, punctualAlignedSize, punctual);
              }
              
              igBeginDisabled(!isSpotlight);
              
              f32 innerAngle = HMM_ACosF(punctual->cosOuterAngle + (1.0f / punctual->spotScale));
              f32 outerAngle = HMM_ACosF(punctual->cosOuterAngle);
              
              if (igDragFloat("Inner angle", &innerAngle, 0.01f, 0.01f, outerAngle, "%.2f", ImGuiInputTextFlags_CharsDecimal))
              {                
                punctual->cosOuterAngle = HMM_CosF(outerAngle);
                punctual->spotScale = 1.0f / fmaxf(HMM_CosF(innerAngle) - punctual->cosOuterAngle, 0.001f),
                glNamedBufferSubData(directLights->bufferObject, punctualOffset, punctualAlignedSize, punctual);
              }
              
              if (igDragFloat("Outer angle", &outerAngle, 0.01f, innerAngle, 0.5f, "%.2f", ImGuiInputTextFlags_CharsDecimal))
              {
                punctual->cosOuterAngle = HMM_CosF(outerAngle);
                punctual->spotScale = 1.0f / fmaxf(HMM_CosF(innerAngle) - punctual->cosOuterAngle, 0.001f),
                glNamedBufferSubData(directLights->bufferObject, punctualOffset, punctualAlignedSize, punctual);
              }
                            
              igText("Scale: %f", punctual->spotScale);

              igEndDisabled();
              
              igTreePop();
            }
            igPopID();
          }
          igEndTabItem();          
        }
        
        if (igBeginTabItem("Settings", NULL, 0))
        {
          igSeparatorText("Mouse");
          
          igText("Last mouse delta: "V2_FORMAT, V2_ARGS(inputs->lastMouseDelta));
          igSliderFloat("Mouse sensitivity", &inputs->mouseSensitivity, 0.0001f, 0.001f, "%.5f", ImGuiSliderFlags_Logarithmic);
          
          igEndTabItem();
        }
        
        igEndTabBar();
      }
    }
    igEnd();
  }
}

void RenderImGui(void)
{
  igRender();
  ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
