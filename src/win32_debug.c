void LogEx(const char *format, char *filePath, int fileLine, char *function, ...)
{  
  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  
  char log[1*KB];
  char *fileName = filePath + strnlen_s(filePath, MAX_PATH);
  while (*(fileName - 1) != '\\') fileName--;
  sprintf_s(log, sizeof(log), "%s(%d) | %s : ", fileName, fileLine, function);
  WriteFile(console, log, (DWORD)strlen(log), 0, NULL);
  OutputDebugString(log);
  
  char message[4*KB];
  va_list args;
  va_start(args, function);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);
    
  WriteFile(console, message, (DWORD)strlen(message), 0, NULL);
  OutputDebugString(message);
}

void Print(const char *format, ...)
{
  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  char message[4*KB];
  va_list args;
  va_start(args, format);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);
    
  WriteFile(console, message, (DWORD)strlen(message), 0, NULL);
  OutputDebugString(message);
}

void ClearConsole(void)
{
  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  
  COORD coordScreen = { 0, 0 };
  DWORD charsWritten;
  CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
  DWORD consoleSize;
  
  GetConsoleScreenBufferInfo(console, &consoleInfo);
  consoleSize = consoleInfo.dwSize.X * consoleInfo.dwSize.Y;
  
  FillConsoleOutputCharacter(console, (TCHAR)' ', consoleSize, coordScreen,
                             &charsWritten);

  FillConsoleOutputAttribute(console, consoleInfo.wAttributes, consoleSize,
                             coordScreen, &charsWritten);
  
  SetConsoleCursorPosition(console, coordScreen);
}

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
