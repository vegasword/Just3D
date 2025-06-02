typedef bool b8;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

typedef signed char sc; 
typedef unsigned char uc; 

typedef HMM_Vec2 v2;
typedef HMM_Vec3 v3;
typedef HMM_Vec4 v4;
typedef HMM_Mat3 m3;
typedef HMM_Mat4 m4;
typedef HMM_Quat qt;

#if DEBUG

#define V2_FORMAT "%f, %f"
#define V2_ARGS(v2) v2.X, v2.Y

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

#endif
