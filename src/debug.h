#if DEBUG
void LogEx(const char *format, char *filePath, int fileLine, char *function, ...);
#define Log(format, ...) LogEx(format, __FILE__, __LINE__, __func__, __VA_ARGS__)
void PrintEx(const char *format, ...);
#define Print(format, ...) PrintEx(format, __VA_ARGS__)

#define V2U32_FORMAT "%d %d"
#define V2U32_ARGS(v2) v2.x, v2.y

#define V3_FORMAT "%f, %f, %f"
#define V3_ARGS(v3) v3.X, v3.Y, v3.Z

#define V4_FORMAT "%f, %f, %f, %f"
#define V4_ARGS(v4) v4.X, v4.Y, v4.Z, v4.W

#define TRANSFORM_FORMAT "pos = (%f, %f, %f); rot = (%f, %f, %f); sca: %f"
#define TRANSFORM_ARGS(t) V3_ARGS(t.position), V3_ARGS(t.rotation), V3_ARGS(t.scale)

#define M4_FORMAT "\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n"
#define M4_ARGS(m4)\
m4.Elements[0][0], m4.Elements[0][1], m4.Elements[0][2], m4.Elements[0][3],\
m4.Elements[1][0], m4.Elements[1][1], m4.Elements[1][2], m4.Elements[1][3],\
m4.Elements[2][0], m4.Elements[2][1], m4.Elements[2][2], m4.Elements[2][3],\
m4.Elements[3][0], m4.Elements[3][1], m4.Elements[3][2], m4.Elements[3][3]

#else
#define Log(format, ...) ;
#define Print(format, ...) ;
#endif

