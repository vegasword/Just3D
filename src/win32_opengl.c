#if DEBUG && DEBUG_IMGUI
LRESULT CALLBACK ImGuiWindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
#else
LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

#define PFNGL(name) PFNGL##name##PROC
#define PFNWGL(name) PFNWGL##name##PROC

#define GL_FUNC(X) \
    X(PFNGL(CREATEBUFFERS),             glCreateBuffers             )\
    X(PFNGL(NAMEDBUFFERDATA),           glNamedBufferData           )\
    X(PFNGL(NAMEDBUFFERSTORAGE),        glNamedBufferStorage        )\
    X(PFNGL(NAMEDBUFFERSUBDATA),        glNamedBufferSubData        )\
    X(PFNGL(BINDBUFFERBASE),            glBindBufferBase            )\
\
    X(PFNGL(CREATEVERTEXARRAYS),        glCreateVertexArrays        )\
    X(PFNGL(VERTEXARRAYATTRIBBINDING),  glVertexArrayAttribBinding  )\
    X(PFNGL(VERTEXARRAYATTRIBFORMAT),   glVertexArrayAttribFormat   )\
    X(PFNGL(VERTEXARRAYBINDINGDIVISOR), glVertexArrayBindingDivisor )\
    X(PFNGL(VERTEXARRAYATTRIBIFORMAT),  glVertexArrayAttribIFormat  )\
    X(PFNGL(VERTEXARRAYVERTEXBUFFER),   glVertexArrayVertexBuffer   )\
    X(PFNGL(VERTEXARRAYELEMENTBUFFER),  glVertexArrayElementBuffer  )\
    X(PFNGL(ENABLEVERTEXARRAYATTRIB),   glEnableVertexArrayAttrib   )\
    X(PFNGL(BINDVERTEXARRAY),           glBindVertexArray           )\
\
    X(PFNGL(CREATESHADERPROGRAMV),      glCreateShaderProgramv      )\
    X(PFNGL(VALIDATEPROGRAM),           glValidateProgram           )\
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

#define GL_DEBUG_CASE(buffer, category, subcategory) \
  case GL_DEBUG_##category##_##subcategory##: \
    strncpy_s(buffer, 32, #subcategory, sizeof(#subcategory) - 1); \
    break
    
#pragma warning(push)
#pragma warning(disable: 4100)
void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user)
{
  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
  
  char sourceStr[32];
  switch (source)
  {
    GL_DEBUG_CASE(sourceStr, SOURCE, API);
    GL_DEBUG_CASE(sourceStr, SOURCE, WINDOW_SYSTEM);
    GL_DEBUG_CASE(sourceStr, SOURCE, SHADER_COMPILER);
    GL_DEBUG_CASE(sourceStr, SOURCE, THIRD_PARTY);
    GL_DEBUG_CASE(sourceStr, SOURCE, APPLICATION);
    GL_DEBUG_CASE(sourceStr, SOURCE, OTHER);
  }

  char typeStr[32];
  switch (type)
  {
    GL_DEBUG_CASE(typeStr, TYPE, ERROR);
    GL_DEBUG_CASE(typeStr, TYPE, DEPRECATED_BEHAVIOR);
    GL_DEBUG_CASE(typeStr, TYPE, UNDEFINED_BEHAVIOR);
    GL_DEBUG_CASE(typeStr, TYPE, PORTABILITY);
    GL_DEBUG_CASE(typeStr, TYPE, PERFORMANCE);
    GL_DEBUG_CASE(typeStr, TYPE, OTHER);
    default: break;
  }

  char severityStr[32];
  switch (severity)
  {
    GL_DEBUG_CASE(severityStr, SEVERITY, LOW);
    GL_DEBUG_CASE(severityStr, SEVERITY, MEDIUM);
    GL_DEBUG_CASE(severityStr, SEVERITY, HIGH);
  }
  Log("[OPENGL] [%s/%s/%s] %s\n", sourceStr, typeStr, severityStr, message);
}
#pragma warning(pop)

static PFNWGL(SWAPINTERVALEXT) wglSwapIntervalEXT = NULL;
static PFNWGL(CHOOSEPIXELFORMATARB) wglChoosePixelFormatARB = NULL;
static PFNWGL(CREATECONTEXTATTRIBSARB) wglCreateContextAttribsARB = NULL;

#if DEBUG
static bool shaderRuntimeCompilationFatal = true;
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
    .lpszClassName = WINDOW_TITLE"Class",
    .hCursor = LoadCursor(NULL, IDC_ARROW)
  };
  i32 result = RegisterClass(&windowClass);
  assert(result);

  GetWglFunctions();
  
  HWND window = CreateWindowEx(0, WINDOW_TITLE"Class", WINDOW_TITLE, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, (LPVOID)context);
  assert(window);
  
  context->dc = GetDC(window);
  assert(context->dc);
            
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
  
  char title[128] = WINDOW_TITLE;
  strncat_s(title, 128, " - ", _TRUNCATE);
  strncat_s(title, 128, (const char *)glGetString(GL_VERSION), _TRUNCATE);
  SetWindowText(window, title);
  
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
  File file = ReadWholeFile(arena, path);
    
  ModelHeader modelHeader = *(ModelHeader *)file.buffer;
  file.buffer += sizeof(ModelHeader);
  
  Model model = { .indicesCount = modelHeader.indicesCount, .data = *(ModelData *)file.buffer };
  file.buffer += offsetof(ModelData, roughnessFactor) + sizeof(f32);
  
  u16 *bounds = (u16 *)file.buffer;
  for (u32 i = 0; i < 3; ++i)
  {
    model.bounds.min.Elements[i] = (f32)bounds[i] / 65535.f;
    model.bounds.max.Elements[i] = (f32)bounds[i+3] / 65535.f;
  }
  file.buffer += 6 * sizeof(u16);
    
  GLuint ebo;
  glCreateBuffers(1, &ebo);
  glNamedBufferStorage(ebo, modelHeader.indicesSize, file.buffer, GL_DYNAMIC_STORAGE_BIT);
  
  GLuint vbo;
  glCreateBuffers(1, &vbo);
  glNamedBufferStorage(vbo, modelHeader.verticesSize, file.buffer + modelHeader.indicesSize, GL_DYNAMIC_STORAGE_BIT);

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

  return model;
}

Shader CompileShader(Arena *arena, const char *path, GLenum type)
{
  Shader shader = { .program = -1, .type = type };
  
#if DEBUG
  File file = ReadWholeFileEx(arena, path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, 0);
  if (shader.fileHandle == 0)
  {
    shader.fileHandle = file.handle;
  }
  GetFileTime(shader.fileHandle, 0, 0, &shader.lastWriteTime);
#else
  File file = ReadWholeFile(arena, path);
#endif

  GLuint program = glCreateShaderProgramv(type, 1, (const GLchar *const *)&file.buffer);
    
  GLint linked;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
    
  if (linked)
  {    
    glValidateProgram(program);
    GLint validated;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &validated);
    if (validated)
    {
      shader.program = program;
      goto shader_succesfully_compiled;
    }
  }  
  
#if DEBUG
  TmpArena tmpArena = {0};
  TmpBegin(&tmpArena, arena);
  
  GLint logsLength = 0;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logsLength);
  
  char *logs = (char *)Alloc(arena, logsLength);
  glGetProgramInfoLog(program, logsLength, NULL, logs);
  
  glDeleteProgram(program);
  
  Log("Error on %s:\n%s", path, logs);
    
  assert(!shaderRuntimeCompilationFatal);
#endif

shader_succesfully_compiled:
#if DEBUG == 0
  CloseHandle(file.handle);
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
  GetFileTime(shader->fileHandle, 0, 0, &shader->lastWriteTime);
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
        if (!GetFinalPathNameByHandleA(currentPipeline->shaders[j].fileHandle, path, MAX_PATH, VOLUME_NAME_NONE))
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
            glBindProgramPipeline(currentPipeline->program);
            break;
          }
          else
          {
            Log("Waiting for changes...\n");
            do GetShaderNewWriteTime(&currentPipeline->shaders[j], &prevWrite, &lastWrite);
            while (prevWrite.QuadPart == lastWrite.QuadPart);
          }
        }
      }     
    }
  }
}
#endif

u32 CreateShaderBufferObject(u32 binding, size_t size, GLenum target, GLenum usage)
{
  GLuint shaderBufferObject;
  glCreateBuffers(1, &shaderBufferObject);
  glNamedBufferData(shaderBufferObject, size, 0, usage);
  glBindBufferBase(target, binding, shaderBufferObject);
  return shaderBufferObject;
}
