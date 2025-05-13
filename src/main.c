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
#include "debug.h"
#include "algorithm.c"

#if DEBUG_MEMORY
  #include "arena_debug.c"
#else
  #include "arena.c"
#endif

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
  #include "win32_opengl.c"
  #include "win32_platform.c"
  #if DEBUG && DEBUG_IMGUI
    #include "win32_imgui.c"
  #endif
  #include "win32_main.c"
#endif 

