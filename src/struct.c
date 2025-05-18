typedef struct GameButtons GameButtons;
typedef struct GameInputs GameInputs;
typedef struct Transform Transform;
typedef struct Vertex Vertex;
typedef struct Bounds Bounds;
typedef struct MetallicRoughnessMaterial MetallicRoughnessMaterial;
typedef struct Model Model;
typedef struct Camera Camera;
typedef struct Entity Entity;

struct GameButtons {
  bool moveForward : 1;
  bool moveBackward : 1;
  bool moveLeft : 1;
  bool moveRight : 1;
  bool moveUp : 1;
  bool moveDown : 1;
  bool back : 1;
  bool enter : 1;
  bool escape : 1;
};

struct GameInputs {
  v2 mousePosition;
  v2 lastAbsMousePosition;
  v2 deltaMousePos;
  u32 mouseSmoothing;
  v2 *mousePosBuffer;
  bool isKeyPressed;
  GameButtons buttons;
};

struct Transform {
  v3 position;
  v3 rotation;
  v3 scale;
};

struct Vertex {
  u16 x, y, z;
  i8 nx, ny, nz;
  i8 tx, ty, tz, handedness;
  u16 u, v;
};

struct Bounds {
  v3 min, max;
};

struct MetallicRoughnessMaterial {
  v4 baseColorFactor;
  f32 metallicFactor;
  f32 roughnessFactor;
  u32 baseColor;
  u32 metallicRoughness;
  u32 normalMap;
  u32 ambientOcclusion;
};

struct Model {
  u32 indicesCount;
  u32 indicesSize;
  u32 verticesCount;
  u32 verticesSize;  
  v2 uvScale;
  v2 uvOffset;
  Bounds bounds;
  u32 vao;
  MetallicRoughnessMaterial material;
};

struct Camera {
  Entity *entity;
  f32 pitch;
  f32 yaw;
  f32 fov;
  f32 aspect;
  m4 view;
  m4 projection;
};

struct Entity {  
  Transform transform;
  m4 transformMatrix;
  m3 normalMatrix;
  ComponentType componentType;
  union {
    Model *model;
    Camera *camera;
    void *data;
  } component;
};
