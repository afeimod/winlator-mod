#ifndef VEC3_H
#define VEC3_H

#include "math.h"

static inline void vec3_apply_mat4(float vector[3], float matrix[16]) {
    float x = vector[0], y = vector[1], z = vector[2];
    vector[0] = matrix[0] * x + matrix[4] * y + matrix[8] * z + matrix[12];
    vector[1] = matrix[1] * x + matrix[5] * y + matrix[9] * z + matrix[13];
    vector[2] = matrix[2] * x + matrix[6] * y + matrix[10] * z + matrix[14];
}

static inline float vec3_length(float vector[3]) {
    return (float)sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
}

static inline void vec3_normalize(float vector[3]) {
    float invLength = 1.0f / vec3_length(vector);

    if (!isfinite(invLength)) {
        vector[0] = vector[1] = vector[2] = 0.0f;
        return;
    }

    vector[0] *= invLength;
    vector[1] *= invLength;
    vector[2] *= invLength;
}

static inline void vec3_transform_direction(float vector[3], float matrix[16]) {
    float x = vector[0], y = vector[1], z = vector[2];
    vector[0] = matrix[0] * x + matrix[4] * y + matrix[8] * z;
    vector[1] = matrix[1] * x + matrix[5] * y + matrix[9] * z;
    vector[2] = matrix[2] * x + matrix[6] * y + matrix[10] * z;
    vec3_normalize(vector);
}

#endif