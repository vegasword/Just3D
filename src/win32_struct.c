typedef struct {
  LARGE_INTEGER frequency;
  LARGE_INTEGER t1;
  LARGE_INTEGER t2;
  u64 elapsedMilliseconds;
} PerfCounter;

typedef struct File {
  HANDLE handle;
  uc *buffer;
} File;

typedef struct {
  GLint program;
  GLenum type;
#if DEBUG
  HANDLE fileHandle;
  FILETIME lastWriteTime;
#endif
} Shader;

typedef struct {
  GLuint program;
  u32 shadersCount;
  Shader *shaders;
} Pipeline;

typedef struct {
  HDC dc;
  HGLRC glrc;
  Arena *arena;
  GameInputs *inputs;
  v2 viewport;
  f32 viewportAspect;
  i32 virtualDesktopWidth;
  i32 virtualDesktopHeight;
  i32 primaryScreenWidth;
  i32 primaryScreenHeight;
  bool quitting;
#if DEBUG
  bool imguiDebugging;
#endif
} Win32Context;
