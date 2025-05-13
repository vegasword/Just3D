i32 WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
  hInstPrev; cmdline; cmdshow;
  
  Arena arena;
  Init(&arena, VirtualAlloc(NULL, 2*GB, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE), 2*GB);
    
  Win32Context *win32 = (Win32Context *)Alloc(&arena, sizeof(Win32Context));
  win32->arena = &arena;
  
  HWND window = CreateOpenGLContext(hInst, win32);
  
  GameInputs *inputs = (GameInputs *)Alloc(&arena, sizeof(GameInputs));
  *inputs = InitGameInputs(&arena, window);
  win32->inputs = inputs;
          
  Shader *shaders = (Shader *)Alloc(&arena, 4 * sizeof(Shader));
  shaders[0] = CompileShader(&arena, "data/shaders/model.vert", GL_VERTEX_SHADER);
  shaders[1] = CompileShader(&arena, "data/shaders/model.frag", GL_FRAGMENT_SHADER);
  
#if DEBUG
  shaderCompilationFatal = false;
#endif

  u32 pipelinesCount = 1;
  Pipeline *pipelines = (Pipeline *)Alloc(&arena, pipelinesCount * sizeof(Pipeline));
  pipelines[0] = SetupPipeline(&arena, 2, &shaders[0]);

  u32 texturesCount = 1;
  u32 *textures = (u32 *)Alloc(&arena, texturesCount * sizeof(u32));
  textures[0] = CreateTexture("data/textures/crt.png");
    
  u32 modelsCount = 1;
  Model *models = (Model *)Alloc(&arena, modelsCount * sizeof(Model));
  models[0] = CreateModel(&arena, "data/models/crt.model");
      
  Camera *camera = (Camera *)Alloc(&arena, sizeof(Camera));
  camera->fov = 0.25;
  camera->yaw = 0.5;
  camera->transform = New(&arena, Transform);
  camera->view = New(&arena, m4);
  camera->projection = New(&arena, m4);
  
  u32 entitiesCount = 1;
  Entity *entities = (Entity *)Alloc(&arena, entitiesCount * sizeof(Entity));
  entities[0] = (Entity) {
    .transform = New(&arena, Transform),
    .model = &models[0],
    .texture = textures[0]
  };
  *entities[0].transform =  (Transform) { .position = (v3){-0.15,-0.15,1}, .rotation = (v3){0,0.5,0}, .scale = (v3){1,1,1} };
  
  for (u32 i = 0; i < entitiesCount; ++i)
  {
    entities[i].modelMatrix = (m4 *)Alloc(&arena, sizeof(m4));
    UpdateEntityCenteredModelMatrix(&entities[i]);        
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
    .texturesCount = &texturesCount,
    .textures = textures,
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
        Destroy(&arena);
        return 0;
      }
    }
      
    camera->aspect = win32->viewportAspect;
    ApplyNonLinearMouseFiltering(inputs);    
    UpdateCamera(camera, *inputs, deltaTime);
    
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    {
      GLint vertexShader = pipelines[0].shaders[0].program;
  
      for (u32 i = 0; i < entitiesCount; ++i)
      {
        Entity *entity = &entities[i];
        if (entity->model)
        {
          Model model = *entity->model;
          glBindProgramPipeline(pipelines[0].program);
      
          m4 mvp = ComputeMVP(entity->modelMatrix, camera);
          glProgramUniformMatrix4fv(vertexShader, 0, 1, false, (GLfloat *)&mvp);
          glProgramUniform2fv(vertexShader, 1, 1, (GLfloat *)&model.uvScale);
          glProgramUniform2fv(vertexShader, 2, 1, (GLfloat *)&model.uvOffset);
      
          glBindTextureUnit(0, entity->texture);
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
    i32 swapped = SwapBuffers(win32->dc);
    assert(swapped);
#else
    SwapBuffers(win32->dc);
#endif
  }
}  
