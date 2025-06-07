#define WINDOW_TITLE "Just3D"

#if DEBUG
#define DEBUG_IMGUI 1
#endif

#include "stdint.h"
#include "stdbool.h"
#include "time.h"
#include "float.h"

#define HANDMADE_MATH_USE_TURNS
#include "HandmadeMath.h"

#pragma warning(push)
#pragma warning(disable: 4100)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma warning(pop)

#if DEBUG && DEBUG_IMGUI
  #define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
  #include "cimgui.h"
  #ifdef _WIN32
    #include "cimgui_win32_backend.h"
  #endif
#endif

#include "type.c"

#if DEBUG
void LogEx(const char *format, char *filePath, int fileLine, char *function, ...);
#define Log(format, ...) LogEx(format, __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define Log(format, ...) ;
#endif

#include "algorithm.c"

#include "arena.c"

#include "struct.c"
#include "math.c"
#include "entity.c"

#ifdef _WIN32
  #include "glcorearb.h"
  #include "wglext.h"
  #include "GL/gl.h"
  #include "hidusage.h"
  #if DEBUG
    #include "win32_debug.c"
  #endif  
  #include "win32_struct.c"
  #include "win32_time.c"
  #include "win32_input.c"
  #include "win32_platform.c"
  #include "win32_opengl.c"
  #if DEBUG && DEBUG_IMGUI
    #include "win32_imgui.c"
  #endif
  #include "win32_main.c"
#endif 

