#ifndef MATH_DEF
#define MATH_DEF
#include <math.h>
#include <string.h>
#include <float.h>

#define SWAP(x, y, T) do { T SWAP = x; x = y; y = SWAP; } while(0)

#define EPSILON 0.000005
#define PI 3.14159265359

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    float a;
    float b;
    float c;
    float d;
} Plane;

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec4;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Quat;

typedef struct {
    Vec2 pos;
    Vec2 line;
} Ray;

typedef struct {
    float x;
    float y;
    float z;
    float w;
    float h;
} Rect;

typedef struct {
    float x;
    float y;
    float z;
    float w;
    float h;
    float d;
} Cube;

typedef struct {
    float x;
    float y;
    float w;
    float h;
} Rect2D;

typedef struct {
    float mag;
    Quat quat;
} Joint;

typedef struct {
    Joint *joints;
    int nJoints;
    Vec3 basePos;
} Arm;

static inline char Math_Vec2EqualToVec2(Vec2 v1, Vec2 v2){ return (char)(v1.x == v2.x && v1.y == v2.y); }
static inline Vec2 Math_Vec2MultFloat(Vec2 v1, float s){ return (Vec2){v1.x * s, v1.y * s}; }
static inline Vec2 Math_Vec2MultVec2(Vec2 v1, Vec2 v2){ return (Vec2){v1.x * v2.x, v1.y * v2.y}; }
static inline Vec2 Math_Vec2DivideFloat(Vec2 v1, float s){ return (Vec2){v1.x / s, v1.y / s}; }
static inline Vec2 Math_Vec2DivideVec2(Vec2 v1, Vec2 v2){ return (Vec2){v1.x / v2.x, v1.y / v2.y}; }
static inline Vec2 Math_Vec2AddFloat(Vec2 v1, float s){ return (Vec2){v1.x + s, v1.y + s}; }
static inline Vec2 Math_Vec2AddVec2(Vec2 v1, Vec2 v2){ return (Vec2){v1.x + v2.x, v1.y + v2.y}; }
static inline Vec2 Math_Vec2SubFloat(Vec2 v1, float s){ return (Vec2){v1.x - s, v1.y - s}; }
static inline Vec2 Math_Vec2SubVec2(Vec2 v1, Vec2 v2){ return (Vec2){v1.x - v2.x, v1.y - v2.y}; }
static inline float Math_Vec2Magnitude(Vec2 v) { return sqrt(v.x*v.x + v.y*v.y); }
static inline float Math_Vec2Dot(Vec2 v1, Vec2 v2){ return (v1.x * v2.x) + (v1.y * v2.y); }
static inline Vec2 Math_RoundVec2(Vec2 v){ return (Vec2){roundf(v.x), roundf(v.y)}; }

static inline Vec2 Math_Vec2Reflect(Vec2 v, Vec2 l){
    return Math_Vec2SubVec2(Math_Vec2MultFloat(Math_Vec2MultFloat(l, 2), (Math_Vec2Dot(v, l) / Math_Vec2Dot(l, l))), v);
}

static inline Vec2 Math_Vec2Normalize(Vec2 v){
    float mag = Math_Vec2Magnitude(v);
    v.x /= mag;
    v.y /= mag;
    return v;
}


static inline char Math_Vec3EqualToVec3(Vec3 v1, Vec3 v2){ return (char)(v1.x == v2.x && v1.y == v2.y && v1.z == v2.z); }
static inline Vec3 Math_Vec3MultFloat(Vec3 v1, float s){ return (Vec3){v1.x * s, v1.y * s, v1.z * s}; }
static inline Vec3 Math_Vec3MultVec3(Vec3 v1, Vec3 v2){ return (Vec3){v1.x * v2.x, v1.y * v2.y, v1.z * v2.z}; }
static inline Vec3 Math_Vec3DivideFloat(Vec3 v1, float s){ return (Vec3){v1.x / s, v1.y / s, v1.z / s}; }
static inline Vec3 Math_Vec3DivideVec3(Vec3 v1, Vec3 v2){ return (Vec3){v1.x / v2.x, v1.y / v2.y, v1.z / v2.z}; }
static inline Vec3 Math_Vec3AddFloat(Vec3 v1, float s){ return (Vec3){v1.x + s, v1.y + s, v1.z + s}; }
static inline Vec3 Math_Vec3AddVec3(Vec3 v1, Vec3 v2){ return (Vec3){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }
static inline Vec3 Math_Vec3SubFloat(Vec3 v1, float s){ return (Vec3){v1.x - s, v1.y - s, v1.z - s}; }
static inline Vec3 Math_Vec3SubVec3(Vec3 v1, Vec3 v2){ return (Vec3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }
static inline float Math_Vec3Magnitude(Vec3 v) { return sqrt(v.x*v.x + v.y*v.y + v.z* v.z); }
static inline float Math_Vec3Dot(Vec3 v1, Vec3 v2){ return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z); }
static inline Vec3 Math_Vec3Cross(Vec3 v1, Vec3 v) { return (Vec3){(v1.y * v.z) - (v1.z * v.y), (v1.z * v.x) - (v1.x * v.z), (v1.x * v.y) - (v1.y * v.x) }; }
static inline Vec2 Math_Vec3ToVec2(Vec3 v){ return (Vec2){v.x, v.y }; }
static inline Vec3 Math_Vec2ToVec3(Vec2 v){ return (Vec3){v.x, v.y, 0}; }
static inline Vec3 Math_RoundVec3(Vec3 v){ return (Vec3){roundf(v.x), roundf(v.y), roundf(v.z)}; }

static inline Vec3 Math_Vec3Reflect(Vec3 v, Vec3 l){
    return Math_Vec3SubVec3(Math_Vec3MultFloat(Math_Vec3MultFloat(l, 2), (Math_Vec3Dot(v, l) / Math_Vec3Dot(l, l))), v);
}

static inline Vec3 Math_Vec3Normalize(Vec3 v){
    float mag = Math_Vec3Magnitude(v);
    v.x /= mag;
    v.y /= mag;
    v.z /= mag;
    return v;
}

static inline Vec3 Math_Vec4ToVec3(Vec4 v){ return (Vec3){v.x, v.y, v.z}; }
static inline Vec2 Math_Vec4ToVec2(Vec4 v){ return (Vec2){v.x, v.y }; }
static inline char Math_Vec3EqualToVec4(Vec4 v1, Vec4 v2){ return (char)(v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w); }
static inline Vec4 Math_Vec4MultFloat(Vec4 v1, float s){ return (Vec4){v1.x * s, v1.y * s, v1.z * s, v1.w * s}; }
static inline Vec4 Math_Vec4MultVec4(Vec4 v1, Vec4 v2){ return (Vec4){v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w}; }
static inline Vec4 Math_Vec4DivideFloat(Vec4 v1, float s){ return (Vec4){v1.x / s, v1.y / s, v1.z / s, v1.w / s}; }
static inline Vec4 Math_Vec4DivideVec4(Vec4 v1, Vec4 v2){ return (Vec4){v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w}; }
static inline Vec4 Math_Vec4AddFloat(Vec4 v1, float s){ return (Vec4){v1.x + s, v1.y + s, v1.z + s, v1.w + s}; }
static inline Vec4 Math_Vec4AddVec4(Vec4 v1, Vec4 v2){ return (Vec4){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w}; }
static inline Vec4 Math_Vec4SubFloat(Vec4 v1, float s){ return (Vec4){v1.x - s, v1.y - s, v1.z - s, v1.w - s}; }
static inline Vec4 Math_Vec4SubVec4(Vec4 v1, Vec4 v2){ return (Vec4){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w}; }
static inline float Math_Vec4Magnitude(Vec4 v) { return sqrt(v.x*v.x + v.y*v.y + v.z* v.z + v.w*v.w); }
static inline Vec4 Math_RoundVec4(Vec4 v){ return (Vec4){roundf(v.x), roundf(v.y), roundf(v.z), roundf(v.w)}; }

static inline Vec4 Math_Vec4Normalize(Vec4 v){
    float mag = Math_Vec4Magnitude(v);
    v.x /= mag;
    v.y /= mag;
    v.z /= mag;
    v.w /= mag;
    return v;
}

static inline Rect Math_Rect(Vec3 pos, float width, float height){ return (Rect){pos.x, pos.y, pos.z, width, height}; }
static inline Vec3 Math_RectXYZ(Rect r){ return (Vec3){r.x, r.y, r.z};  }
static inline Rect2D Math_RoundRect2D(Rect2D r){ return (Rect2D){roundf(r.x), roundf(r.y), roundf(r.w), roundf(r.h) }; }
static inline Rect Math_RoundRect(Rect r){ return (Rect){roundf(r.x), roundf(r.y), roundf(r.z), roundf(r.w), roundf(r.h) }; }

inline char Math_CheckCollisionRect(Rect r1, Rect r){
    if(r1.x <= r.x + r.w  && r1.w  + r1.x >= r.x &&
       r1.y <= r.y + r.h && r1.h + r1.y >= r.y) return 1;

    return 0;
}

inline char Math_CheckCollisionRect2D(Rect2D r1, Rect2D r){
    if(r1.x <= r.x + r.w  && r1.w  + r1.x >= r.x &&
       r1.y <= r.y + r.h && r1.h + r1.y >= r.y) return 1;

    return 0;
}

inline Rect Math_Rect2DToRect(Rect2D r, float d){
    return (Rect){r.x, r.y, d, r.w, r.h};
}

inline char Math_RectIsCompletelyInside(Rect r1, Rect r) {
    if(r1.x >= r.x && r1.x + r1.w <= r.x + r.w && 
       r1.y >= r.y && r1.y + r1.h <= r.y + r.h) return 1;

    return 0;
}

static inline Cube Math_Cube(Vec3 pos, float width, float height, float depth){ return (Cube){pos.x, pos.y, pos.z, width, height, depth}; }
static inline Vec3 Math_CubeXYZ(Cube r){ return (Vec3){r.x, r.y, r.z};  }

inline char Math_CheckCollisionCube(Cube r1, Cube r){
    if(r1.x <= r.x + r.w && r1.w + r1.x >= r.x &&
       r1.y <= r.y + r.h && r1.h + r1.y >= r.y &&
       r1.z <= r.z + r.d && r1.d + r1.z >= r.z) return 1;

    return 0;
}

inline char Math_CubeIsCompletelyInside(Cube r1, Cube r) {
    if(r1.x >= r.x && r1.x + r1.w <= r.x + r.w && 
       r1.y >= r.y && r1.y + r1.h <= r.y + r.h &&
       r1.z >= r.z && r1.z + r1.d <= r.z + r.d) return 1;

    return 0;
}

inline float Math_DistanceToPlane(Plane p, Vec3 point) { return p.a*point.x + p.b*point.y + p.c*point.z + p.d; }
inline double Math_PlaneMagnitude(Plane p){ return sqrt(p.a*p.a + p.b*p.b + p.c*p.c); }
inline void Math_PlaneNormalize(Plane *p){ double mag = Math_PlaneMagnitude(*p); p->a /= mag; p->b /= mag; p->c /= mag; p->d /= mag;}


int Math_CheckFrustumCollision(Cube r, Plane *frustumPlanes);

float Math_GetDistanceFloat(float min1, float max1, float min2, float max2);
Vec2 Math_GetDistanceRect(Rect r1, Rect r2);
Vec3 Math_GetDistanceCube(Cube r1, Cube r2);
int Math_GetDistanceInt(int min1, int max1, int min2, int max2);
float Math_RectCheckCollisionRay(Rect r, Ray ray);
float Math_CubeCheckCollisionRay(Cube r, Ray ray);

void Math_Ik_AddJoint(Arm *arm, float mag, Vec3 rotation);
float Math_Ik_ArmLength(Arm arm);
void Math_Ik_Solve(Arm *arm, Vec3 endPos);
Vec3 Math_LerpVec3(Vec3 a1, Vec3 a2, float t);
float Math_Lerp(float a1, float a2, float t);
float Math_DistanceToLine(Vec2 n, Vec2 o, Vec2 p);
char Math_QuatEqualToQuat(Quat q1, Quat q2);
Quat Math_QuatMult(Quat q1, Quat q2);
Vec3 Math_QuatRotate(Quat q, Vec3 v);
Quat Math_Quat(Vec3 v, float a);
Quat Math_QuatConj(Quat q);
float Math_QuatMag(Quat q);
Quat Math_QuatInv(Quat q);
Quat Math_QuatNormalize(Quat q);
Quat Math_QuatLookAt(Vec3 forward, Vec3 up);
Vec3 Math_RotateMatrixToEuler(float *m);
float Math_Clamp(float m, float min, float max);
void Math_InverseMatrixNxN(float *mat, int n);
float Math_Determinant(float *mat, int n);
void Math_Mat4ToMat3(float *mat4, float *mat3);
void Math_InverseMatrixMat3(float *mat3);
void Math_TransposeMatrix(float *matrix, int n);
Quat Math_MatrixToQuat(float *matrix);
void Math_OuterProduct(Vec3 vec, Vec3 trans, float *matrix);
void Math_RotateMatrix(float *matrix, Vec3 angles);
void Math_Perspective( float *matrix, float fov, float a, float n, float f);
void Math_Perspective2(float *matrix, float l, float r, float t, float b, float n, float f);
void Math_Ortho(float *matrix, float l, float r, float t, float b, float n, float f);
void Math_MatrixMatrixMult(float *res, float *a, float *b);
void Math_LookAt(float *ret, Vec3 eye, Vec3 center, Vec3 up );
void Math_RotateAroundAxis(Vec3 p, float a, float *);
void Math_MatrixFromQuat(Quat q, float*);
Quat Math_Slerp(Quat q, Quat q2, float);
Vec3 Math_MatrixMult(Vec3,float*);
Vec4 Math_MatrixMult4(Vec4,float*);
void Math_CopyMatrix(float *m1, float *m2);
void Math_InverseMatrix(float *m);
void Math_ScaleMatrix(float *matrix, int n, float amount);
void Math_ScalingMatrixXYZ(float *matrix, const Vec3 amount);
Vec3 Math_Rotate(Vec3 pos, Vec3 angles);
void Math_TranslateMatrix(float *matrix, Vec3 vector);
void Math_ScalingMatrix(float *matrix, float amount);
void Math_MatrixMatrixAdd(float *matrix, float *m0, float *m1);
void Math_MatrixMatrixSub(float *matrix, float *m0, float *m1);
Vec3 Math_QuatToAxisAngle(Quat quat, float *angle);
Vec3 Math_AxisAngleToEuler(Vec3 axis, float angle) ;
Vec3 Math_QuatToEuler(Quat quat);
Quat Math_EulerToQuat(const Vec3 euler);
void Math_Identity(float *matrix);
// Vec3 Math_RotateMatrixToEuler(float *m);
Vec3 Math_Ik_GetJointPos(Arm *arm, int jointIndex);

#endif