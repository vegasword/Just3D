POINT GetClientCenter(HWND window)
{
  RECT cliRect;
  GetClientRect(window, &cliRect);
  POINT cliCenter = (POINT) { .x = cliRect.left + (cliRect.right - cliRect.left) / 2, .y = cliRect.top  + (cliRect.bottom - cliRect.top) / 2 };
  ClientToScreen(window, &cliCenter);
  return cliCenter;
}

#if DEBUG
void InitDebugConsole()
{
  AllocConsole();
  AttachConsole(GetCurrentProcessId());
  HANDLE logger = GetStdHandle(STD_OUTPUT_HANDLE);
  SetStdHandle(STD_OUTPUT_HANDLE, logger);
  SetStdHandle(STD_ERROR_HANDLE, logger);
}
#endif

void Quit(HWND window, Win32Context *context)
{
#if DEBUG
  SetStdHandle(STD_OUTPUT_HANDLE, NULL);
  SetStdHandle(STD_ERROR_HANDLE, NULL);
  FreeConsole();
#endif
  context->quitting = true;
  ReleaseDC(window, context->dc);
  wglDeleteContext(context->glrc);
}

// WARNING: This function doesn't close the created file handle
File ReadWholeFileEx(Arena *arena, const char* path, DWORD desiredAccess, DWORD sharedMode, DWORD creationDisposition, DWORD flags)
{
  File file = {0};
  file.handle = CreateFile(path, desiredAccess, sharedMode, NULL, creationDisposition, flags, NULL);
  assert(file.handle != INVALID_HANDLE_VALUE);
  
  LARGE_INTEGER largeSize = {0};
  GetFileSizeEx(file.handle, &largeSize);

  DWORD numberOfBytesToRead = (DWORD)largeSize.QuadPart;
  DWORD numberOfBytesRead = 0;

  TmpArena tmpArena = {0};
  TmpBegin(&tmpArena, arena);
  
  file.buffer = (uc *)Alloc(arena, numberOfBytesToRead);

  BOOL result = ReadFile(file.handle, file.buffer, numberOfBytesToRead, &numberOfBytesRead, NULL);
  assert(result && numberOfBytesToRead == numberOfBytesRead);

  file.buffer[numberOfBytesRead] = '\0';
  
  TmpEnd(&tmpArena);

  return file;
}
#define ReadWholeFile(arena, path) ReadWholeFileEx(arena, path, GENERIC_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY)
