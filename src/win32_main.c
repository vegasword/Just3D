i32 WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
  hInstPrev; cmdline; cmdshow; // To avoid stupid MSVC warnings
  
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
  models[0].transform = (Transform) {
    .position = (v3){0,0,0.5f},
    .rotation = (v3){0,0.5f,0},
    .scale = (v3){1,1,1}
  };
  
  for (u32 i = 0; i < modelsCount; ++i)
  {
    UpdateCenteredModelTransformMatrix(&models[i]);        
  }
      
  Camera *camera = (Camera *)Alloc(&arena, sizeof(Camera));
  camera->transform = (Transform) { .position = (v3){0.092f,0.116f,0.130f} },
  camera->speed = 0.0005f;
  camera->fov = 0.25f;
  camera->yaw = 0.5f;
  
  u32 uniformBufferObject = CreateUniformBufferObject(4, sizeof(UniformBuffer), GL_DYNAMIC_DRAW);
  UniformBuffer *uniformBuffer = New(&arena, UniformBuffer);
      
#if DEBUG && DEBUG_IMGUI
  InitImGui(window);
  
  ImGuiDebugData imguiDebugData = (ImGuiDebugData) {
    .arena = &arena,
    .win32 = win32,
    .modelsCount = &modelsCount,
    .models = models,
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
    UpdateCamera(camera, *inputs, deltaTime);
    
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    {      
      glBindProgramPipeline(pipelines[0].program);
      
      for (u32 i = 0; i < modelsCount; ++i)
      {
        Model model = models[i];
        m4 modelTransformMatrix = model.transformMatrix;
        m3 normalMatrix = ComputeNormalMatrix(modelTransformMatrix);
        
        uniformBuffer->mvp = ComputeModelViewProjectionMatrix(modelTransformMatrix, camera);
        uniformBuffer->modelMatrix = modelTransformMatrix;
        uniformBuffer->normalMatrixFirstColumn = (v4){ .XYZ = normalMatrix.Columns[0] };
        uniformBuffer->normalMatrixSecondColumn = (v4){ .XYZ = normalMatrix.Columns[1] };
        uniformBuffer->normalMatrixThirdColumn = (v4){ .XYZ = normalMatrix.Columns[2] };
        uniformBuffer->cameraPosition = (v4){ .XYZ = camera->transform.position };
        uniformBuffer->baseColor = model.baseColor;
        uniformBuffer->uvScale = model.uvScale;
        uniformBuffer->uvOffset = model.uvOffset;
        uniformBuffer->metallicFactor = model.metallicFactor;
        uniformBuffer->roughnessFactor = model.roughnessFactor;
        
        glNamedBufferSubData(uniformBufferObject, 0, sizeof(UniformBuffer), uniformBuffer);
          
        glBindTextureUnit(0, model.baseColorMap);
        glBindTextureUnit(1, model.metallicRoughnessMap);
        glBindTextureUnit(2, model.normalMap);
        glBindTextureUnit(3, model.ambientOcclusionMap);
        
        glBindVertexArray(model.vao);
  
        //TODO: GL_TRIANGLE_STRIP rendering (requires meshopt impl in gltf2custom)
        glDrawElements(GL_TRIANGLES, model.indicesCount, GL_UNSIGNED_SHORT, NULL);
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
