typedef struct {
  bool moveForward : 1;
  bool moveBackward : 1;
  bool moveLeft : 1;
  bool moveRight : 1;
  bool moveUp : 1;
  bool moveDown : 1;
  bool back : 1;
  bool enter : 1;
  bool escape : 1;
} GameButtons;

typedef struct {
  v2 mousePosition;
  v2 lastAbsMousePosition;
  v2 deltaMousePos;
  u32 mouseSmoothing;
  v2 *mousePosBuffer;
  bool isKeyPressed;
  GameButtons buttons;
} GameInputs;

typedef struct {
  v3 position;
  v3 rotation;
  v3 scale;
} Transform;

typedef struct {
  v3 min;
  v3 max;
} Boundings;

typedef struct {
  u16 x, y, z, pad0;
  i8 nx, ny, nz, pad1;
  u16 u, v;
} Vertex;

typedef struct {
  u32 vao;
  u32 indicesCount;
  u32 indicesSize;
  u32 verticesCount;
  u32 verticesSize;
  v2 uvScale;
  v2 uvOffset;
  Boundings boundings;
} Model;

typedef struct {
  f32 pitch;
  f32 yaw;
  f32 fov;
  f32 aspect;
  Transform *transform;
  m4 *view;
  m4 *projection;
} Camera;

typedef struct {  
  Transform *transform;
  m4 *modelMatrix;
  Model *model;
  u32 texture;
} Entity;
