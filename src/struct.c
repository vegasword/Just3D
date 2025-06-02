/*
// Gameplay
*/

typedef struct GameButtons {
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

typedef struct GameInputs {
  v2 mousePosition;
  v2 lastAbsMousePosition;
  v2 deltaMousePos;
  u32 mouseSmoothing;
  v2 *mousePosBuffer;
  bool isKeyPressed;
  GameButtons buttons;
} GameInputs;

/*
// Geometry
*/

typedef struct Transform {
  v3 position;
  v3 rotation;
  v3 scale;
} Transform;

typedef struct Vertex {
  u16 x, y, z;
  i8 nx, ny, nz;
  i8 tx, ty, tz, handedness;
  u16 u, v;
} Vertex;

typedef struct Bounds {
  v3 min, max;
} Bounds;

/*
// Rendering
*/

typedef struct UniformBuffer
{
  m4 mvp;
  m4 modelMatrix;
  v4 normalMatrixFirstColumn;
  v4 normalMatrixSecondColumn;
  v4 normalMatrixThirdColumn;
  v4 baseColor;
  v4 cameraPosition;
  v2 uvScale;
  v2 uvOffset;
  f32 metallicFactor;
  f32 roughnessFactor;
} UniformBuffer;

/*
// Model
*/

typedef struct ModelHeader
{
  u32 indicesCount;
  u32 indicesSize;
  u32 verticesCount;
  u32 verticesSize;
} ModelHeader;

typedef struct ModelData {
  v2 uvScale;
  v2 uvOffset;
  v4 baseColor;
  f32 metallicFactor;
  f32 roughnessFactor;
} ModelData;

typedef struct Model {
  m4 transformMatrix;
  m4 viewMatrix;
  ModelData data;
  u32 baseColorMap;
  u32 metallicRoughnessMap;
  u32 ambientOcclusionMap;
  u32 normalMap;
  u32 vao;
  u32 indicesCount;
  Transform transform;
  Bounds bounds;
} Model;

/*
// Entities
*/

typedef struct Camera {
  m4 transformMatrix;
  m4 viewMatrix;
  m4 projectionMatrix;
  f32 speed;
  f32 pitch;
  f32 yaw;
  f32 fov;
  f32 aspect;
  Transform transform;
} Camera;
