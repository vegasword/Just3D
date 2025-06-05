i32 WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
  hInstPrev; cmdline; cmdshow; // To avoid stupid MSVC warnings
  
  Arena arena;
  Init(&arena, VirtualAlloc(NULL, 2*GB, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE), 2*GB);
    
  Win32Context *win32 = (Win32Context *)Alloc(&arena, sizeof(Win32Context));
  win32->arena = &arena;
  
  HWND window = CreateOpenGLContext(hInst, win32);

#if DEBUG
  InitDebugConsole();
#endif
  
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
  glBindProgramPipeline(pipelines[0].program);

  u32 modelsCount = 1;
  Model *models = (Model *)Alloc(&arena, modelsCount * sizeof(Model));
  models[0] = LoadModel(&arena, "data/models/helmet.model");
  models[0].baseColorMap = LoadTexture("data/textures/helmet_base_color.png");
  models[0].metallicRoughnessMap = LoadTexture("data/textures/helmet_metallic_roughness.png");
  models[0].normalMap = LoadTexture("data/textures/helmet_normal.png");
  models[0].ambientOcclusionMap = LoadTexture("data/textures/helmet_ambient_occlusion.png");
  models[0].transform = (Transform) {
    .position = (v3){0,0,0},
    .rotation = (v3){0,0.5f,0},
    .scale = (v3){1,1,1}
  };
  
  for (u32 i = 0; i < modelsCount; ++i)
  {
    UpdateCenteredModelTransformMatrix(&models[i]);        
  }
      
  Camera *camera = (Camera *)Alloc(&arena, sizeof(Camera));
  camera->transform = (Transform) { .position = (v3){0.092f,0.116f,-0.5f} },
  camera->speed = 0.0005f;
  camera->fov = 0.25f;
  camera->yaw = 0.5f;
  
  DirectLights *directLights = (DirectLights *)Alloc(&arena, sizeof(DirectLights));
  
  u32 punctualsBufferBaseOffset = ALIGN_UP(sizeof(u32), 16);
  directLights->punctualsCount = 3;
  u32 punctualsBufferSize = directLights->punctualsCount * sizeof(PunctualLight);
  directLights->punctualsBufferAlignedSize = ALIGN_UP(punctualsBufferSize, 16);
  
  directLights->punctuals = (PunctualLight *)Alloc(&arena, punctualsBufferSize);
  directLights->punctuals[0] = CreateSpotLight((v3){0,1,0},(v3){0,-1,0},(v3){0,1,0}, 5, 1.5f, 0.05f, 0.15f);
  directLights->punctuals[1] = CreateSpotLight((v3){-1,0,0},(v3){1,0,0},(v3){1,0,0}, 5, 1.5f, 0.05f, 0.15f);
  directLights->punctuals[2] = CreateSpotLight((v3){1,0,0},(v3){-1,0,0},(v3){0,0,1}, 5, 1.5f, 0.05f, 0.15f);
      
  u32 modelUniformsObject = CreateShaderBufferObject(4, sizeof(ModelUniforms), GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
  ModelUniforms *modelUniforms = New(&arena, ModelUniforms);
  
  directLights->puntualsBufferObject = CreateShaderBufferObject(5, punctualsBufferBaseOffset + punctualsBufferSize, GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
  glNamedBufferSubData(directLights->puntualsBufferObject, 0, sizeof(u32), &directLights->punctualsCount);
  glNamedBufferSubData(directLights->puntualsBufferObject, punctualsBufferBaseOffset, directLights->punctualsBufferAlignedSize, directLights->punctuals);
      
#if DEBUG && DEBUG_IMGUI
  InitImGui(window);
  
  ImGuiDebugData imguiDebugData = (ImGuiDebugData) {
    .arena = &arena,
    .win32 = win32,
    .modelsCount = modelsCount,
    .models = models,
    .directLights = directLights,
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
      for (u32 i = 0; i < modelsCount; ++i)
      {
        Model model = models[i];
        m4 modelTransformMatrix = model.transformMatrix;
        m3 normalMatrix = ComputeNormalMatrix(modelTransformMatrix);
        
        modelUniforms->mvp = ComputeModelViewProjectionMatrix(modelTransformMatrix, camera);
        modelUniforms->modelMatrix = modelTransformMatrix;
        modelUniforms->normalMatrixFirstColumn = (v4){ .XYZ = normalMatrix.Columns[0] };
        modelUniforms->normalMatrixSecondColumn = (v4){ .XYZ = normalMatrix.Columns[1] };
        modelUniforms->normalMatrixThirdColumn = (v4){ .XYZ = normalMatrix.Columns[2] };
        modelUniforms->cameraPosition = (v4){ .XYZ = camera->transform.position };
        modelUniforms->baseColor = model.data.baseColor;
        modelUniforms->uvScale = model.data.uvScale;
        modelUniforms->uvOffset = model.data.uvOffset;
        modelUniforms->metallicFactor = model.data.metallicFactor;
        modelUniforms->roughnessFactor = model.data.roughnessFactor;
        
        glNamedBufferSubData(modelUniformsObject, 0, sizeof(ModelUniforms), modelUniforms);
          
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

LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{  
  Win32Context *context;
  if (msg == WM_CREATE)
  {
    CREATESTRUCT *createStruct = (CREATESTRUCT *)lParam;
    context = (Win32Context *)createStruct->lpCreateParams;
    SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)context);
  }
  else
  {
    context = (Win32Context *)GetWindowLongPtr(window, GWLP_USERDATA);
  }
  
  switch (msg)
  {    
    case WM_INPUT: {
      HandleRawInputs(lParam, context->inputs);
    } break;

    case WM_SIZE: {
      v2 viewport = context->viewport = UpdateViewportDimensions(window);
      context->viewportAspect = viewport.Width / viewport.Height;
    } break;
    
    case WM_SETFOCUS: {
      POINT cliCenter = GetClientCenter(window);
      RECT cursorClipBounds = (RECT) { .left = cliCenter.x, .top = cliCenter.y, .right = cliCenter.x, .bottom = cliCenter.y };
      ClipCursor(&cursorClipBounds);
      ShowCursor(false);
    } break;
    
    case WM_KILLFOCUS: {
      ClipCursor(NULL);
      ShowCursor(true);
      memset(&context->inputs->buttons, 0, sizeof(context->inputs->buttons));
    } break;

    case WM_CLOSE:
    case WM_QUIT:
    case WM_DESTROY: {
      Quit(window, context);
    } break;
    
    default: return DefWindowProc(window, msg, wParam, lParam);
  }
  
  return 0;
}

#if DEBUG && DEBUG_IMGUI
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ImGuiWindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
  ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam);
  
  Win32Context *context;
  if (msg == WM_CREATE)
  {
    CREATESTRUCT *createStruct = (CREATESTRUCT *)lParam;
    context = (Win32Context *)createStruct->lpCreateParams;
    SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)context);
  }
  else
  {
    context = (Win32Context *)GetWindowLongPtr(window, GWLP_USERDATA);
  }
  
  switch (msg)
  {    
    case WM_INPUT: {
      if (!context->imguiDebugging || (context->imguiDebugging && igIsMouseDown_Nil(ImGuiMouseButton_Right)))
      {
        HandleRawInputs(lParam, context->inputs);
      }
      else
      {
        memset(&context->inputs->buttons, 0, sizeof(context->inputs->buttons));
      }
    } break;
    
    case WM_SIZE: {
      v2 viewport = context->viewport = UpdateViewportDimensions(window);
      context->viewportAspect = viewport.Width / viewport.Height;
    } break;

    case WM_SETFOCUS: {
      if (!context->imguiDebugging)
      {
        POINT cliCenter = GetClientCenter(window);
        RECT cursorClipBounds = (RECT) { .left = cliCenter.x, .top = cliCenter.y, .right = cliCenter.x, .bottom = cliCenter.y };
        ClipCursor(&cursorClipBounds);
        ShowCursor(false);
      }
    } break;
    
    case WM_KILLFOCUS: {
      if (!context->imguiDebugging)
      {
        ClipCursor(NULL);
        ShowCursor(true);
        memset(&context->inputs->buttons, 0, sizeof(context->inputs->buttons));
      }
    } break;

    case WM_CLOSE:
    case WM_QUIT:
    case WM_DESTROY: {
      Quit(window, context);
    } break;
    
    default: return DefWindowProc(window, msg, wParam, lParam);
  }
  
  return 0;
}
#endif
