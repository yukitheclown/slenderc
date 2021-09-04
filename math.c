#include "math.h"
#include "svd.h"
#include <math.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

float Math_GetDistanceFloat(float min1, float max1, float min2, float max2){

    if(max1 < min1) SWAP(min1, max1, float);
    if(max2 < min2) SWAP(min2, max2, float);

    float option1 = max1 - min2;
    float option2 = max2 - min1;
    float option3 = min1 - max2;

    if(min1 < min2)
        if(max1 <= max2)
            return option1;
        else
            if(option1 < option2) return option1; else return -option2;
    else
        if(max1 >= max2)
            return option3;
        else
            if(option1 < option2) return option1; else return -option2;

    return 0;
}

Vec2 Math_GetDistanceRect(Rect r1, Rect r2){
    
    return (Vec2){ Math_GetDistanceFloat(r1.x, r1.x+r1.w, r2.x, r2.x+r2.w), Math_GetDistanceFloat(r1.y, r1.y+r1.h, r2.y, r2.y+r2.h) };
}

Vec3 Math_GetDistanceCube(Cube r1, Cube r2){

    Vec3 ret;
    ret.x = Math_GetDistanceFloat(r1.x, r1.x+r1.w, r2.x, r2.x+r2.w);
    ret.y = Math_GetDistanceFloat(r1.y, r1.y+r1.h, r2.y, r2.y+r2.h);
    ret.z = Math_GetDistanceFloat(r1.z, r1.z+r1.d, r2.z, r2.z+r2.d);
    return ret;
}

int Math_GetDistanceInt(int min1, int max1, int min2, int max2){

    if(max1 < min1) SWAP(min1, max1, int);
    if(max2 < min2) SWAP(min2, max2, int);

    int option1 = max1 - min2;
    int option2 = max2 - min1;
    int option3 = min1 - max2;

    if(min1 < min2)
        if(max1 <= max2)
            return option1;
        else
            if(option1 < option2) return option1; else return -option2;
    else
        if(max1 >= max2)
            return option3;
        else
            if(option1 < option2) return option1; else return -option2;

    return 0;
}


float Math_CubeCheckCollisionRay(Cube r, Ray ray){
    
    float ret = HUGE_VAL;

    Vec3 points[8];

    points[0] = (Vec3){r.x, r.y, r.z};
    points[1] = (Vec3){r.x+r.w, r.y, r.z};
    points[2] = (Vec3){r.x+r.w, r.y+r.h, r.z};
    points[3] = (Vec3){r.x, r.y+r.h, r.z};
    points[4] = (Vec3){r.x, r.y, r.z+r.d};
    points[5] = (Vec3){r.x+r.w, r.y, r.z+r.d};
    points[6] = (Vec3){r.x+r.w, r.y+r.h, r.z+r.d};
    points[7] = (Vec3){r.x, r.y+r.h, r.z+r.d};

    Vec3 rayLine = (Vec3){ray.line.x, ray.line.y, 0};
    Vec3 rayPos = (Vec3){ray.pos.x, ray.pos.y, 0};

    int k;
    for(k = 0; k < 8; k++){
        
        Vec3 norm;
        
        if(k != 7)
            norm = Math_Vec3Normalize(Math_Vec3SubVec3(points[k], points[k+1]));
        else
            norm = Math_Vec3Normalize(Math_Vec3SubVec3(points[k], points[0]));

        float d = Math_Vec3Dot(Math_Vec3SubVec3(points[k], rayPos), norm) / Math_Vec3Dot(rayLine, norm);
        if(d < 0) continue;

        Vec3 collisionPoint = Math_Vec3AddVec3(rayPos, Math_Vec3MultFloat(rayLine,d));

        if(collisionPoint.x >= r.x && collisionPoint.x <= r.x+r.w && collisionPoint.y >= r.y && collisionPoint.y <= r.y+r.h)
            if(d < ret) ret = d;
    }

    return ret;
}

int Math_CheckFrustumCollision(Cube r, Plane *frustumPlanes){

    Vec3 points[8];

    points[0] = (Vec3){r.x, r.y, r.z};
    points[1] = (Vec3){r.x+r.w, r.y, r.z};
    points[2] = (Vec3){r.x+r.w, r.y+r.h, r.z};
    points[3] = (Vec3){r.x, r.y+r.h, r.z};
    points[4] = (Vec3){r.x, r.y, r.z+r.d};
    points[5] = (Vec3){r.x+r.w, r.y, r.z+r.d};
    points[6] = (Vec3){r.x+r.w, r.y+r.h, r.z+r.d};
    points[7] = (Vec3){r.x, r.y+r.h, r.z+r.d};

    int k;
    for(k = 0; k < 6; k++){

        int f;
        for(f = 0; f < 8; f++){

            if(Math_DistanceToPlane(frustumPlanes[k], points[f]) >= 0) break;
        }

        if(f >= 8) return 0;
    }

    return 1;
}

float Math_RectCheckCollisionRay(Rect r, Ray ray){
    
    float ret = HUGE_VAL;

    Vec3 norms[4];
    Vec3 points[4];

    points[0] = (Vec3){r.x, r.y, r.z};
    points[1] = (Vec3){r.x+r.w, r.y, r.z};
    points[2] = (Vec3){r.x+r.w, r.y+r.h, r.z};
    points[3] = (Vec3){r.x, r.y+r.h, r.z};
    norms[0] = (Vec3){0,1,0};
    norms[1] = (Vec3){1,0,0};
    norms[2] = (Vec3){0,-1,0};
    norms[3] = (Vec3){-1,0,0};

    Vec3 rayLine = (Vec3){ray.line.x, ray.line.y, 0};
    Vec3 rayPos = (Vec3){ray.pos.x, ray.pos.y, 0};

    int k;
    for(k = 0; k < 4; k++){
        
        float d = Math_Vec3Dot(Math_Vec3SubVec3(points[k], rayPos), norms[k]) / Math_Vec3Dot(rayLine, norms[k]);
        if(d < 0) continue;

        Vec3 collisionPoint = Math_Vec3AddVec3(rayPos, Math_Vec3MultFloat(rayLine,d));

        if(collisionPoint.x >= r.x && collisionPoint.x <= r.x+r.w && collisionPoint.y >= r.y && collisionPoint.y <= r.y+r.h)
            if(d < ret) ret = d;
    }

    return ret;
}

char Math_QuatEqualToQuat(Quat q1, Quat q2){ return (char)(q1.x == q2.x && q1.y == q2.y && q1.z == q2.z && q1.w == q2.w); }

Quat Math_QuatMult(Quat q1, Quat q2){
    Quat ret = {
        q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
        q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
        q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
        q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z,
    };
    return ret;
}

Vec3 Math_QuatRotate(Quat q, Vec3 v){
    Quat q1 = Math_QuatMult(q, (Quat){v.x, v.y, v.z, 0});
    q1 = Math_QuatMult(q1, (Quat){-q.x, -q.y, -q.z, q.w});

    return (Vec3){q1.x, q1.y, q1.z};
}

Quat Math_Quat(Vec3 v, float a){
    Quat q;
    q.x = v.x * sin(a/2);
    q.y = v.y * sin(a/2);
    q.z = v.z * sin(a/2);
    q.w = cos(a/2);
    return q;
}

Quat Math_QuatConj(Quat q){ return (Quat){-q.x, -q.y, -q.z, q.w}; }

float Math_QuatMag(Quat q){
    return sqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
}

Quat Math_QuatInv(Quat q){
    Quat conjugate = Math_QuatConj(q);
    float mag = pow(Math_QuatMag(q), 2);
    conjugate.x /= mag;
    conjugate.y /= mag;
    conjugate.z /= mag;
    conjugate.w /= mag;
    return conjugate;
}

Quat Math_QuatNormalize(Quat q){
    float mag = Math_QuatMag(q);
    q.x /= mag;
    q.y /= mag;
    q.z /= mag;
    q.w /= mag;
    return q;
}

void Math_Ortho(float *matrix, float l, float r, float t, float b, float n, float f){
    float m[16] = {
        2/(r-l),    0,           0,             -((r+l)/(r-l)),
        0,          2/(t-b),     0,             -((t+b)/(t-b)),
        0,          0,           1/(f-n),       n/(f-n),
        0,          0,           0,              1
    };

    memcpy(matrix, m, sizeof(m));
}

void Math_Perspective2(float *matrix, float l, float r, float t, float b, float n, float f){
    float m[16] = {
        (2*n)/(r-l),    0,               (r+l)/(r-l),      0,
        0,              (2*n)/(t-b),     (t+b)/(t-b),      0,
        0,              0,               -(f+n)/(f-n),    -(2*f*n)/(f-n),
        0,              0,               -1,               0
    };

    memcpy(matrix, m, sizeof(m));
}

void Math_Identity(float *matrix){
    memset(matrix,0, sizeof(float) * 16);
    matrix[0] = 1;
    matrix[5] = 1;
    matrix[10] = 1;
    matrix[15] = 1;
}

void Math_Perspective( float *matrix, float fov, float a, float n, float f){
    float m[4][4] = {};

    float tanHalfFov = tan(fov * 0.5f);

    m[0][0] = 1. / (a * tanHalfFov);
    m[1][1] = 1. / tanHalfFov;
    m[2][2] = f / (n - f);
    m[3][2] = -1.;
    m[2][3] = (f * n) / (n - f);
    m[3][3] = 0.0f;
    memcpy(matrix, m, sizeof(m));
}

void Math_MatrixMatrixMultNxN(float *ret, float *a, int aSizeX, int aSizeY, float *b, int bSizeX, int bSizeY){
    
    int m = aSizeY, n = aSizeX;
    int p = bSizeY, q = bSizeX;

    int c, d, k;
    for (c = 0; c < m; c++) {
        for (d = 0; d < q; d++) {

            float sum = 0;

            for (k = 0; k < p; k++)
                sum += a[(c*n)+k]*b[(k*q)+d];

            ret[(c*q)+d] = sum;
        }
    }
}

void Math_MatrixMatrixMult(float *res, float *a, float *b){

    float r[16] = {
        (a[0]  * b[0]) + (a[1]  *  b[4]) + (a[2]  * b[8])  + (a[3]  * b[12]),
        (a[0]  * b[1]) + (a[1]  *  b[5]) + (a[2]  * b[9])  + (a[3]  * b[13]),
        (a[0]  * b[2]) + (a[1]  *  b[6]) + (a[2]  * b[10]) + (a[3]  * b[14]),
        (a[0]  * b[3]) + (a[1]  *  b[7]) + (a[2]  * b[11]) + (a[3]  * b[15]),
        (a[4]  * b[0]) + (a[5]  *  b[4]) + (a[6]  * b[8])  + (a[7]  * b[12]),
        (a[4]  * b[1]) + (a[5]  *  b[5]) + (a[6]  * b[9])  + (a[7]  * b[13]),
        (a[4]  * b[2]) + (a[5]  *  b[6]) + (a[6]  * b[10]) + (a[7]  * b[14]),
        (a[4]  * b[3]) + (a[5]  *  b[7]) + (a[6]  * b[11]) + (a[7]  * b[15]),
        (a[8]  * b[0]) + (a[9]  *  b[4]) + (a[10] * b[8])  + (a[11] * b[12]),
        (a[8]  * b[1]) + (a[9]  *  b[5]) + (a[10] * b[9])  + (a[11] * b[13]),
        (a[8]  * b[2]) + (a[9]  *  b[6]) + (a[10] * b[10]) + (a[11] * b[14]),
        (a[8]  * b[3]) + (a[9]  *  b[7]) + (a[10] * b[11]) + (a[11] * b[15]),
        (a[12] * b[0]) + (a[13] *  b[4]) + (a[14] * b[8])  + (a[15] * b[12]),
        (a[12] * b[1]) + (a[13] *  b[5]) + (a[14] * b[9])  + (a[15] * b[13]),
        (a[12] * b[2]) + (a[13] *  b[6]) + (a[14] * b[10]) + (a[15] * b[14]),
        (a[12] * b[3]) + (a[13] *  b[7]) + (a[14] * b[11]) + (a[15] * b[15]),
    };
    
    memcpy(res, r, sizeof(r));
}

void Math_LookAt(float *ret, Vec3 eye, Vec3 center, Vec3 up ){

    Vec3 z = Math_Vec3Normalize(Math_Vec3SubVec3(eye, center));  // Forward
    Vec3 x = Math_Vec3Normalize(Math_Vec3Cross(up, z)); // Right
    Vec3 y = Math_Vec3Cross(z, x);

    float m[] = { x.x,  x.y,  x.z,  -(Math_Vec3Dot(x, eye)),
                  y.x,  y.y,  y.z,  -(Math_Vec3Dot(y, eye)),
                  z.x,  z.y,  z.z,  -(Math_Vec3Dot(z, eye)),
                  0,    0,    0,    1 };
    memcpy(ret, m, sizeof(m));
}

void Math_OuterProduct(Vec3 v, Vec3 t, float *matrix){
    float m[16] = {
        v.x*t.x, v.x*t.y, v.x*t.z, 0,
        v.y*t.x, v.y*t.y, v.y*t.z, 0,
        v.z*t.x, v.z*t.y, v.z*t.z, 0,
        0, 0, 0, 1,
    };
    memcpy(matrix, m, sizeof(m));
}

static float determinant(float *a,int k, int originalSize)
{
    float s = 1, det=0, b[originalSize][originalSize];

    int i,j,m,n,c;
    if (k==1) return (a[0]);

    det=0;
    for (c=0;c<k;c++){
        m=0;
        n=0;
        for (i=0;i<k;i++){
            for (j=0;j<k;j++){
                b[i][j]=0;
                if (i != 0 && j != c){
                    b[m][n]=a[(i*originalSize)+j];
                    if (n<(k-2))
                        n++;
                    else{
                        n=0;
                        m++;
                    }
                }
            }
        }

        det=det + s * (a[c] * determinant(&b[0][0], k-1, originalSize));
        s=-1 * s;
    }
 
    return (det);
}
 
static void transpose(float *ret, float *mat,float *fac, int r, int originalSize){
  
  int i,j;

  float b[r][r],d;

  for (i=0;i<r;i++){
     for (j=0;j<r;j++){
         b[i][j]=fac[(j*r)+i];
        }
    }
  
  d=determinant(mat,r,originalSize);

  for (i=0;i<r;i++){
     for (j=0;j<r;j++){
        ret[(i*(int)r)+j] = b[i][j] / d;
        }
    }
}

static void InverseNxN(float *ret, float *mat, int f, int originalSize){

    float b[f][f],fac[f][f];

    int p,q,m,n,i,j;

    for (q=0;q<f;q++){
        for (p=0;p<f;p++){
            m=0;
            n=0;
            for (i=0;i<f;i++){
                for (j=0;j<f;j++){
                    if (i != q && j != p){
                        b[m][n]=mat[(i*originalSize)+j];
                        if (n<(f-2))
                            n++;
                        else{
                            n=0;
                            m++;
                        }
                    }
                }
            }

            fac[q][p]=pow(-1,q + p) * determinant(&b[0][0], f-1, f);
        }
    }

    transpose(ret, mat, &fac[0][0], f, originalSize);
}

void Math_InverseMatrixNxN(float *mat, int n){

    float *tmp = (float *)malloc(n*n*sizeof(float));
    
    InverseNxN(tmp, mat, n, n);
    
    memcpy(mat, tmp, n*n*sizeof(float));
    
    if(tmp) free(tmp);
}

void Math_InverseMatrix(float *m){
    float m00 = m[ 0], m01 = m[ 1], m02 = m[ 2], m03 = m[ 3];
    float m10 = m[ 4], m11 = m[ 5], m12 = m[ 6], m13 = m[ 7];
    float m20 = m[ 8], m21 = m[ 9], m22 = m[10], m23 = m[11];
    float m30 = m[12], m31 = m[13], m32 = m[14], m33 = m[15];

    m[ 0] = m12*m23*m31 - m13*m22*m31 + m13*m21*m32 - m11*m23*m32 - m12*m21*m33 + m11*m22*m33;
    m[ 1] = m03*m22*m31 - m02*m23*m31 - m03*m21*m32 + m01*m23*m32 + m02*m21*m33 - m01*m22*m33;
    m[ 2] = m02*m13*m31 - m03*m12*m31 + m03*m11*m32 - m01*m13*m32 - m02*m11*m33 + m01*m12*m33;
    m[ 3] = m03*m12*m21 - m02*m13*m21 - m03*m11*m22 + m01*m13*m22 + m02*m11*m23 - m01*m12*m23;
    m[ 4] = m13*m22*m30 - m12*m23*m30 - m13*m20*m32 + m10*m23*m32 + m12*m20*m33 - m10*m22*m33;
    m[ 5] = m02*m23*m30 - m03*m22*m30 + m03*m20*m32 - m00*m23*m32 - m02*m20*m33 + m00*m22*m33;
    m[ 6] = m03*m12*m30 - m02*m13*m30 - m03*m10*m32 + m00*m13*m32 + m02*m10*m33 - m00*m12*m33;
    m[ 7] = m02*m13*m20 - m03*m12*m20 + m03*m10*m22 - m00*m13*m22 - m02*m10*m23 + m00*m12*m23;
    m[ 8] = m11*m23*m30 - m13*m21*m30 + m13*m20*m31 - m10*m23*m31 - m11*m20*m33 + m10*m21*m33;
    m[ 9] = m03*m21*m30 - m01*m23*m30 - m03*m20*m31 + m00*m23*m31 + m01*m20*m33 - m00*m21*m33;
    m[10] = m01*m13*m30 - m03*m11*m30 + m03*m10*m31 - m00*m13*m31 - m01*m10*m33 + m00*m11*m33;
    m[11] = m03*m11*m20 - m01*m13*m20 - m03*m10*m21 + m00*m13*m21 + m01*m10*m23 - m00*m11*m23;
    m[12] = m12*m21*m30 - m11*m22*m30 - m12*m20*m31 + m10*m22*m31 + m11*m20*m32 - m10*m21*m32;
    m[13] = m01*m22*m30 - m02*m21*m30 + m02*m20*m31 - m00*m22*m31 - m01*m20*m32 + m00*m21*m32;
    m[14] = m02*m11*m30 - m01*m12*m30 - m02*m10*m31 + m00*m12*m31 + m01*m10*m32 - m00*m11*m32;
    m[15] = m01*m12*m20 - m02*m11*m20 + m02*m10*m21 - m00*m12*m21 - m01*m10*m22 + m00*m11*m22;
    float det = (m00 * m[0]) + (m01 * m[4]) + (m02 * m[8]) + (m03 * m[12]); 
    Math_ScaleMatrix(m, 4, 1 / det);
}

void Math_Mat4ToMat3(float *mat4, float *mat3){
    mat3[0] = mat4[0];
    mat3[1] = mat4[1];
    mat3[2] = mat4[2];
    mat3[3] = mat4[4];
    mat3[4] = mat4[5];
    mat3[5] = mat4[6];
    mat3[6] = mat4[8];
    mat3[7] = mat4[9];
    mat3[8] = mat4[10];
}

void Math_InverseMatrixMat3(float *m){
    float matrix[16] = {
        m[0], m[1], m[2], 0,
        m[3], m[4], m[5], 0,
        m[6], m[7], m[8], 0,
        0, 0, 0, 1
    };
    Math_InverseMatrix(matrix);
    Math_Mat4ToMat3(matrix, m);
}

void Math_ScaleMatrix(float *matrix, int n, float amount){
    int k;
    for(k = 0; k < n*n; k++) matrix[k] *= amount;
}

void Math_ScalingMatrix(float *matrix, float amount){
    Math_ScalingMatrixXYZ(matrix, (Vec3){amount, amount, amount});
}

void Math_ScalingMatrixXYZ(float *matrix, const Vec3 amount){
    float m[16] = {
        amount.x, 0, 0, 0,
        0, amount.y, 0, 0,
        0, 0, amount.z, 0,
        0, 0, 0, 1 };

    memcpy(matrix, m, sizeof(m) );
}

Vec3 Math_MatrixMult( Vec3 vert, float *matrix){
    Vec3 out;
    out.x = ((vert.x * matrix[0])  + (vert.y * matrix[1])  + (vert.z * matrix[2])  + (1 * matrix[3]));
    out.y = ((vert.x * matrix[4])  + (vert.y * matrix[5])  + (vert.z * matrix[6])  + (1 * matrix[7]));
    out.z = ((vert.x * matrix[8])  + (vert.y * matrix[9])  + (vert.z * matrix[10]) + (1 * matrix[11]));
    return out;
}

Vec4 Math_MatrixMult4( Vec4 vert, float *matrix){
    Vec4 out;
    out.x = ((vert.x * matrix[0])  + (vert.y * matrix[1])  + (vert.z * matrix[2])  + (vert.w * matrix[3]));
    out.y = ((vert.x * matrix[4])  + (vert.y * matrix[5])  + (vert.z * matrix[6])  + (vert.w * matrix[7]));
    out.z = ((vert.x * matrix[8])  + (vert.y * matrix[9])  + (vert.z * matrix[10]) + (vert.w * matrix[11]));
    out.w = ((vert.x * matrix[12]) + (vert.y * matrix[13]) + (vert.z * matrix[14]) + (vert.w * matrix[15]));
    return out;
}

Vec3 Math_LerpVec3(Vec3 a1, Vec3 a2, float t){
    return Math_Vec3AddVec3(Math_Vec3MultFloat(a1,(1.0-t)), Math_Vec3MultFloat(a2, t));
}

float Math_Lerp(float a1, float a2, float t){
    return (a1*(1.0-t)) + (a2 * t);
}

// float Math_DistanceToLine(Vec2 p1, Vec2 p2, Vec2 p){

//     float dx = p2.x - p1.x;
//     float dy = p2.y - p1.y;

//     return ((dy*p.x) - (dx*p.y) + (p2.x*p1.y) - (p2.y*p1.x)) / sqrt(dy*dy + dx*dx);
// }

float Math_DistanceToLine(Vec2 n, Vec2 o, Vec2 p){ return Math_Vec2Dot(n, Math_Vec2SubVec2(o, p)); }


Quat Math_Slerp(Quat qa, Quat qb, float t){
    Quat qm;
    float cosHalfTheta = qa.w*qb.w + qa.x*qb.x + qa.y*qb.y + qa.z*qb.z;
    if(cosHalfTheta < 0){
        qb.x *= -1; qb.y *= -1; qb.z *= -1; qb.w *= -1;
        cosHalfTheta *= -1; 
    }
    if(fabs(cosHalfTheta) >= 1.0){
        qm.w = qa.w; 
        qm.x = qa.x; 
        qm.y = qa.y;
        qm.z = qa.z;
        return qm;
    }
    float halfTheta = acos(cosHalfTheta);
    float sinHalfTheta = sin(halfTheta);
    if(fabs(sinHalfTheta) < 0.001){
        qm.w = (qa.w * 0.5 + qb.w * 0.5);
        qm.x = (qa.x * 0.5 + qb.x * 0.5);
        qm.y = (qa.y * 0.5 + qb.y * 0.5);
        qm.z = (qa.z * 0.5 + qb.z * 0.5);
        return qm;
    }
    float ratioA = sin((1-t) * halfTheta) / sinHalfTheta;
    float ratioB = sin(t * halfTheta) / sinHalfTheta;
    qm.w = (qa.w * ratioA + qb.w * ratioB);
    qm.x = (qa.x * ratioA + qb.x * ratioB);
    qm.y = (qa.y * ratioA + qb.y * ratioB);
    qm.z = (qa.z * ratioA + qb.z * ratioB);
    return qm;
}

Quat Math_SlerpTheta(Quat qa, Quat qb, float theta){
    Quat qm;
    float cosHalfTheta = qa.w*qb.w + qa.x*qb.x + qa.y*qb.y + qa.z*qb.z;
    if(cosHalfTheta < 0){
        qb.x *= -1; qb.y *= -1; qb.z *= -1; qb.w *= -1;
        cosHalfTheta *= -1; 
    }
    if(fabs(cosHalfTheta) >= 1.0){
        qm.w = qa.w; 
        qm.x = qa.x; 
        qm.y = qa.y;
        qm.z = qa.z;
        return qm;
    }
    float halfTheta = acos(cosHalfTheta);
    float sinHalfTheta = sin(halfTheta);
    if(fabs(sinHalfTheta) < 0.001){
        qm.w = (qa.w * 0.5 + qb.w * 0.5);
        qm.x = (qa.x * 0.5 + qb.x * 0.5);
        qm.y = (qa.y * 0.5 + qb.y * 0.5);
        qm.z = (qa.z * 0.5 + qb.z * 0.5);
        return qm;
    }
    float ratioA = sin((halfTheta-(theta/2.0))) / sinHalfTheta;
    float ratioB = sin((theta/2.0)) / sinHalfTheta;
    qm.w = (qa.w * ratioA + qb.w * ratioB);
    qm.x = (qa.x * ratioA + qb.x * ratioB);
    qm.y = (qa.y * ratioA + qb.y * ratioB);
    qm.z = (qa.z * ratioA + qb.z * ratioB);
    return qm;
}

void Math_MatrixFromQuat(Quat q, float *matrix){
    float qx = q.x;
    float qy = q.y;
    float qz = q.z;
    float qw = q.w;
    float n = 1.0f/sqrt(qx*qx+qy*qy+qz*qz+qw*qw);
    qx *= n; qy *= n; qz *= n; qw *= n;
    float r[16] = {
        1.0f - 2.0f*qy*qy - 2.0f*qz*qz, 2.0f*qx*qy - 2.0f*qz*qw, 2.0f*qx*qz + 2.0f*qy*qw, 0.0f,
        2.0f*qx*qy + 2.0f*qz*qw, 1.0f - 2.0f*qx*qx - 2.0f*qz*qz, 2.0f*qy*qz - 2.0f*qx*qw, 0.0f,
        2.0f*qx*qz - 2.0f*qy*qw, 2.0f*qy*qz + 2.0f*qx*qw, 1.0f - 2.0f*qx*qx - 2.0f*qy*qy, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    memcpy(matrix, r, sizeof(r));
}

void Math_TranslateMatrix(float *matrix, Vec3 vector){

    float r[16] = {
        1, 0, 0, vector.x,
        0, 1, 0, vector.y,
        0, 0, 1, vector.z,
        0, 0, 0, 1,
    };

    memcpy(matrix, r, sizeof(r));
}

void Math_TransposeMatrix(float *matrix, int n){

    float r[n][n];
    float r2[n][n];
    memcpy(r2, matrix, sizeof(float) * n*n);

    int k, f;
    for(k = 0; k < n; k++)
        for(f = 0; f < n; f++)
            r[k][f] = r2[f][k];

    memcpy(matrix, r, n*n*sizeof(float));
}

void Math_CopyMatrix(float *m1, float *m2){
    int k;
    for(k = 0; k < 16; k++)
        m1[k] = m2[k];
}

Quat Math_QuatLookAt(Vec3 forward, Vec3 up){

    Vec3 forwardW = {0,0,-1};

    up = Math_Vec3Normalize(up);

    Vec3 axis = Math_Vec3Normalize(Math_Vec3Cross(forward, forwardW));

    float theta = acos(Math_Vec3Dot(forward, forwardW));

    Vec3 b = Math_Vec3Normalize(Math_Vec3Cross(axis, forwardW));

    if(Math_Vec3Dot(b, forward) < 0) theta = -theta;

    Quat qb = Math_Quat(axis, theta);

    Vec3 upL = Math_Vec3Normalize(Math_QuatRotate(qb, up));
    Vec3 right = Math_Vec3Normalize(Math_Vec3Cross(forward, up));
    Vec3 upW = Math_Vec3Normalize(Math_Vec3Cross(right, forward));

    axis = Math_Vec3Cross(upL, upW);

    theta = acos(Math_Vec3Dot(upL, upW));

    // b = Math_Vec3Cross(axis, upW);

    // if(Math_Vec3Dot(b, upL) < 0) theta = -theta;

    return Math_QuatMult(Math_Quat(axis, theta), qb);
}

void Math_RotateMatrix(float *matrix, Vec3 angles){
    float sx = sin(angles.x);
    float cx = cos(angles.x);
    float sy = sin(angles.y);
    float cy = cos(angles.y);
    float sz = sin(angles.z);
    float cz = cos(angles.z);

    float m[16] = {
        cy*cz,(-cy*sz*cx) + (sy*sx),(cy*sz*sx) + (sy*cx),0,
        sz,cz*cx,-cz*sx,0,
        -sy*cz,(sy*sz*cx) + (cy*sx),(-sy*sz*sx) + (cy*cx),0,
        0,0,0,1
    };

    memcpy(matrix, m, sizeof(m));
}

Vec3 Math_Rotate(Vec3 pos, Vec3 angles){
    float matrix[16];
    Math_RotateMatrix(matrix, angles);
    return Math_MatrixMult(pos, matrix);
}

void Math_RotateAroundAxis(Vec3 p, float a, float *matrix){
    float qx = p.x * sin(a/2);
    float qy = p.y * sin(a/2);
    float qz = p.z * sin(a/2);
    float qw = cos(a/2);
    float n = 1.0f/sqrt(qx*qx+qy*qy+qz*qz+qw*qw);
    qx *= n; qy *= n; qz *= n; qw *= n;
    float r[16] = {
        1.0f - 2.0f*qy*qy - 2.0f*qz*qz, 2.0f*qx*qy - 2.0f*qz*qw, 2.0f*qx*qz + 2.0f*qy*qw, 0.0f,
        2.0f*qx*qy + 2.0f*qz*qw, 1.0f - 2.0f*qx*qx - 2.0f*qz*qz, 2.0f*qy*qz - 2.0f*qx*qw, 0.0f,
        2.0f*qx*qz - 2.0f*qy*qw, 2.0f*qy*qz + 2.0f*qx*qw, 1.0f - 2.0f*qx*qx - 2.0f*qy*qy, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    memcpy(matrix, r, sizeof(r));
}

void Math_MatrixMatrixAdd(float *matrix, float *m0, float *m1){
    float m[16] = {
        m0[0]+m1[0],       m0[1]+m1[1],       m0[2]+m1[2],       m0[3]+m1[3],
        m0[4+0]+m1[4+0],   m0[4+1]+m1[4+1],   m0[4+2]+m1[4+2],   m0[4+3]+m1[4+3],
        m0[8+0]+m1[8+0],   m0[8+1]+m1[8+1],   m0[8+2]+m1[8+2],   m0[8+3]+m1[8+3],
        m0[12+0]+m1[12+0], m0[12+1]+m1[12+1], m0[12+2]+m1[12+2], m0[12+3]+m1[12+3],
    };

    memcpy(matrix, m, sizeof(m));
}

void Math_MatrixMatrixSub(float *matrix, float *m0, float *m1){
    float m[16] = {
        m0[0]-m1[0], m0[1]-m1[1], m0[2]-m1[2], m0[3]-m1[3],
        m0[4+0]-m1[4+0], m0[4+1]-m1[4+1], m0[4+2]-m1[4+2], m0[4+3]-m1[4+3],
        m0[8+0]-m1[8+0], m0[8+1]-m1[8+1], m0[8+2]-m1[8+2], m0[8+3]-m1[8+3],
        m0[12+0]-m1[12+0], m0[12+1]-m1[12+1], m0[12+2]-m1[12+2], m0[12+3]-m1[12+3],
    };

    memcpy(matrix, m, sizeof(m));
}

Quat Math_MatrixToQuat(float *m){
    Quat q;

    float tr = m[0] + m[5] + m[10];

    if (tr > 0) { 
        float S = sqrt(tr+1.0) * 2;
        q.w = 0.25 * S;
        q.x = (m[9] - m[6]) / S;
        q.y = (m[2] - m[8]) / S; 
        q.z = (m[4] - m[1]) / S; 
    } else if ((m[0] > m[5])&(m[0] > m[10])) { 
        float S = sqrt(1.0 + m[0] - m[5] - m[10]) * 2;
        q.w = (m[9] - m[6]) / S;
        q.x = 0.25 * S;
        q.y = (m[1] + m[4]) / S; 
        q.z = (m[2] + m[8]) / S; 
    } else if (m[5] > m[10]) { 
        float S = sqrt(1.0 + m[5] - m[0] - m[10]) * 2;
        q.w = (m[2] - m[8]) / S;
        q.x = (m[1] + m[4]) / S; 
        q.y = 0.25 * S;
        q.z = (m[6] + m[9]) / S; 
    } else { 
        float S = sqrt(1.0 + m[10] - m[0] - m[5]) * 2;
        q.w = (m[4] - m[1]) / S;
        q.x = (m[2] + m[8]) / S;
        q.y = (m[6] + m[9]) / S;
        q.z = 0.25 * S;
    }

    return q;
}

Vec3 Math_QuatToAxisAngle(Quat quat, float *angle){

    if(quat.w > 1) quat = Math_QuatNormalize(quat);

    if(angle) *angle = 2 * acos(quat.w);

    float s = sqrt(1.0 - (quat.w*quat.w));
    
    if(s < 0.001)
        return (Vec3){quat.x, quat.y, quat.z};

    return (Vec3){quat.x / s, quat.y / s, quat.z / s};
}

Vec3 Math_AxisAngleToEuler(Vec3 axis, float angle) {
    float s=sin(angle);
    float c=cos(angle);
    float t=1-c;

    axis = Math_Vec3Normalize(axis);

    Vec3 ret;

    if ((axis.x*axis.y*t + axis.z*s) > 0.998) { // north pole singularity detected
    
        ret.y = 2*atan2(axis.x*sin(angle/2),cos(angle/2));
        ret.z = PI/2;
        ret.x = 0;
        return ret;
    }
    if ((axis.x*axis.y*t + axis.z*s) < -0.998) { // south pole singularity detected
        ret.y = -2*atan2(axis.x*sin(angle/2),cos(angle/2));
        ret.z = -PI/2;
        ret.x = 0;
        return ret;
    }
    ret.y = atan2(axis.y * s- axis.x * axis.z * t , 1 - (axis.y*axis.y+ axis.z*axis.z ) * t);
    ret.z = asin(axis.x * axis.y * t + axis.z * s) ;
    ret.x = atan2(axis.x * s - axis.y * axis.z * t , 1 - (axis.x*axis.x + axis.z*axis.z) * t);

    return ret;
}

Vec3 Math_RotateMatrixToEuler(float *m){
    // Assuming the angles are in radians.
    Vec3 ret;
    if (m[4] > 0.998) { // singularity at north pole
        ret.y = atan2(m[2],m[10]);
        ret.z = PI/2;
        ret.x = 0;
        return ret;
    }
    if (m[4] < -0.998) { // singularity at south pole
        ret.y = atan2(m[2],m[10]);
        ret.z = -PI/2;
        ret.x = 0;
        return ret;
    }

    ret.y = atan2(-m[8],m[0]);
    ret.x = atan2(-m[6],m[5]);
    ret.z = asin(m[4]);
    return ret;
}

Quat Math_EulerToQuat(const Vec3 euler){

    float c1 = cos(euler.y);
    float s1 = sin(euler.y);
    float c2 = cos(euler.z);
    float s2 = sin(euler.z);
    float c3 = cos(euler.x);
    float s3 = sin(euler.x);

    Quat ret;
    ret.w = sqrt(1.0 + c1 * c2 + c1*c3 - s1 * s2 * s3 + c2*c3) / 2.0;
    float w4 = (4.0 * ret.w);
    ret.x = (c2 * s3 + c1 * s3 + s1 * s2 * c3) / w4 ;
    ret.y = (s1 * c2 + s1 * c3 + c1 * s2 * s3) / w4 ;
    ret.z = (-s1 * s3 + c1 * s2 * c3 +s2) / w4 ;

    return ret;
}

Vec3 Math_QuatToEuler(Quat quat){
    float sqw = quat.w*quat.w;
    float sqx = quat.x*quat.x;
    float sqy = quat.y*quat.y;
    float sqz = quat.z*quat.z;
    float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
    float test = quat.x*quat.y + quat.z*quat.w;

    Vec3 ret;

    if (test > 0.499*unit) { // singularity at north pole
        ret.y = 2 * atan2(quat.x,quat.w);
        ret.z = PI/2;
        ret.x = 0;
        return ret;
    }
    if (test < -0.499*unit) { // singularity at south pole
        ret.y = -2 * atan2(quat.x,quat.w);
        ret.z = -PI/2;
        ret.x = 0;
        return ret;
    }
    ret.y = atan2(2*quat.y*quat.w-2*quat.x*quat.z , sqx - sqy - sqz + sqw);
    ret.z = asin(2*test/unit);
    ret.x = atan2(2*quat.x*quat.w-2*quat.y*quat.z , -sqx + sqy - sqz + sqw);

    return ret;
}

Vec3 Math_Ik_GetArmEnd(Arm *arm){

    Vec3 ret = {0,0,0};

    int k;
    for(k = 0; k < arm->nJoints; k++)
        ret = Math_Vec3AddVec3(ret, Math_QuatRotate(arm->joints[k].quat, (Vec3){0,arm->joints[k].mag,0}));
 
    return ret;
}

Vec3 Math_Ik_GetJointPos(Arm *arm, int jointIndex){

    Vec3 ret = arm->basePos;

    int k;
    for(k = 0; k < jointIndex; k++)
        ret = Math_Vec3AddVec3(ret, Math_QuatRotate(arm->joints[k].quat, (Vec3){0,arm->joints[k].mag,0}));
 
    return ret;
}

Vec3 Math_Ik_GetJointRotation(Arm *arm, int jointIndex){

    Quat ret = {0,0,0,1};

    int k;
    for(k = 0; k < jointIndex; k++)
        ret = Math_QuatMult(ret, arm->joints[k].quat);

    return Math_Vec3SubVec3(Math_QuatToEuler(arm->joints[jointIndex].quat), Math_QuatToEuler(ret));
}

float Math_Ik_ArmLength(Arm arm){

    float ret = 0;

    int k;
    for(k = 0; k < arm.nJoints; k++)
        ret += arm.joints[k].mag;

    return ret;
}

Vec3 Math_Ik_GetJacobianColumn(Arm *arm, Joint *joint, Vec3 axis, Vec3 relativeGoal){

    Vec3 start = Math_Ik_GetArmEnd(arm);

    Vec3 tAxis = Math_QuatRotate(joint->quat, axis);

    Quat quat = Math_Quat(tAxis, EPSILON);

    Quat tmpQuat = joint->quat;

    joint->quat = Math_QuatMult(quat, joint->quat);

    Vec3 diff = Math_Vec3SubVec3(Math_Ik_GetArmEnd(arm), start);

    Vec3 columnn = Math_Vec3DivideFloat(diff, EPSILON);

    joint->quat = tmpQuat;

    return columnn;
}

void Math_Ik_AddJoint(Arm *arm, float mag, Vec3 rotation){
    arm->joints = realloc(arm->joints, ++arm->nJoints * sizeof(Joint));
    arm->joints[arm->nJoints-1].mag = mag;
    arm->joints[arm->nJoints-1].quat = Math_EulerToQuat(rotation);
}

void Math_Ik_Solve(Arm *arm, Vec3 endPos){

    endPos = Math_Vec3SubVec3(endPos, arm->basePos);

    float length = Math_Ik_ArmLength(*arm);

    if(Math_Vec3Magnitude(endPos) > length)
        endPos = Math_Vec3MultFloat(Math_Vec3Normalize(endPos), length);

    int jacobianSize = arm->nJoints * 3;

    float *jacobian = (float *)malloc(sizeof(float) * 3 * jacobianSize);

    Vec3 dE = Math_Vec3SubVec3(endPos, Math_Ik_GetArmEnd(arm));

    float diff = Math_Vec3Magnitude(dE);

    int k;
    for(k = 0; k < 20 && diff > 1; k++){

        int k;
        for(k = 0; k < jacobianSize; k+=3){
            Vec3 columnnX = Math_Ik_GetJacobianColumn(arm, &arm->joints[k/3], (Vec3){1,0,0}, endPos);
            Vec3 columnnY = Math_Ik_GetJacobianColumn(arm, &arm->joints[k/3], (Vec3){0,1,0}, endPos);
            Vec3 columnnZ = Math_Ik_GetJacobianColumn(arm, &arm->joints[k/3], (Vec3){0,0,1}, endPos);
                
            jacobian[k] = columnnX.x;
            jacobian[jacobianSize+k] = columnnX.y;
            jacobian[(jacobianSize*2)+k] = columnnX.z;

            jacobian[(k+1)] = columnnY.x;
            jacobian[jacobianSize+(k+1)] = columnnY.y;
            jacobian[(jacobianSize*2)+(k+1)] = columnnY.z;

            jacobian[(k+2)] = columnnZ.x;
            jacobian[jacobianSize+(k+2)] = columnnZ.y;
            jacobian[(jacobianSize*2)+(k+2)] = columnnZ.z;
        }

        SVD_PseudoInverse(jacobian,jacobian,3,jacobianSize);

        float angles[jacobianSize];
        memset(angles, 0, sizeof(angles));
        Math_MatrixMatrixMultNxN(angles, jacobian, 3, jacobianSize, &dE.x, 1, 3);

        for(k = 0; k < jacobianSize; k+=3){

            Joint *joint = &arm->joints[k/3];

            Vec3 axes[3] = { {1,0,0}, {0,1,0}, {0,0,1} };            

            int j;
            for(j = 0; j < 3; j++){
                if(!isnan(angles[j+k])){
                    Vec3 tAxis = Math_QuatRotate(joint->quat, axes[j]);
                    joint->quat = Math_QuatMult(Math_Quat(tAxis, angles[k+j]), joint->quat);
                }
            }
        }

        dE = Math_Vec3SubVec3(endPos, Math_Ik_GetArmEnd(arm));
        float newDiff = Math_Vec3Magnitude(dE);

        if(newDiff > diff)
            break;

        diff = newDiff;
    }

    if(jacobian) free(jacobian);
}