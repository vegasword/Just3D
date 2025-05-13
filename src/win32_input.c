GameInputs InitGameInputs(Arena *arena, HWND window)
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
    .mouseSmoothing = 2,
    .lastAbsMousePosition = (v2){FLT_MAX, FLT_MAX},
    .mousePosBuffer = (v2 *)Alloc(arena, 2 * sizeof(v2)),
  };
}

void ApplyNonLinearMouseFiltering(GameInputs *pInputs)
{
  GameInputs inputs = *pInputs;
  v2 filter = {0};
  
  inputs.mousePosBuffer[0] = inputs.deltaMousePos;    
  inputs.deltaMousePos = HMM_V2(0, 0);

  for (u32 i = inputs.mouseSmoothing - 1; i > 0; --i)
  {
    inputs.mousePosBuffer[i] = inputs.mousePosBuffer[i - 1];
    filter = HMM_AddV2(filter, HMM_MulV2F(inputs.mousePosBuffer[i], powf(1 - ((f32)i / inputs.mouseSmoothing), 5)));
  }

  inputs.mousePosition = HMM_DivV2F(filter, (f32)inputs.mouseSmoothing);
  *pInputs = inputs;
}

#define MapInput(code, key) case code: ##key## = isKeyPressed; break
void HandleRawInputs(LPARAM lParam, GameInputs *inputs)
{ 
  u32 dwSize = sizeof(RAWINPUT);
  u32 hSize = sizeof(RAWINPUTHEADER);
  static BYTE lpb[sizeof(RAWINPUT)];
  GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, hSize);

  RAWINPUT *raw = (RAWINPUT *)lpb;
  switch (raw->header.dwType)
  {
    case RIM_TYPEMOUSE: {     
      RAWMOUSE mouse = raw->data.mouse;
      if ((mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE)
      {
        bool isVirtualDesktop = (mouse.usFlags & MOUSE_VIRTUAL_DESKTOP) == MOUSE_VIRTUAL_DESKTOP;
        
        i32 width, height;
        if (isVirtualDesktop)
        {
          width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
          height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        }
        else
        {
          width = GetSystemMetrics(SM_CXSCREEN);
          height = GetSystemMetrics(SM_CYSCREEN);
        }
        
        v2 absoluteMousePosition = (v2) { (mouse.lLastX / (f32)USHRT_MAX) * width, (mouse.lLastY / (f32)USHRT_MAX) * height };
        if (inputs->lastAbsMousePosition.X != FLT_MAX && inputs->lastAbsMousePosition.Y != FLT_MAX)
        {
          inputs->deltaMousePos = HMM_SubV2(absoluteMousePosition, inputs->lastAbsMousePosition);          
        }
        inputs->lastAbsMousePosition = absoluteMousePosition;
      }
      else
      {
        inputs->deltaMousePos = (v2) {  (f32)raw->data.mouse.lLastX,  (f32)raw->data.mouse.lLastY };
      }
    } break;

    case RIM_TYPEKEYBOARD: {
      u32 vkCode = raw->data.keyboard.VKey;
      u16 scanCode = raw->data.keyboard.MakeCode;
      u32 isKeyPressed = ((raw->data.keyboard.Flags & RI_KEY_BREAK) == 0);
      
      switch (scanCode) // Layout agnostic
      {
        MapInput(0x0011, inputs->buttons.moveForward);  // W
        MapInput(0x001E, inputs->buttons.moveLeft);     // A
        MapInput(0x001F, inputs->buttons.moveBackward); // S
        MapInput(0x0020, inputs->buttons.moveRight);    // D          
      }
      
      if (vkCode == VK_SHIFT || vkCode == VK_CONTROL || vkCode == VK_MENU)
      {
        vkCode = LOWORD(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
      }

      switch (vkCode) // Layout dependent
      {
        MapInput(VK_SPACE,  inputs->buttons.moveUp);
        MapInput(VK_LSHIFT,  inputs->buttons.moveDown);
        MapInput(VK_ESCAPE, inputs->buttons.escape);
        MapInput(VK_RETURN, inputs->buttons.enter);
        MapInput(VK_BACK,   inputs->buttons.back);
      }
      
      inputs->isKeyPressed = isKeyPressed;
    } break;
  }
}
