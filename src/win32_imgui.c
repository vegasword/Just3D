typedef struct  {
  ImGuiContext *ctx;
  ImGuiIO *io;
} ImGui;

typedef struct {  
  Arena *arena;
  Win32Context *win32;
  f32 frameDelay;
  u32 *entitiesCount;
  Entity *entities;
  u32 *modelsCount;
  Model *models;
  u32 *texturesCount;
  u32 *textures;
} ImGuiDebugData;

static i32 newModelIndex = 0;
static i32 newTextureIndex = 0;

void InitImGui(HWND window)
{
  ImGui imgui = { 0 };
  imgui.ctx = igCreateContext(NULL);
  imgui.ctx->Style.ItemSpacing = (ImVec2){0};
  imgui.ctx->Style.ItemInnerSpacing = (ImVec2){0};
  imgui.io = igGetIO();
  imgui.io->IniFilename = NULL;
  ImGui_ImplWin32_InitForOpenGL(window);
  ImGui_ImplOpenGL3_Init(NULL);
  igStyleColorsDark(NULL);
}

void UpdateImGui(ImGuiDebugData *data)
{
  Win32Context *win32 = data->win32;
  
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplWin32_NewFrame();
  igNewFrame();
  
  if (igGetKeyPressedAmount(ImGuiKey_F1, 0.5f, 0.1f))
  {
    win32->imguiDebugging = !win32->imguiDebugging;
    if (win32->imguiDebugging)
    {
      ClipCursor(NULL);
      ShowCursor(true);
    }
    else
    {
      POINT cliCenter = GetClientCenter(WindowFromDC(win32->dc));
      RECT cursorClipBounds = (RECT) { .left = cliCenter.x, .top = cliCenter.y, .right = cliCenter.x, .bottom = cliCenter.y };
      ClipCursor(&cursorClipBounds);
      ShowCursor(false);
    }
  }
  
  if (win32->imguiDebugging)
  {
    if (igBegin("DEBUG MODE", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
    {
      igSetWindowPos_Vec2((ImVec2){0, 0}, ImGuiCond_Always);
      igSetWindowSize_Vec2((ImVec2){win32->viewport.Width / 4, win32->viewport.Height}, ImGuiCond_Once);
      
      igText("%.1fms", data->frameDelay);
    
      ImGuiTabBarFlags tab_bar_flags = 0;
      if (igBeginTabBar("DebugTabBar", tab_bar_flags))
      {      
        if (igBeginTabItem("Entities", NULL, 0))
        {
          for (u32 i = 0; i < *data->entitiesCount; ++i)
          {
            Entity *entity = &data->entities[i];
            
            igPushID_Ptr(entity);
            
            if (igTreeNodeEx_StrStr("##", 0, "Entity %d (%lx)", i, entity))
            {              
              igSeparatorText("Transform");

              Transform *transform = entity->transform;              
              if (igDragFloat3("Position", (f32 *)&transform->position, 0.01f, 0, 0, "%f", ImGuiInputTextFlags_CharsDecimal))
              {
                UpdateEntityCenteredModelMatrix(entity);
              }
      
              if (igDragFloat3("Rotation", (f32 *)&transform->rotation, 0.01f, 0, 0, "%f", ImGuiInputTextFlags_CharsDecimal))
              {
                UpdateEntityCenteredModelMatrix(entity);
              }
      
              if (igDragFloat3("Scale", (f32 *)&transform->scale, 0.01f, 0.001f, FLT_MAX, "%f", ImGuiInputTextFlags_CharsDecimal))
              {
                UpdateEntityCenteredModelMatrix(entity);
              }

              if (igButton("Copy transform", (ImVec2){0}))
              {
                char transformString[512];
                
                v3 position = transform->position;
                v3 rotation = transform->rotation;
                v3 scale = transform->scale;
                
                igImFormatString(transformString, sizeof(transform),
                                 "(Transform){ (v3){%.3f,%.3f,%.3f}, (v3){%.3f,%.3f,%.3f}, (v3){%.3f,%.3f,%.3f} }",
                                 position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, scale.X, scale.Y, scale.Z);
                
                igSetClipboardText(transformString);
              }

              igSeparatorText("Rendering");
              
              if (igDragInt("Model", &newModelIndex, 0.05f, 0, *data->modelsCount - 1, "%d", ImGuiSliderFlags_AlwaysClamp))
              {
                entity->model = &data->models[newModelIndex];
              }
              
              if (igDragInt("Texture", &newTextureIndex, 0.05f, 0, *data->texturesCount - 1, "%d", ImGuiSliderFlags_AlwaysClamp))
              {
                entity->texture = data->textures[newTextureIndex];
              }

              igTreePop();
            }
            igPopID();
          }
          
          igEndTabItem();
        }
        igEndTabBar();
      }
    }
    igEnd();
  }
}

void RenderImGui(void)
{
  igRender();
  ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
