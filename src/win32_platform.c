POINT GetClientCenter(HWND window)
{
  RECT cliRect;
  GetClientRect(window, &cliRect);
  POINT cliCenter = (POINT) { .x = cliRect.left + (cliRect.right - cliRect.left) / 2, .y = cliRect.top  + (cliRect.bottom - cliRect.top) / 2 };
  ClientToScreen(window, &cliCenter);
  return cliCenter;
}

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
