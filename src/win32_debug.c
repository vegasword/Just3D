void LogEx(const char *format, char *filePath, int fileLine, char *function, ...)
{    
  HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
  
  char log[4*KB];
  char *fileName = filePath + strnlen_s(filePath, MAX_PATH);
  while (*(fileName - 1) != '\\') fileName--;
  sprintf_s(log, sizeof(log), "%s(%d) | %s : ", fileName, fileLine, function);
  WriteFile(console, log, (DWORD)strlen(log), 0, NULL);
  
  char message[4*KB];
  va_list args;
  va_start(args, function);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);
  WriteFile(console, message, (DWORD)strlen(message), 0, NULL);
}
