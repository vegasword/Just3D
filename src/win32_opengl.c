#if DEBUG && DEBUG_IMGUI
LRESULT CALLBACK ImGuiWindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
#else
LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

#define PFNGL(name) PFNGL##name##PROC
#define PFNWGL(name) PFNWGL##name##PROC

#define GL_FUNC(X) \
    X(PFNGL(CREATEBUFFERS),             glCreateBuffers             )\
    X(PFNGL(NAMEDBUFFERSTORAGE),        glNamedBufferStorage        )\
\
    X(PFNGL(CREATEVERTEXARRAYS),        glCreateVertexArrays        )\
    X(PFNGL(VERTEXARRAYATTRIBBINDING),  glVertexArrayAttribBinding  )\
    X(PFNGL(VERTEXARRAYATTRIBFORMAT),   glVertexArrayAttribFormat   )\
    X(PFNGL(VERTEXARRAYBINDINGDIVISOR), glVertexArrayBindingDivisor )\
    X(PFNGL(NAMEDBUFFERSUBDATA),        glNamedBufferSubData        )\
    X(PFNGL(VERTEXARRAYATTRIBIFORMAT),  glVertexArrayAttribIFormat  )\
    X(PFNGL(VERTEXARRAYVERTEXBUFFER),   glVertexArrayVertexBuffer   )\
    X(PFNGL(VERTEXARRAYELEMENTBUFFER),  glVertexArrayElementBuffer  )\
    X(PFNGL(ENABLEVERTEXARRAYATTRIB),   glEnableVertexArrayAttrib   )\
    X(PFNGL(BINDVERTEXARRAY),           glBindVertexArray           )\
\
    X(PFNGL(CREATESHADERPROGRAMV),      glCreateShaderProgramv      )\
    X(PFNGL(GETPROGRAMIV),              glGetProgramiv              )\
    X(PFNGL(GETPROGRAMINFOLOG),         glGetProgramInfoLog         )\
    X(PFNGL(USEPROGRAM),                glUseProgram                )\
    X(PFNGL(DELETEPROGRAM),             glDeleteProgram             )\
\
    X(PFNGL(CREATEPROGRAMPIPELINES),    glCreateProgramPipelines    )\
    X(PFNGL(USEPROGRAMSTAGES),          glUseProgramStages          )\
    X(PFNGL(BINDPROGRAMPIPELINE),       glBindProgramPipeline       )\
    X(PFNGL(DELETEPROGRAMPIPELINES),    glDeleteProgramPipelines    )\
\
    X(PFNGL(CREATETEXTURES),            glCreateTextures            )\
    X(PFNGL(BINDTEXTUREUNIT),           glBindTextureUnit           )\
    X(PFNGL(TEXTUREPARAMETERI),         glTextureParameteri         )\
    X(PFNGL(TEXTURESTORAGE2D),          glTextureStorage2D          )\
    X(PFNGL(TEXTURESUBIMAGE2D),         glTextureSubImage2D         )\
    X(PFNGL(GENERATETEXTUREMIPMAP),     glGenerateTextureMipmap     )\
\
    X(PFNGL(PROGRAMUNIFORM2UIV),        glProgramUniform2uiv        )\
    X(PFNGL(PROGRAMUNIFORM1FV),         glProgramUniform1fv         )\
    X(PFNGL(PROGRAMUNIFORM2FV),         glProgramUniform2fv         )\
    X(PFNGL(PROGRAMUNIFORM3FV),         glProgramUniform3fv         )\
    X(PFNGL(PROGRAMUNIFORM4FV),         glProgramUniform4fv         )\
    X(PFNGL(PROGRAMUNIFORMMATRIX3FV),   glProgramUniformMatrix3fv   )\
    X(PFNGL(PROGRAMUNIFORMMATRIX4FV),   glProgramUniformMatrix4fv   )\
\
    X(PFNGL(DRAWARRAYSINSTANCED),       glDrawArraysInstanced       )\
\
    X(PFNGL(DEBUGMESSAGECALLBACK),      glDebugMessageCallback      )

#define X(type, name) static type name;
GL_FUNC(X)
#undef X

static PFNWGL(SWAPINTERVALEXT) wglSwapIntervalEXT = NULL;
static PFNWGL(CHOOSEPIXELFORMATARB) wglChoosePixelFormatARB = NULL;
static PFNWGL(CREATECONTEXTATTRIBSARB) wglCreateContextAttribsARB = NULL;

#if DEBUG
static bool shaderCompilationFatal = true;
#endif

static u32 shaderTypesBit[] = {
  [GL_VERTEX_SHADER] = GL_VERTEX_SHADER_BIT,
  [GL_FRAGMENT_SHADER] = GL_FRAGMENT_SHADER_BIT
};

v2 UpdateViewportDimensions(HWND window)
{
  RECT rect;
  bool nonZero = GetClientRect(window, &rect) != 0;
  
  u32 width = 0, height = 0;
  if (nonZero)
  {
    width  = rect.right - rect.left;
    height = rect.bottom - rect.top;
    glViewport(0, 0, width, height);
  }
  
  return (v2) { (f32)width, (f32)height };
}

void GetWglFunctions(void)
{ 
  HWND dummy = CreateWindowExA(0, "STATIC", "DummyWindow", WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);
  assert(dummy != NULL);

  HDC dc = GetDC(dummy);
  assert(dc != NULL);

  PIXELFORMATDESCRIPTOR desc =
  {
    .nSize      = sizeof(desc),
    .nVersion   = 1,
    .dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
    .iPixelType = PFD_TYPE_RGBA,
    .cColorBits = 24
  };

  i32 format = ChoosePixelFormat(dc, &desc);
  assert(format);

  i32 result = DescribePixelFormat(dc, format, sizeof(desc), &desc);
  assert(result);

  result = SetPixelFormat(dc, format, &desc);
  assert(result);

  HGLRC rc = wglCreateContext(dc);
  assert(rc);

  result = wglMakeCurrent(dc, rc);
  assert(result);

  PFNWGL(GETEXTENSIONSSTRINGARB) wglGetExtensionsStringARB = (PFNWGL(GETEXTENSIONSSTRINGARB))wglGetProcAddress("wglGetExtensionsStringARB");
  assert(wglGetExtensionsStringARB);

  const char *extsARB = wglGetExtensionsStringARB(dc);
  assert(extsARB != NULL);

  size_t extsLen = strlen(extsARB);
  
  if (FastStrCmp("WGL_EXT_swap_control", extsARB, extsLen))
  {
    wglSwapIntervalEXT = (PFNWGL(SWAPINTERVALEXT))wglGetProcAddress("wglSwapIntervalEXT");
  }
  
  if (FastStrCmp("WGL_ARB_pixel_format", extsARB, extsLen))
  {
    wglChoosePixelFormatARB = (PFNWGL(CHOOSEPIXELFORMATARB))wglGetProcAddress("wglChoosePixelFormatARB");
  }
  
  if (FastStrCmp("WGL_ARB_create_context", extsARB, extsLen))
  {
    wglCreateContextAttribsARB = (PFNWGL(CREATECONTEXTATTRIBSARB))wglGetProcAddress("wglCreateContextAttribsARB");
  }

  assert(wglChoosePixelFormatARB && wglCreateContextAttribsARB && wglSwapIntervalEXT);

  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(rc);
  ReleaseDC(dummy, dc);
  DestroyWindow(dummy);
}

HWND CreateOpenGLContext(HINSTANCE instance, Win32Context *context)
{
  WNDCLASS windowClass = (WNDCLASS) {
#if DEBUG && DEBUG_IMGUI
    .lpfnWndProc = ImGuiWindowProc,
#else
    .lpfnWndProc = WindowProc,
#endif
    .hInstance = instance,
    .lpszClassName = "Just3DClass",
    .hCursor = LoadCursor(NULL, IDC_ARROW)
  };
  i32 result = RegisterClass(&windowClass);
  assert(result);

  GetWglFunctions();
  
  HWND window = CreateWindowEx(0, "Just3DClass", "Just3D", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, (LPVOID)context);
  assert(window);
  
  context->dc = GetDC(window);
  assert(context->dc);
  
#if DEBUG
  AllocConsole();
  AttachConsole(GetCurrentProcessId());
  HANDLE logger = GetStdHandle(STD_OUTPUT_HANDLE);
  SetStdHandle(STD_OUTPUT_HANDLE, logger);
  SetStdHandle(STD_ERROR_HANDLE, logger);
#endif
          
  i32 attribs[] =
  {
    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
    WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
    WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
    WGL_COLOR_BITS_ARB,     24,
    WGL_DEPTH_BITS_ARB,     24,
    WGL_STENCIL_BITS_ARB,   8,
    WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
    WGL_SAMPLE_BUFFERS_ARB, 1,
    WGL_SAMPLES_ARB,        4,
    0
  };
  
  i32 format; u32 formats;
  result = wglChoosePixelFormatARB(context->dc, attribs, 0, 1, &format, &formats);
  assert(result && formats);

  PIXELFORMATDESCRIPTOR desc = { .nSize = sizeof(desc) };
  result = DescribePixelFormat(context->dc, format, desc.nSize, &desc);
  assert(result);

  format = SetPixelFormat(context->dc, format, &desc);
  assert(format);
  
  i32 contextAttribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
    WGL_CONTEXT_MINOR_VERSION_ARB, 6,
    WGL_CONTEXT_PROFILE_MASK_ARB,
    WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#if DEBUG
    WGL_CONTEXT_FLAGS_ARB,
    WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
    0
  };
  
  context->glrc = wglCreateContextAttribsARB(context->dc, 0, contextAttribs);
  assert(context->glrc);
  
  result = wglMakeCurrent(context->dc, context->glrc);
  assert(result);

#define X(type, name) \
  name = (type)wglGetProcAddress(#name); \
  assert(name);
  GL_FUNC(X)
#undef X
  
#if DEBUG
  glDebugMessageCallback(&glDebugCallback, context);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
    
  UpdateViewportDimensions(window);
  
  return window;
}

GLuint LoadTextureEx(const char * path, GLint wrapS, GLint wrapT, GLint min, GLint mag, bool generateMipmap, GLenum format, GLenum subFormat)
{
  i32 width, height;
  uc *data = stbi_load(path, &width, &height, NULL, STBI_rgb);
  
  GLuint texture;
  glCreateTextures(GL_TEXTURE_2D, 1, &texture);
  
  if (wrapS != -1 && wrapT != -1)
  {
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, wrapS);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, wrapT);
  }
  
  if (min != -1 && mag != -1)
  {
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, min);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, mag);
  }
  
  glTextureStorage2D(texture, 1, format, width, height);
  glTextureSubImage2D(texture, 0, 0, 0, width, height, subFormat, GL_UNSIGNED_BYTE, data);
  
  if (generateMipmap) glGenerateTextureMipmap(texture);
  
  STBI_FREE(data);

  return texture;
}

#define LoadTexture(path) LoadTextureEx(path, GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true, GL_RGB8, GL_RGB);

Model LoadModel(Arena *arena, const char *path)
{
  HANDLE file = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
  assert(file != INVALID_HANDLE_VALUE);
  
  //TODO: Read whole file then attributions + SIMD optimizations
  Model model = {0};
  if (file != INVALID_HANDLE_VALUE)
  {
    ReadFile(file, &model.indicesCount, 4 * sizeof(u32), 0, NULL);
    ReadFile(file, &model.uvScale, 10 * sizeof(f32), 0, NULL);
    
    u16 bounds[6] = {0};
    ReadFile(file, &bounds, 6 * sizeof(u16), 0, NULL);
    for (u32 i = 0; i < 3; ++i)
    {
      model.bounds.min.Elements[i] = (f32)bounds[i] / 65535.f;
      model.bounds.max.Elements[i] = (f32)bounds[i+3] / 65535.f;
    }
    
    TmpArena tmpArena = {0};
    TmpBegin(&tmpArena, arena);
    
    u16 *indices = (u16 *)Alloc(arena, model.indicesSize);
    Vertex *vertices = (Vertex *)Alloc(arena, model.verticesSize);
    ReadFile(file, indices, model.indicesSize, 0, NULL);
    ReadFile(file, vertices, model.verticesSize, 0, NULL);
        
    GLuint vbo;
    glCreateBuffers(1, &vbo);
    glNamedBufferStorage(vbo, model.verticesSize, vertices, GL_DYNAMIC_STORAGE_BIT);

    GLuint ebo;
    glCreateBuffers(1, &ebo);
    glNamedBufferStorage(ebo, model.indicesSize, indices, GL_DYNAMIC_STORAGE_BIT);

    glCreateVertexArrays(1, &model.vao);
    glVertexArrayVertexBuffer(model.vao, 0, vbo, 0, sizeof(Vertex));
    glVertexArrayElementBuffer(model.vao, ebo);

    glEnableVertexArrayAttrib(model.vao, 0);
    glVertexArrayAttribIFormat(model.vao, 0, 3, GL_UNSIGNED_SHORT, offsetof(Vertex, x));
    glVertexArrayAttribBinding(model.vao, 0, 0);

    glEnableVertexArrayAttrib(model.vao, 1);
    glVertexArrayAttribIFormat(model.vao, 1, 3, GL_BYTE, offsetof(Vertex, nx));
    glVertexArrayAttribBinding(model.vao, 1, 0);
    
    glEnableVertexArrayAttrib(model.vao, 2);
    glVertexArrayAttribIFormat(model.vao, 2, 4, GL_BYTE, offsetof(Vertex, tx));
    glVertexArrayAttribBinding(model.vao, 2, 0);

    glEnableVertexArrayAttrib(model.vao, 3);
    glVertexArrayAttribIFormat(model.vao, 3, 2, GL_UNSIGNED_SHORT,  offsetof(Vertex, u));
    glVertexArrayAttribBinding(model.vao, 3, 0);
    
    TmpEnd(&tmpArena);
    CloseHandle(file);
  }
  //TODO: else return a default dummy texture handle
  
  return model;
}

Shader CompileShader(Arena *arena, const char *path, GLenum type)
{
  Shader shader = { .program = -1, .type = type };
  
#if DEBUG
  shader.file = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
#else
  shader.file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
#endif
  
  if (!shader.file)
  {
    Log("Failed to open %s", path);
    assert(0);
  }

#if DEBUG
  GetFileTime(shader.file, 0, 0, &shader.lastWriteTime);
#endif
  
  LARGE_INTEGER largeSize = {0};
  GetFileSizeEx(shader.file, &largeSize);
  DWORD size = (DWORD)largeSize.QuadPart;
  
  TmpArena tmpArena = {0};
  TmpBegin(&tmpArena, arena);
  
  char *fileBuffer = (char *)Alloc(arena, size);
  assert(fileBuffer);
  
  ReadFile(shader.file, (char *)fileBuffer, size, 0, 0);
  fileBuffer[size] = '\0';
  
  GLuint program = glCreateShaderProgramv(type, 1, &fileBuffer);
  
  TmpEnd(&tmpArena);
  
  GLint linked;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (linked)
  {
    shader.program = program;
  }
  else
  {
    char logs[1024];
    glGetProgramInfoLog(program, sizeof(logs), NULL, logs);
    glDeleteProgram(program);
    
#if DEBUG
    if (shaderCompilationFatal)
    {
      Log("Error on %s:\n%s", path, logs);
      assert(0);
    }
#endif
  }
  
#ifndef DEBUG
  CloseHandle(shader.file);
#endif

  return shader;
}

Pipeline CreatePipeline(Arena *arena, u32 shadersCount, Shader *shaders)
{
  Pipeline pipeline = { .shadersCount = shadersCount, .shaders = (Shader *)Alloc(arena, shadersCount * sizeof(Shader)) };
  
  glCreateProgramPipelines(1, &pipeline.program);
  
  for (u32 i = 0; i < shadersCount; ++i)
  {
    glUseProgramStages(pipeline.program, shaderTypesBit[shaders[i].type], shaders[i].program);    
    pipeline.shaders[i] = shaders[i];
  }
  
  return pipeline;
}

#if DEBUG
void GetShaderNewWriteTime(Shader *shader, ULARGE_INTEGER *prevTime, ULARGE_INTEGER *newTime)
{
  *prevTime = (ULARGE_INTEGER) { .LowPart = shader->lastWriteTime.dwLowDateTime, .HighPart = shader->lastWriteTime.dwHighDateTime };
  GetFileTime(shader->file, 0, 0, &shader->lastWriteTime);
  *newTime = (ULARGE_INTEGER) { .LowPart = shader->lastWriteTime.dwLowDateTime, .HighPart = shader->lastWriteTime.dwHighDateTime };
}

void ShadersHotReloading(Arena *arena, Pipeline *pipelines, u32 pipelinesCount)
{
  ULARGE_INTEGER prevWrite, lastWrite;
  bool pipelineModified = false;
  
  for (u32 i = 0; i < pipelinesCount; ++i)
  {
    Pipeline *currentPipeline = &pipelines[i];
    for (u32 j = 0; j < currentPipeline->shadersCount; ++j)
    {
      GetShaderNewWriteTime(&currentPipeline->shaders[j], &prevWrite, &lastWrite);
      if (prevWrite.QuadPart < lastWrite.QuadPart)
      {
        pipelineModified = true;
        break;
      }
    }

    if (pipelineModified)
    {
      glDeleteProgramPipelines(1, &currentPipeline->program);
      glCreateProgramPipelines(1, &currentPipeline->program);

      for (u32 j = 0; j < currentPipeline->shadersCount; ++j)
      {
        char path[MAX_PATH];
        if (!GetFinalPathNameByHandleA(currentPipeline->shaders[j].file, path, MAX_PATH, VOLUME_NAME_NONE))
        {
          return;
        }
      
        for (;;)
        {
          if (currentPipeline->shaders[j].program != -1)
          {
            glDeleteProgram(currentPipeline->shaders[j].program);
          }
        
          currentPipeline->shaders[j] = CompileShader(arena, path, currentPipeline->shaders[j].type);
        
          if (currentPipeline->shaders[j].program >= 0)
          {
            glUseProgramStages(currentPipeline->program, shaderTypesBit[currentPipeline->shaders[j].type], currentPipeline->shaders[j].program);
            break;
          }
          else
          {
            Log("Errors in %s! Waiting for changes...\n", path);
            GetShaderNewWriteTime(&currentPipeline->shaders[j], &prevWrite, &lastWrite);
            while (prevWrite.QuadPart == lastWrite.QuadPart)
            {
              GetShaderNewWriteTime(&currentPipeline->shaders[j], &prevWrite, &lastWrite);
            }
          }
        }
      }     
    }
  }
}
#endif
