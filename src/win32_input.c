GameInputs InitGameInputs(HWND window, f32 mouseSensitivity)
{
  RAWINPUTDEVICE rid[2] = {
    [0] = {
      .usUsagePage = HID_USAGE_PAGE_GENERIC,
      .usUsage = HID_USAGE_GENERIC_MOUSE,
      .hwndTarget = window
    },
    [1] = {
      .usUsagePage = HID_USAGE_PAGE_GENERIC,
      .usUsage = HID_USAGE_GENERIC_KEYBOARD,
      .hwndTarget = window
    }
  };
  
#ifdef DEBUG
  i32 registered = RegisterRawInputDevices(rid, 2, sizeof(rid[0]));
  assert(registered);
#else
  RegisterRawInputDevices(rid, 2, sizeof(rid[0]));
#endif

  return (GameInputs) {
    .mouseSensitivity = mouseSensitivity,
    .lastMouseDelta = (v2){FLT_MAX, FLT_MAX}
  };
}

#define MapInput(code, key) case code: ##key## = isKeyPressed; break

void HandleRawInputs(LPARAM lParam, Win32Context *context)
{ 
  GameInputs newInputs = *context->inputs;
  
  u32 dwSize = sizeof(RAWINPUT);
  u32 hSize = sizeof(RAWINPUTHEADER);
  static BYTE lpb[sizeof(RAWINPUT)];
  GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, hSize);

  RAWINPUT *raw = (RAWINPUT *)lpb;
  switch (raw->header.dwType)
  {
    case RIM_TYPEMOUSE: {
      RAWMOUSE mouse = raw->data.mouse;
      
      i32 width, height;
      bool virtualDesktop = (mouse.usFlags & MOUSE_VIRTUAL_DESKTOP) == MOUSE_VIRTUAL_DESKTOP;
      
      if (virtualDesktop)
      {
        width = context->virtualDesktopWidth;
        height = context->virtualDesktopHeight;
      }
      else
      {
        width = context->primaryScreenWidth;
        height = context->primaryScreenHeight;
      }
      
      if ((mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE)
      {
        v2 absoluteMouseDelta = (v2) {
          mouse.lLastX / (f32)USHRT_MAX,
          mouse.lLastY / (f32)USHRT_MAX
        };
        
        if (newInputs.lastMouseDelta.X != FLT_MAX && newInputs.lastMouseDelta.Y != FLT_MAX)
        {
          newInputs.mouseDelta = HMM_SubV2(absoluteMouseDelta, newInputs.lastMouseDelta);
        }
        
        newInputs.lastMouseDelta = absoluteMouseDelta;
      }
      else
      {
        newInputs.lastMouseDelta = newInputs.mouseDelta;
        newInputs.mouseDelta = (v2) {
          (f32)raw->data.mouse.lLastX / (f32)width,
          (f32)raw->data.mouse.lLastY / (f32)width
        };
        newInputs.mouseDelta = HMM_MulV2F(HMM_AddV2(newInputs.mouseDelta, newInputs.lastMouseDelta), 0.5f);
      }
    } break;

    case RIM_TYPEKEYBOARD: {
      u32 vkCode = raw->data.keyboard.VKey;
      u16 scanCode = raw->data.keyboard.MakeCode;
      u32 isKeyPressed = ((raw->data.keyboard.Flags & RI_KEY_BREAK) == 0);
      
      switch (scanCode) // Layout agnostic
      {
        MapInput(0x0011, newInputs.buttons.moveForward);  // W
        MapInput(0x001E, newInputs.buttons.moveLeft);     // A
        MapInput(0x001F, newInputs.buttons.moveBackward); // S
        MapInput(0x0020, newInputs.buttons.moveRight);    // D
      }
      
      if (vkCode == VK_SHIFT || vkCode == VK_CONTROL || vkCode == VK_MENU)
      {
        vkCode = LOWORD(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
      }

      switch (vkCode) // Layout dependent
      {
        MapInput(VK_SPACE,  newInputs.buttons.moveUp);
        MapInput(VK_LSHIFT, newInputs.buttons.moveDown);
        MapInput(VK_ESCAPE, newInputs.buttons.escape);
        MapInput(VK_RETURN, newInputs.buttons.enter);
        MapInput(VK_BACK,   newInputs.buttons.back);
      }
      
      newInputs.isKeyPressed = isKeyPressed;
    } break;

    default: break;
  }
  
  *context->inputs = newInputs;
}

