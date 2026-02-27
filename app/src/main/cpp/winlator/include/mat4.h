#ifndef MAT4_H
#define MAT4_H

#include <malloc.h>
#include <math.h>

#define MAT4_SIZE 16 * sizeof(float)
#define IDENTITY_MATRIX {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}

#define TO_RADIANS(x) (x * (M_PI / 180.0f))
#define TO_DEGREES(x) (x * (180.0f / M_PI))

static inline void mat4_identity(float matrix[16]) {
    matrix[0] = 1;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;
    matrix[4] = 0;
    matrix[5] = 1;
    matrix[6] = 0;
    matrix[7] = 0;
    matrix[8] = 0;
    matrix[9] = 0;
    matrix[10] = 1;
    matrix[11] = 0;
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;
}

static inline float* mat4_alloc() {
    float* matrix = malloc(MAT4_SIZE);
    mat4_identity(matrix);
    return matrix;
}

static inline float* mat4_clone(const float* srcMatrix) {
    void* dstMatrix = malloc(MAT4_SIZE);
    memcpy(dstMatrix, srcMatrix, MAT4_SIZE);
    return dstMatrix;
}

static inline void mat4_scale(float result[16], float matrix[16], float x, float y, float z) {
    result[0] = matrix[0] * x;
    result[1] = matrix[1] * x;
    result[2] = matrix[2] * x;
    result[3] = matrix[3] * x;
    result[4] = matrix[4] * y;
    result[5] = matrix[5] * y;
    result[6] = matrix[6] * y;
    result[7] = matrix[7] * y;
    result[8] = matrix[8] * z;
    result[9] = matrix[9] * z;
    result[10] = matrix[10] * z;
    result[11] = matrix[11] * z;
    result[12] = matrix[12];
    result[13] = matrix[13];
    result[14] = matrix[14];
    result[15] = matrix[15];    
}

static inline void mat4_translate(float result[16], float matrix[16], float x, float y, float z) {
    float a00, a01, a02, a03,
          a10, a11, a12, a13,
          a20, a21, a22, a23;

    if (matrix == result) {
        result[12] = matrix[0] * x + matrix[4] * y + matrix[8] * z + matrix[12];
        result[13] = matrix[1] * x + matrix[5] * y + matrix[9] * z + matrix[13];
        result[14] = matrix[2] * x + matrix[6] * y + matrix[10] * z + matrix[14];
        result[15] = matrix[3] * x + matrix[7] * y + matrix[11] * z + matrix[15];
    } 
    else {
        a00 = matrix[0]; a01 = matrix[1]; a02 = matrix[2]; a03 = matrix[3];
        a10 = matrix[4]; a11 = matrix[5]; a12 = matrix[6]; a13 = matrix[7];
        a20 = matrix[8]; a21 = matrix[9]; a22 = matrix[10]; a23 = matrix[11];

        result[0] = a00; result[1] = a01; result[2] = a02; result[3] = a03;
        result[4] = a10; result[5] = a11; result[6] = a12; result[7] = a13;
        result[8] = a20; result[9] = a21; result[10] = a22; result[11] = a23;

        result[12] = a00 * x + a10 * y + a20 * z + matrix[12];
        result[13] = a01 * x + a11 * y + a21 * z + matrix[13];
        result[14] = a02 * x + a12 * y + a22 * z + matrix[14];
        result[15] = a03 * x + a13 * y + a23 * z + matrix[15];
    }    
}

static inline void mat4_rotate(float result[16], float matrix[16], float angle, float x, float y, float z) {
    float len = sqrt(x * x + y * y + z * z),
          s, c, t,
          a00, a01, a02, a03,
          a10, a11, a12, a13,
          a20, a21, a22, a23,
          b00, b01, b02,
          b10, b11, b12,
          b20, b21, b22;

    if (fabs(len) < 0.000001) return;

    len = 1 / len;
    x *= len;
    y *= len;
    z *= len;

    s = sinf(angle);
    c = cosf(angle);
    t = 1 - c;

    a00 = matrix[0]; a01 = matrix[1]; a02 = matrix[2]; a03 = matrix[3];
    a10 = matrix[4]; a11 = matrix[5]; a12 = matrix[6]; a13 = matrix[7];
    a20 = matrix[8]; a21 = matrix[9]; a22 = matrix[10]; a23 = matrix[11];

    b00 = x * x * t + c; b01 = y * x * t + z * s; b02 = z * x * t - y * s;
    b10 = x * y * t - z * s; b11 = y * y * t + c; b12 = z * y * t + x * s;
    b20 = x * z * t + y * s; b21 = y * z * t - x * s; b22 = z * z * t + c;

    result[0] = a00 * b00 + a10 * b01 + a20 * b02;
    result[1] = a01 * b00 + a11 * b01 + a21 * b02;
    result[2] = a02 * b00 + a12 * b01 + a22 * b02;
    result[3] = a03 * b00 + a13 * b01 + a23 * b02;
    result[4] = a00 * b10 + a10 * b11 + a20 * b12;
    result[5] = a01 * b10 + a11 * b11 + a21 * b12;
    result[6] = a02 * b10 + a12 * b11 + a22 * b12;
    result[7] = a03 * b10 + a13 * b11 + a23 * b12;
    result[8] = a00 * b20 + a10 * b21 + a20 * b22;
    result[9] = a01 * b20 + a11 * b21 + a21 * b22;
    result[10] = a02 * b20 + a12 * b21 + a22 * b22;
    result[11] = a03 * b20 + a13 * b21 + a23 * b22;

    if (matrix != result) {
        result[12] = matrix[12];
        result[13] = matrix[13];
        result[14] = matrix[14];
        result[15] = matrix[15];
    }
}

static inline void mat4_frustum(float matrix[16], float left, float right, float bottom, float top, float near, float far) {
    float x = 2.0f * near / (right - left);
    float y = 2.0f * near / (top - bottom);

    float a = (right + left) / (right - left);
    float b = (top + bottom) / (top - bottom);
    float c = - (far + near) / (far - near);
    float d = - 2.0f * far * near / (far - near);

    matrix[0] = x;
    matrix[4] = 0;
    matrix[8] = a;
    matrix[12] = 0;
    matrix[1] = 0;
    matrix[5] = y;
    matrix[9] = b;
    matrix[13] = 0;
    matrix[2] = 0;
    matrix[6] = 0;
    matrix[10] = c;
    matrix[14] = d;
    matrix[3] = 0;
    matrix[7] = 0;
    matrix[11] = -1;
    matrix[15] = 0;
}

static inline void mat4_ortho(float matrix[16], float left, float right, float top, float bottom, float near, float far) {
    float w = 1.0f / (right - left);
    float h = 1.0f / (top - bottom);
    float p = 1.0f / (far - near);

    float x = (right + left) * w;
    float y = (top + bottom) * h;
    float z = (far + near) * p;

    matrix[0] = 2 * w;
    matrix[4] = 0;
    matrix[8] = 0;
    matrix[12] = -x;
    matrix[1] = 0;
    matrix[5] = 2 * h;
    matrix[9] = 0;
    matrix[13] = -y;
    matrix[2] = 0;
    matrix[6] = 0;
    matrix[10] = -2 * p;
    matrix[14] = -z;
    matrix[3] = 0;
    matrix[7] = 0;
    matrix[11] = 0;
    matrix[15] = 1;
}

static inline void mat4_perspective(float matrix[16], float fov, float aspect, float vnear, float vfar) {
    float f = 1.0f / tanf(fov * 0.5f);
    float nf = 1 / (vnear - vfar);
    
    matrix[0] = f / aspect;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;
    matrix[4] = 0;
    matrix[5] = f;
    matrix[6] = 0;
    matrix[7] = 0;
    matrix[8] = 0;
    matrix[9] = 0;
    matrix[10] = (vfar + vnear) * nf;
    matrix[11] = -1;
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = (2 * vfar * vnear) * nf;
    matrix[15] = 0;
}

static inline void mat4_lookat(float matrix[16], float eye[3], float center[3], float up[3]) {
    float x0, x1, x2, y0, y1, y2, z0, z1, z2, len,
          eyex = eye[0],
          eyey = eye[1],
          eyez = eye[2],
          upx = up[0],
          upy = up[1],
          upz = up[2],
          centerx = center[0],
          centery = center[1],
          centerz = center[2];

    if (fabs(eyex - centerx) < 0.000001f &&
        fabs(eyey - centery) < 0.000001f &&
        fabs(eyez - centerz) < 0.000001f) {
        return mat4_identity(matrix);
    }

    z0 = eyex - centerx;
    z1 = eyey - centery;
    z2 = eyez - centerz;

    len = 1 / sqrtf(z0 * z0 + z1 * z1 + z2 * z2);
    z0 *= len;
    z1 *= len;
    z2 *= len;

    x0 = upy * z2 - upz * z1;
    x1 = upz * z0 - upx * z2;
    x2 = upx * z1 - upy * z0;
    len = sqrt(x0 * x0 + x1 * x1 + x2 * x2);
    if (!len) {
        x0 = 0;
        x1 = 0;
        x2 = 0;
    } 
    else {
        len = 1 / len;
        x0 *= len;
        x1 *= len;
        x2 *= len;
    }

    y0 = z1 * x2 - z2 * x1;
    y1 = z2 * x0 - z0 * x2;
    y2 = z0 * x1 - z1 * x0;

    len = sqrt(y0 * y0 + y1 * y1 + y2 * y2);
    if (!len) {
        y0 = 0;
        y1 = 0;
        y2 = 0;
    } 
    else {
        len = 1 / len;
        y0 *= len;
        y1 *= len;
        y2 *= len;
    }

    matrix[0] = x0;
    matrix[1] = y0;
    matrix[2] = z0;
    matrix[3] = 0;
    matrix[4] = x1;
    matrix[5] = y1;
    matrix[6] = z1;
    matrix[7] = 0;
    matrix[8] = x2;
    matrix[9] = y2;
    matrix[10] = z2;
    matrix[11] = 0;
    matrix[12] = -(x0 * eyex + x1 * eyey + x2 * eyez);
    matrix[13] = -(y0 * eyex + y1 * eyey + y2 * eyez);
    matrix[14] = -(z0 * eyex + z1 * eyey + z2 * eyez);
    matrix[15] = 1;   
}

static inline void mat4_multiply(float result[16], float ma[16], float mb[16]) {
    float a11 = ma[0], a12 = ma[4], a13 = ma[8], a14 = ma[12];
    float a21 = ma[1], a22 = ma[5], a23 = ma[9], a24 = ma[13];
    float a31 = ma[2], a32 = ma[6], a33 = ma[10], a34 = ma[14];
    float a41 = ma[3], a42 = ma[7], a43 = ma[11], a44 = ma[15];

    float b11 = mb[0], b12 = mb[4], b13 = mb[8], b14 = mb[12];
    float b21 = mb[1], b22 = mb[5], b23 = mb[9], b24 = mb[13];
    float b31 = mb[2], b32 = mb[6], b33 = mb[10], b34 = mb[14];
    float b41 = mb[3], b42 = mb[7], b43 = mb[11], b44 = mb[15];

    result[0] = a11 * b11 + a12 * b21 + a13 * b31 + a14 * b41;
    result[4] = a11 * b12 + a12 * b22 + a13 * b32 + a14 * b42;
    result[8] = a11 * b13 + a12 * b23 + a13 * b33 + a14 * b43;
    result[12] = a11 * b14 + a12 * b24 + a13 * b34 + a14 * b44;

    result[1] = a21 * b11 + a22 * b21 + a23 * b31 + a24 * b41;
    result[5] = a21 * b12 + a22 * b22 + a23 * b32 + a24 * b42;
    result[9] = a21 * b13 + a22 * b23 + a23 * b33 + a24 * b43;
    result[13] = a21 * b14 + a22 * b24 + a23 * b34 + a24 * b44;

    result[2] = a31 * b11 + a32 * b21 + a33 * b31 + a34 * b41;
    result[6] = a31 * b12 + a32 * b22 + a33 * b32 + a34 * b42;
    result[10] = a31 * b13 + a32 * b23 + a33 * b33 + a34 * b43;
    result[14] = a31 * b14 + a32 * b24 + a33 * b34 + a34 * b44;

    result[3] = a41 * b11 + a42 * b21 + a43 * b31 + a44 * b41;
    result[7] = a41 * b12 + a42 * b22 + a43 * b32 + a44 * b42;
    result[11] = a41 * b13 + a42 * b23 + a43 * b33 + a44 * b43;
    result[15] = a41 * b14 + a42 * b24 + a43 * b34 + a44 * b44;
}

static inline void mat4_inverse(float dstMatrix[16], float srcMatrix[16]) {
    float n11 = srcMatrix[0], n21 = srcMatrix[1], n31 = srcMatrix[2], n41 = srcMatrix[3],
          n12 = srcMatrix[4], n22 = srcMatrix[5], n32 = srcMatrix[6], n42 = srcMatrix[7],
          n13 = srcMatrix[8], n23 = srcMatrix[9], n33 = srcMatrix[10], n43 = srcMatrix[11],
          n14 = srcMatrix[12], n24 = srcMatrix[13], n34 = srcMatrix[14], n44 = srcMatrix[15];

    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44,
          t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44,
          t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44,
          t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    if (det == 0) {
        mat4_identity(dstMatrix);
        return;
    }

    float invDet = 1.0f / det;

    dstMatrix[0] = t11 * invDet;
    dstMatrix[1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * invDet;
    dstMatrix[2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * invDet;
    dstMatrix[3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * invDet;

    dstMatrix[4] = t12 * invDet;
    dstMatrix[5] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * invDet;
    dstMatrix[6] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * invDet;
    dstMatrix[7] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * invDet;

    dstMatrix[8] = t13 * invDet;
    dstMatrix[9] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * invDet;
    dstMatrix[10] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * invDet;
    dstMatrix[11] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * invDet;

    dstMatrix[12] = t14 * invDet;
    dstMatrix[13] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * invDet;
    dstMatrix[14] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * invDet;
    dstMatrix[15] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * invDet;
}

static inline void mat4_transpose(float matrix[16]) {
    float tmp;
    tmp = matrix[1]; matrix[1] = matrix[4]; matrix[4] = tmp;
    tmp = matrix[2]; matrix[2] = matrix[8]; matrix[8] = tmp;
    tmp = matrix[6]; matrix[6] = matrix[9]; matrix[9] = tmp;

    tmp = matrix[3]; matrix[3] = matrix[12]; matrix[12] = tmp;
    tmp = matrix[7]; matrix[7] = matrix[13]; matrix[13] = tmp;
    tmp = matrix[11]; matrix[11] = matrix[14]; matrix[14] = tmp;
}

#endif