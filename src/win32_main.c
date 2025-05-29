i32 WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
  hInstPrev; cmdline; cmdshow; // To avoid stupid warnings
  
  Arena arena;
  Init(&arena, VirtualAlloc(NULL, 2*GB, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE), 2*GB);
    
  Win32Context *win32 = (Win32Context *)Alloc(&arena, sizeof(Win32Context));
  win32->arena = &arena;
  
  HWND window = CreateOpenGLContext(hInst, win32);
  
  GameInputs *inputs = (GameInputs *)Alloc(&arena, sizeof(GameInputs));
  *inputs = InitGameInputs(&arena, window);
  win32->inputs = inputs;
          
  Shader *shaders = (Shader *)Alloc(&arena, 4 * sizeof(Shader));
  shaders[0] = CompileShader(&arena, "data/shaders/vertex.glsl", GL_VERTEX_SHADER);
  shaders[1] = CompileShader(&arena, "data/shaders/fragment.glsl", GL_FRAGMENT_SHADER);
  
#if DEBUG
  shaderRuntimeCompilationFatal = false;
#endif

  u32 pipelinesCount = 1;
  Pipeline *pipelines = (Pipeline *)Alloc(&arena, pipelinesCount * sizeof(Pipeline));
  pipelines[0] = CreatePipeline(&arena, 2, &shaders[0]);

  u32 modelsCount = 1;
  Model *models = (Model *)Alloc(&arena, modelsCount * sizeof(Model));
  models[0] = LoadModel(&arena, "data/models/helmet.model");
  models[0].baseColorMap = LoadTexture("data/textures/helmet_base_color.png");
  models[0].metallicRoughnessMap = LoadTexture("data/textures/helmet_metallic_roughness.png");
  models[0].normalMap = LoadTexture("data/textures/helmet_normal.png");
  models[0].ambientOcclusionMap = LoadTexture("data/textures/helmet_ambient_occlusion.png");
    
  Camera *camera = (Camera *)Alloc(&arena, sizeof(Camera));
  camera->speed = 0.0005f;
  camera->fov = 0.25f;
  camera->yaw = 0.5f;
  
  u32 entitiesCount = 2;
  Entity *entities = (Entity *)Alloc(&arena, entitiesCount * sizeof(Entity));
  entities[0] = (Entity) {
    .transform = (Transform) { .position = (v3){0.092f,0.116f,0.130f} },
    .componentType = COMPONENT_CAMERA,
    .component.camera = camera,
  };
  entities[1] = (Entity) {
    .transform = (Transform) {
      .position = (v3){0,0,0.5f},
      .rotation = (v3){0,0.5f,0},
      .scale = (v3){1,1,1}
    },
    .componentType = COMPONENT_MODEL,
    .component.model = &models[0],
  };
  
  for (u32 i = 0; i < entitiesCount; ++i)
  {
    Entity *entity = &entities[i];
    AttachComponent(entity, entity->component.data);
    UpdateEntityCenteredTransformMatrix(entity);        
    entity->normalMatrix = ComputeNormalMatrix(entity->transformMatrix);
  }
      
#if DEBUG && DEBUG_IMGUI
  InitImGui(window);
  
  ImGuiDebugData imguiDebugData = (ImGuiDebugData) {
    .arena = &arena,
    .win32 = win32,
    .modelsCount = &modelsCount,
    .models = models,
    .entitiesCount = &entitiesCount,
    .entities = entities,
  };
  
  DEVMODE devMode = (DEVMODE) { .dmSize = sizeof(DEVMODE) };
  EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode);
  u32 desiredDelay = 1 / devMode.dmDisplayFrequency;
#endif
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  wglSwapIntervalEXT(1);
  ShowWindow(window, SW_MAXIMIZE);
    
  PerfCounter deltaCounter = InitPerfCounter();
  StartPerfCounter(&deltaCounter);
  for (;;)
  {
    f32 deltaTime = GetDeltaTime(&deltaCounter);
#if DEBUG
    ShadersHotReloading(&arena, pipelines, pipelinesCount);
#endif

    inputs->isKeyPressed = 0;
    
    MSG message;
    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
    {      
      if (!win32->quitting)
      {
        TranslateMessage(&message);
        DispatchMessage(&message);
        win32->quitting = inputs->isKeyPressed && inputs->buttons.escape;
      }
      else
      {
        Quit(window, win32);
        return 0;
      }
    }
      
    camera->aspect = win32->viewportAspect;
    ApplyNonLinearMouseFiltering(inputs);
    UpdateCamera(&entities[0], *inputs, deltaTime);
    
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    {
      GLint vertexShader = pipelines[0].shaders[0].program;
      glBindProgramPipeline(pipelines[0].program);
  
      for (u32 i = 0; i < entitiesCount; ++i)
      {
        Entity entity = entities[i];
        if (entity.componentType == COMPONENT_MODEL)
        {
          
          Model model = *entity.component.model;
          m4 mvp = ComputeModelViewProjectionMatrix(entity.transformMatrix, camera);
          v3 cameraPosition = camera->entity->transform.position;
          cameraPosition.Z *= -1; //ISSUE: It seems that there is a handedness issue in my OpenGL / GLSL logic that I should investigate
          
          glProgramUniformMatrix4fv(vertexShader, 0, 1, false, (GLfloat *)&mvp);
          glProgramUniformMatrix3fv(vertexShader, 1, 1, false, (GLfloat *)&entity.normalMatrix);
          glProgramUniform2fv(vertexShader, 2, 1, (GLfloat *)&model.uvScale);
          glProgramUniform2fv(vertexShader, 3, 1, (GLfloat *)&model.uvOffset);
          glProgramUniform3fv(vertexShader, 4, 1, (GLfloat *)&cameraPosition);
          glProgramUniform4fv(vertexShader, 5, 1, (GLfloat *)&model.baseColor);
          glProgramUniform1fv(vertexShader, 6, 1, (GLfloat *)&model.metallicFactor);
          glProgramUniform1fv(vertexShader, 7, 1, (GLfloat *)&model.roughnessFactor);
  
          glBindTextureUnit(0, model.baseColorMap);
          glBindTextureUnit(1, model.metallicRoughnessMap);
          glBindTextureUnit(2, model.normalMap);
          glBindTextureUnit(3, model.ambientOcclusionMap);
          
          glBindVertexArray(model.vao);
    
          //TODO: Updated models indices for GL_TRIANGLE_STRIP (requires meshopt impl)
          glDrawElements(GL_TRIANGLES, model.indicesCount, GL_UNSIGNED_SHORT, NULL);
        }
      }
    }
    
        
#if DEBUG && DEBUG_IMGUI
    imguiDebugData.frameDelay = deltaTime - desiredDelay;
    UpdateImGui(&imguiDebugData);    
    RenderImGui();
#endif
    SwapBuffers(win32->dc);
  }
}  
