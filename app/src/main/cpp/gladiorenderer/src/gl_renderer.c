#include "gladio.h"
#include "gl_renderer.h"
#include "shader_material.h"
#include "gl_context.h"
#include "arb_program.h"

thread_local GLRenderer* currentRenderer = NULL;

static void initRaster(GLRenderer* renderer) {
    if (renderer->raster) return;
    GLRaster* raster = calloc(1, sizeof(GLRaster));

    const float position[] = {0.0f, 0.0f, 0.0f, 1.0f};
    memcpy(raster->position, position, sizeof(position));

    raster->pixelZoom[0] = 1.0f;
    raster->pixelZoom[1] = 1.0f;

    glGenTextures(1, &raster->textureId);
    glBindTexture(GL_TEXTURE_2D, raster->textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLTexture* texture = GLTexture_getBound(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture ? texture->id : 0);

    renderer->raster = raster;
}

static void initMaterials(GLRenderer* renderer) {
    if (renderer->materials) return;
    GLMaterial* materials = calloc(2, sizeof(GLMaterial));

    const float ambient[] = {0.2f, 0.2f, 0.2f};
    const float diffuse[] = {0.8f, 0.8f, 0.8f};

    memcpy(materials[0].ambient, ambient, sizeof(ambient));
    memcpy(materials[0].diffuse, diffuse, sizeof(diffuse));
    memcpy(materials[1].ambient, ambient, sizeof(ambient));
    memcpy(materials[1].diffuse, diffuse, sizeof(diffuse));

    renderer->materials = materials;
}

static void initLight(GLLight* light) {
    const float ambient[] = {0.2f, 0.2f, 0.2f};
    const float diffuse[] = {1.0f, 1.0f, 1.0f};
    const float position[] = {0.0f, 0.0f, 1.0f, 0.0f};
    const float attenuation[] = {1.0f, 0.0f, 0.0f};
    const float spotDirection[] = {0.0f, 0.0f, -1.0f};

    memcpy(light->ambient, ambient, sizeof(ambient));
    memcpy(light->diffuse, diffuse, sizeof(diffuse));
    memcpy(light->specular, diffuse, sizeof(diffuse));
    memcpy(light->position, position, sizeof(position));
    memcpy(light->attenuation, attenuation, sizeof(position));
    memcpy(light->spotDirection, spotDirection, sizeof(spotDirection));
    light->spotCutoffExponent[0] = TO_RADIANS(180);
}

void GLRenderer_initOnEGLContext(GLRenderer* renderer) {
    const float color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    const float normal[] = {0.0f, 0.0f, 1.0f};
    const float texCoord[] = {0.0f, 0.0f, 0.0f, 1.0f};

    memcpy(renderer->state.color, color, sizeof(color));
    memcpy(renderer->state.normal, normal, sizeof(normal));

    for (int i = 0; i < MAX_TEXCOORDS; i++) {
        memcpy(renderer->state.texCoords[i], texCoord, sizeof(texCoord));

        GLfloat param = GL_MODULATE;
        GLfloat scale = 1.0f;
        renderer->clientState.activeTexture = i;
        GLRenderer_setTexEnvParams(renderer, GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &param);
        GLRenderer_setTexEnvParams(renderer, GL_TEXTURE_ENV, GL_COMBINE_RGB, &param);
        GLRenderer_setTexEnvParams(renderer, GL_TEXTURE_ENV, GL_RGB_SCALE, &scale);
        GLRenderer_setTexEnvParams(renderer, GL_TEXTURE_ENV, GL_ALPHA_SCALE, &scale);
    }
    renderer->clientState.activeTexture = 0;
    renderer->state.polygonMode = GL_FILL;

    renderer->state.point.size = 1.0f;
    renderer->state.point.sizeMin = 0.0f;
    renderer->state.point.sizeMax = 1.0f;
    renderer->state.point.fadeThresholdSize = 1.0f;
    renderer->state.point.spriteCoordOrigin = GL_UPPER_LEFT;
    const float distanceAttenuation[] = {1.0f, 0.0f, 0.0f};
    memcpy(renderer->state.point.distanceAttenuation, distanceAttenuation, sizeof(distanceAttenuation));

    renderer->state.fog.mode = GL_EXP;
    renderer->state.fog.end = 1.0f;
    renderer->state.fog.density = 1.0f;

    renderer->state.alphaTest.func = GL_ALWAYS;
    renderer->state.alphaTest.ref = 0.0f;

    renderer->state.shadeModel = GL_SMOOTH;
    renderer->state.clampReadColor = false;

    for (int i = 0; i < MAX_LIGHTS; i++) initLight(&renderer->lights[i]);

    glGenBuffers(ARRAY_SIZE(renderer->bufferIds), renderer->bufferIds);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    renderer->queriesStartTime = nanoTime();
}

static void bindVertexBuffer(GLRenderer* renderer, GLuint bufferId, GLint location, int itemSize, int size, const void* data) {
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    GLRenderer_enableVertexAttribute(renderer, location);
    glVertexAttribPointer(location, itemSize, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

static void updateVertexBuffers(GLRenderer* renderer, int* attributeLocations) {
    Geometry* geometry = &renderer->geometry;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->bufferIds[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry->indices.size, geometry->indices.buffer, GL_DYNAMIC_DRAW);

    if (attributeLocations[POSITION_ARRAY_INDEX] != -1) {
        bindVertexBuffer(renderer, renderer->bufferIds[1], attributeLocations[POSITION_ARRAY_INDEX], 4, geometry->vertices.size, geometry->vertices.buffer);
    }

    if (attributeLocations[COLOR_ARRAY_INDEX] != -1) {
        if (geometry->colors.size > 0) {
            bindVertexBuffer(renderer, renderer->bufferIds[2], attributeLocations[COLOR_ARRAY_INDEX], 4, geometry->colors.size, geometry->colors.buffer);
        }
        else {
            GLRenderer_disableVertexAttribute(renderer, attributeLocations[COLOR_ARRAY_INDEX]);
            glVertexAttrib4fv(attributeLocations[COLOR_ARRAY_INDEX], renderer->state.color);
        }
    }

    if (attributeLocations[NORMAL_ARRAY_INDEX] != -1 && geometry->normals.size > 0) {
        bindVertexBuffer(renderer, renderer->bufferIds[3], attributeLocations[NORMAL_ARRAY_INDEX], 3, geometry->normals.size, geometry->normals.buffer);
    }

    for (int i = 0, j = TEXCOORD_ARRAY_INDEX; i < MAX_TEXCOORDS; i++, j++) {
        if (attributeLocations[j] != -1 && geometry->texCoords[i].size > 0) {
            bindVertexBuffer(renderer, renderer->bufferIds[4+i], attributeLocations[j], 4, geometry->texCoords[i].size, geometry->texCoords[i].buffer);
        }
        geometry->texCoords[i].size = 0;
    }

    geometry->indices.size = 0;
    geometry->vertices.size = 0;
    geometry->colors.size = 0;
    geometry->normals.size = 0;
}

static GLenum parseDrawMode(GLenum drawMode, GLenum polygonMode) {
    if (polygonMode == GL_POINT) {
        drawMode = GL_POINTS;
    }
    else if (polygonMode == GL_LINE) {
        drawMode = GL_LINES;
    }
    if (drawMode == GL_QUADS ||
        drawMode == GL_QUAD_STRIP ||
        drawMode == GL_TRIANGLE_STRIP ||
        drawMode == GL_TRIANGLE_FAN ||
        drawMode == GL_POLYGON) {
        drawMode = GL_TRIANGLES;
    }
    return drawMode;
}

static void updatePointSmoothTexture(GLRenderer* renderer) {
    if (renderer->state.point.smooth && !renderer->state.point.sprite) {
        if (renderer->circleTexture == 0) {
            glGenTextures(1, &renderer->circleTexture);
            glBindTexture(GL_TEXTURE_2D, renderer->circleTexture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            const uint8_t white[] = {255, 255, 255, 255};
            uint8_t* pixels = drawSmoothCircle(64, 64, white);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            free(pixels);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glBindTexture(GL_TEXTURE_2D, renderer->circleTexture);
    }
}

static ShaderMaterial* setCurrentMaterial(GLRenderer* renderer, MaterialOptions* options) {
    uint32_t hash = generateMaterialHash(options);
    ShaderMaterial* material = SparseArray_get(&renderer->materialMap, hash);
    if (!material) {
        material = ShaderMaterial_create(options);
        SparseArray_put(&renderer->materialMap, hash, material);
    }

    glUseProgram(material->program);
    ShaderMaterial_updateUniforms(material, renderer, options);
    return material;
}

void GLRenderer_drawImmediate(GLRenderer* renderer) {
    if (!renderer || ArrayDeque_isEmpty(&renderer->meshes)) return;
    GLClientState* clientState = &renderer->clientState;

    ShaderMaterial* material = NULL;
    if (!renderer->clientState.program) {
        ARBProgram* vertexProgram = NULL;
        ARBProgram* fragmentProgram = NULL;
        if (renderer->state.enabledARBPrograms[0] || renderer->state.enabledARBPrograms[1]) {
            vertexProgram = renderer->clientState.arbProgram[indexOfGLTarget(GL_VERTEX_PROGRAM_ARB)];
            fragmentProgram = renderer->clientState.arbProgram[indexOfGLTarget(GL_FRAGMENT_PROGRAM_ARB)];
        }

        uint8_t numTextures = 0;
        for (int i = 0; i < MAX_TEXCOORDS; i++) {
            if (renderer->state.enabledTextures[i][indexOfGLTarget(GL_TEXTURE_2D)]) numTextures++;
        }

        bool pointSprite = renderer->state.point.sprite || renderer->state.point.smooth;
        MaterialOptions options = {renderer->state.lighting, renderer->state.alphaTest.enabled, renderer->state.fog.enabled, pointSprite, true, numTextures, vertexProgram, fragmentProgram};
        material = setCurrentMaterial(renderer, &options);
        updateVertexBuffers(renderer, material->location.attributes);
    }
    else {
        ShaderConverter_updateBoundProgram();
        updateVertexBuffers(renderer, renderer->clientState.program->location.attributes);
    }

    while (!ArrayDeque_isEmpty(&renderer->meshes)) {
        Mesh* mesh = ArrayDeque_removeFirst(&renderer->meshes);
        GLenum drawMode = parseDrawMode(mesh->mode, renderer->state.polygonMode);

        if (material) {
            if (drawMode == GL_POINTS) {
                ShaderMaterial_updatePointUniforms(material, renderer);
                updatePointSmoothTexture(renderer);
            }
            else glUniform1f(material->location.point[0], 0.0f);
        }

        int count = mesh->end - mesh->start;
        glDrawElements(drawMode, count, GL_UNSIGNED_SHORT, (void*)(mesh->start * sizeof(short)));
        MEMFREE(mesh);

        if (drawMode == GL_POINTS && renderer->state.point.smooth && !renderer->state.point.sprite) {
            GLTexture* texture = GLTexture_getBound(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture ? texture->id : 0);
        }
    }

    glFlush();
    renderer->geometry.indices.position = 0;
    renderer->geometry.vertices.position = 0;
    GLRenderer_disableUnusedVertexAttributes(renderer);

    if (renderer->clientState.program) {
        for (int i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
            if (clientState->vao->attribs[i].boundArrayBuffer > 0) {
                GLVertexAttrib* vertexAttrib = &clientState->vao->attribs[i];
                glBindBuffer(GL_ARRAY_BUFFER, clientState->vao->attribs[i].boundArrayBuffer);
                int location = renderer->clientState.program->location.attributes[i];
                GLRenderer_enableVertexAttribute(renderer, location);
                glVertexAttribPointer(location, vertexAttrib->size, vertexAttrib->type, vertexAttrib->normalized, vertexAttrib->stride, vertexAttrib->pointer);
            }
        }
    }

    GLBuffer* arrayBuffer = clientState->vao->buffer[indexOfGLTarget(GL_ARRAY_BUFFER)];
    GLBuffer* elementArrayBuffer = clientState->vao->buffer[indexOfGLTarget(GL_ELEMENT_ARRAY_BUFFER)];
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer ? arrayBuffer->id : 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBuffer ? elementArrayBuffer->id : 0);
}

void GLRenderer_beginImmediate(GLRenderer* renderer, GLenum mode) {
    if (renderer->activeMesh) return;
    Mesh* prevMesh = ArrayDeque_getLast(&renderer->meshes);
    if (prevMesh && parseDrawMode(prevMesh->mode, renderer->state.polygonMode) == GL_TRIANGLES &&
        parseDrawMode(mode, renderer->state.polygonMode) == GL_TRIANGLES) {
        prevMesh->mode = mode;
        prevMesh->indexCount = 0;
        prevMesh->end = renderer->geometry.indices.position;
        renderer->activeMesh = prevMesh;
        return;
    }

    Mesh* mesh = calloc(1, sizeof(Mesh));
    mesh->mode = mode;
    mesh->start = mesh->end = renderer->geometry.indices.position;
    renderer->activeMesh = mesh;
    ArrayDeque_addLast(&renderer->meshes, mesh);
}

void GLRenderer_endImmediate(GLRenderer* renderer) {
    if (renderer->activeMesh) {
        renderer->activeMesh->end = renderer->geometry.indices.position;
        renderer->activeMesh = NULL;

        renderer->geometry.colors.position = 0;
        renderer->geometry.normals.position = 0;

        for (int i = 0; i < MAX_TEXCOORDS; i++) renderer->geometry.texCoords[i].position = 0;
    }
}

float* GLRenderer_getMatrixFromStack(GLRenderer* renderer, int index) {
    static float identityMatrix[] = IDENTITY_MATRIX;
    ArrayList* matrixStack = &renderer->matrixStack[index];
    return matrixStack->size > 0 ? matrixStack->elements[matrixStack->size-1] : identityMatrix;
}

float* GLRenderer_getCurrentMatrix(GLRenderer* renderer) {
    ArrayList* matrixStack = &renderer->matrixStack[renderer->matrixIndex];
    if (matrixStack->size == 0) ArrayList_add(matrixStack, mat4_alloc());
    return matrixStack->elements[matrixStack->size-1];
}

void GLRenderer_pushMatrix(GLRenderer* renderer) {
    ArrayList* matrixStack = &renderer->matrixStack[renderer->matrixIndex];
    if (matrixStack->size < 32) {
        float* matrix = GLRenderer_getCurrentMatrix(renderer);
        ArrayList_add(matrixStack, mat4_clone(matrix));
    }
}

void GLRenderer_popMatrix(GLRenderer* renderer) {
    ArrayList* matrixStack = &renderer->matrixStack[renderer->matrixIndex];
    if (matrixStack->size > 1) {
        float* matrix = ArrayList_removeAt(matrixStack, matrixStack->size-1);
        MEMFREE(matrix);
    }
}

void GLRenderer_addVertex(GLRenderer* renderer, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    if (!renderer->activeMesh) return;
    ArrayBuffer_putFloat4(&renderer->geometry.vertices, x, y, z, w);

    if (renderer->geometry.colors.position > 0 || renderer->geometry.colors.size > 0) {
        ArrayBuffer_putBytes(&renderer->geometry.colors, renderer->state.color, 4 * sizeof(float));
    }

    if (renderer->geometry.normals.position > 0) {
        ArrayBuffer_putBytes(&renderer->geometry.normals, renderer->state.normal, 3 * sizeof(float));
    }

    for (int i = 0; i < MAX_TEXCOORDS; i++) {
        if (renderer->geometry.texCoords[i].position > 0) {
            ArrayBuffer_putBytes(&renderer->geometry.texCoords[i], renderer->state.texCoords[i], 4 * sizeof(float));
        }
    }

    int index = renderer->geometry.vertices.position++;
    GLRenderer_addArrayElement(renderer, index);
}

void GLRenderer_addArrayElement(GLRenderer* renderer, int index) {
    if (!renderer->activeMesh) return;

    ArrayBuffer_putShort(&renderer->geometry.indices, index);
    renderer->geometry.indices.position++;
    renderer->activeMesh->indexCount++;

    switch (renderer->activeMesh->mode) {
        case GL_QUADS:
            if ((renderer->activeMesh->indexCount % 4) == 0) {
                int last = renderer->geometry.indices.position - 1;
                short* indices = (short*)renderer->geometry.indices.buffer;
                ArrayBuffer_putShort(&renderer->geometry.indices, indices[last-3]);
                ArrayBuffer_putShort(&renderer->geometry.indices, indices[last-1]);
                renderer->geometry.indices.position += 2;
            }
            break;
        case GL_TRIANGLE_STRIP:
        case GL_QUAD_STRIP:
            if (renderer->activeMesh->indexCount > 3) {
                int last = renderer->geometry.indices.position - 1;
                short* indices = (short*)renderer->geometry.indices.buffer;
                int first = renderer->activeMesh->end;
                short index1 = renderer->activeMesh->indexCount > 4 ? indices[MAX(first+2, last-6)] : indices[last-2];
                short index2 = renderer->activeMesh->indexCount > 4 ? indices[MAX(first+2, last-3)] : indices[last-1];

                if (renderer->activeMesh->indexCount % 2 == 0) {
                    ArrayBuffer_putShort(&renderer->geometry.indices, index2);
                    ArrayBuffer_putShort(&renderer->geometry.indices, index1);
                }
                else {
                    ArrayBuffer_putShort(&renderer->geometry.indices, index1);
                    ArrayBuffer_putShort(&renderer->geometry.indices, index2);
                }
                renderer->geometry.indices.position += 2;
            }
            break;
        case GL_TRIANGLE_FAN:
        case GL_POLYGON:
            if (renderer->activeMesh->indexCount > 3) {
                int last = renderer->geometry.indices.position - 1;
                short* indices = (short*)renderer->geometry.indices.buffer;
                int first = renderer->activeMesh->end;
                short index2 = renderer->activeMesh->indexCount > 4 ? indices[MAX(first+2, last-3)] : indices[last-1];

                ArrayBuffer_putShort(&renderer->geometry.indices, indices[first]);
                ArrayBuffer_putShort(&renderer->geometry.indices, index2);
                renderer->geometry.indices.position += 2;
            }
            break;
    }
}

void GLRenderer_setCapabilityState(GLRenderer* renderer, GLenum cap, bool state, int index) {
    switch (cap) {
        case GL_LIGHTING:
            renderer->state.lighting = state;
            break;
        case GL_COLOR_MATERIAL:
            renderer->state.colorMaterial.enabled = state;
            break;
        case GL_ALPHA_TEST:
            renderer->state.alphaTest.enabled = state;
            break;
        case GL_TEXTURE_1D:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_CUBE_MAP: {
            GLuint target = parseTexTarget(cap);
            uint8_t activeTexture = renderer->clientState.activeTexture;
            renderer->state.enabledTextures[activeTexture][indexOfGLTarget(target)] = state;
            break;
        }
        case GL_FOG:
            renderer->state.fog.enabled = state;
            break;
        case GL_POINT_SMOOTH:
            renderer->state.point.smooth = state;
            break;
        case GL_POINT_SPRITE:
            renderer->state.point.sprite = state;
            break;
        case GL_LINE_SMOOTH:
        case GL_LINE_STIPPLE:
        case GL_POLYGON_STIPPLE:
        case GL_POLYGON_OFFSET_LINE:
        case GL_VERTEX_PROGRAM_POINT_SIZE:
        case GL_NORMALIZE:
        case GL_TEXTURE_CUBE_MAP_SEAMLESS:
        case GL_DEPTH_CLAMP:
            break;
        case GL_VERTEX_PROGRAM_ARB:
        case GL_FRAGMENT_PROGRAM_ARB:
        case GL_VERTEX_PROGRAM_TWO_SIDE:
            renderer->state.enabledARBPrograms[indexOfGLTarget(cap)] = state;
            break;
        case GL_PRIMITIVE_RESTART:
            cap = GL_PRIMITIVE_RESTART_FIXED_INDEX;
        default:
            if (cap >= GL_LIGHT0 && cap <= GL_LIGHT7) {
                GLRenderer_setLightParams(renderer, cap, GL_LIGHTING, &state);
                return;
            }
            if ((cap >= GL_TEXTURE_GEN_S && cap <= GL_TEXTURE_GEN_Q) || (cap >= GL_CLIP_PLANE0 && cap <= GL_CLIP_PLANE5)) {
                if (state) println("gladio:setCapabilityState: unimplemented cap %x", cap);
                return;
            }

            if (index != -1) {
                if (state) glEnablei(cap, index);
                else glDisablei(cap, index);
            }
            else if (state) glEnable(cap);
            else glDisable(cap);
            break;
    }
}

void GLRenderer_setMaterialParams(GLRenderer* renderer, GLenum face, GLenum pname, void* params) {
    initMaterials(renderer);

    for (int i = 0; i < 2; i++) {
        if ((face == GL_FRONT && i == 0) || (face == GL_BACK && i == 1) || face == GL_FRONT_AND_BACK) {
            GLMaterial* material = &renderer->materials[i];
            switch (pname) {
                case GL_AMBIENT:
                    memcpy(material->ambient, params, sizeof(material->ambient));
                    break;
                case GL_DIFFUSE:
                    memcpy(material->diffuse, params, sizeof(material->diffuse));
                    break;
                case GL_SPECULAR:
                    memcpy(material->specular, params, 3 * sizeof(float));
                    break;
                case GL_EMISSION:
                    memcpy(material->emission, params, sizeof(material->emission));
                    break;
                case GL_SHININESS:
                    material->specular[3] = *(float*)params;
                    break;
                case GL_AMBIENT_AND_DIFFUSE:
                    memcpy(material->ambient, params, sizeof(material->ambient));
                    memcpy(material->diffuse, params, sizeof(material->diffuse));
                    break;
            }
        }
    }
}

void GLRenderer_setLightParams(GLRenderer* renderer, GLenum id, GLenum pname, void* params) {
    int index = id - GL_LIGHT0;
    GLLight* light = index < MAX_LIGHTS ? &renderer->lights[index] : NULL;
    if (!light) return;

    switch (pname) {
        case GL_POSITION: {
            memcpy(light->position, params, sizeof(light->position));
            float* modelViewMatrix = GLRenderer_getMatrixFromStack(renderer, MODEL_VIEW_MATRIX_INDEX);
            if (light->position[3] > 0.0f) {
                vec3_apply_mat4(light->position, modelViewMatrix);
            }
            else {
                float inverseMatrix[16];
                mat4_inverse(inverseMatrix, modelViewMatrix);
                vec3_transform_direction(light->position, inverseMatrix);
            }
            break;
        }
        case GL_AMBIENT:
            memcpy(light->ambient, params, sizeof(light->ambient));
            break;
        case GL_DIFFUSE:
            memcpy(light->diffuse, params, sizeof(light->diffuse));
            break;
        case GL_SPECULAR:
            memcpy(light->specular, params, sizeof(light->specular));
            break;
        case GL_LIGHTING:
            light->enabled = *(bool*)params;
            break;
        case GL_CONSTANT_ATTENUATION:
            light->attenuation[0] = *(float*)params;
            break;
        case GL_LINEAR_ATTENUATION:
            light->attenuation[1] = *(float*)params;
            break;
        case GL_QUADRATIC_ATTENUATION:
            light->attenuation[2] = *(float*)params;
            break;
        case GL_SPOT_CUTOFF:
            light->spotCutoffExponent[0] = TO_RADIANS(*(float*)params);
            break;
        case GL_SPOT_EXPONENT:
            light->spotCutoffExponent[1] = *(float*)params;
            break;
        case GL_SPOT_DIRECTION: {
            memcpy(light->spotDirection, params, sizeof(light->spotDirection));
            float* modelViewMatrix = GLRenderer_getMatrixFromStack(renderer, MODEL_VIEW_MATRIX_INDEX);
            float inverseMatrix[16];
            mat4_inverse(inverseMatrix, modelViewMatrix);
            vec3_transform_direction(light->spotDirection, inverseMatrix);
            break;
        }
    }
}

void GLRenderer_setTexEnvParams(GLRenderer* renderer, GLenum target, GLenum pname, float* params) {
    int activeTexture = renderer->clientState.activeTexture;
    TexEnv* texEnv = &renderer->state.texEnv[activeTexture];
    switch (pname) {
        case GL_TEXTURE_ENV_MODE: {
            GLenum param = (GLenum)params[0];
            if (param == GL_MODULATE ||
                param == GL_REPLACE ||
                param == GL_DECAL ||
                param == GL_BLEND ||
                param == GL_INTERPOLATE ||
                param == GL_ADD ||
                param == GL_COMBINE) {
                if (param == GL_BLEND) param = GL_INTERPOLATE;
                texEnv->mode = param;
            }
            break;
        }
        case GL_TEXTURE_ENV_COLOR:
            memcpy(texEnv->color, params, sizeof(texEnv->color));
            break;
        case GL_RGB_SCALE:
            texEnv->rgbaScale[0] = params[0];
            break;
        case GL_ALPHA_SCALE:
            texEnv->rgbaScale[1] = params[0];
            break;
        case GL_COMBINE_RGB:
            texEnv->combineRGBA[0] = (GLenum)params[0];
            break;
        case GL_COMBINE_ALPHA:
            texEnv->combineRGBA[1] = (GLenum)params[0];
            break;
        case GL_SOURCE0_RGB:
            texEnv->sourceRGBA[0] = (GLenum)params[0];
            break;
        case GL_SOURCE1_RGB:
            texEnv->sourceRGBA[1] = (GLenum)params[0];
            break;
        case GL_SOURCE0_ALPHA:
            texEnv->sourceRGBA[2] = (GLenum)params[0];
            break;
        case GL_SOURCE1_ALPHA:
            texEnv->sourceRGBA[3] = (GLenum)params[0];
            break;
        case GL_OPERAND0_RGB:
            texEnv->operandRGBA[0] = (GLenum)params[0];
            break;
        case GL_OPERAND1_RGB:
            texEnv->operandRGBA[1] = (GLenum)params[0];
            break;
        case GL_OPERAND0_ALPHA:
            texEnv->operandRGBA[2] = (GLenum)params[0];
            break;
        case GL_OPERAND1_ALPHA:
            texEnv->operandRGBA[3] = (GLenum)params[0];
            break;
        default:
            println("gladio:setTexEnvParams: unimplemented pname %x", pname);
            break;
    }
}

void GLRenderer_setFogParams(GLRenderer* renderer, GLenum pname, GLfloat* params) {
    switch (pname) {
        case GL_FOG_MODE:
            renderer->state.fog.mode = (GLenum)params[0];
            break;
        case GL_FOG_DENSITY:
            renderer->state.fog.density = params[0];
            break;
        case GL_FOG_START:
            renderer->state.fog.start = params[0];
            break;
        case GL_FOG_END:
            renderer->state.fog.end = params[0];
            break;
        case GL_FOG_COLOR:
            memcpy(renderer->state.fog.color, params, sizeof(renderer->state.fog.color));
            break;
        default:
            println("gladio:setFogParams: unimplemented pname %x", pname);
            break;
    }
}

void GLRenderer_setPointParams(GLRenderer* renderer, GLenum pname, void* params) {
    switch (pname) {
        case GL_POINT_SIZE_MIN:
            renderer->state.point.sizeMin = *(float*)(params);
            break;
        case GL_POINT_SIZE_MAX:
            renderer->state.point.sizeMax = *(float*)(params);
            break;
        case GL_POINT_FADE_THRESHOLD_SIZE:
            renderer->state.point.fadeThresholdSize = *(float*)(params);
            break;
        case GL_POINT_DISTANCE_ATTENUATION:
            memcpy(renderer->state.point.distanceAttenuation, params, sizeof(renderer->state.point.distanceAttenuation));
            break;
        case GL_POINT_SPRITE_COORD_ORIGIN:
            renderer->state.point.spriteCoordOrigin = *(int*)(params);
            break;
        default:
            println("gladio:setPointParams: unimplemented pname %x", pname);
            break;
    }
}

void GLRenderer_destroy(GLRenderer* renderer) {
    if (!renderer) return;
    glDeleteBuffers(ARRAY_SIZE(renderer->bufferIds), renderer->bufferIds);

    GLVertexArrayObject_delete(&renderer->clientState, 0);

    if (renderer->circleTexture > 0) {
        glDeleteTextures(1, &renderer->circleTexture);
        renderer->circleTexture = 0;
    }

    for (int i = 0; i < renderer->materialMap.size; i++) {
        ShaderMaterial* material = renderer->materialMap.entries[i].value;
        if (material) ShaderMaterial_destroy(material);
    }
    SparseArray_free(&renderer->materialMap, false);
    MEMFREE(renderer->materials);

    ArrayBuffer_free(&renderer->geometry.indices);
    ArrayBuffer_free(&renderer->geometry.vertices);
    ArrayBuffer_free(&renderer->geometry.colors);
    ArrayBuffer_free(&renderer->geometry.normals);

    for (int i = 0; i < MAX_TEXCOORDS; i++) {
        ArrayBuffer_free(&renderer->geometry.texCoords[i]);
    }

    ArrayDeque_free(&renderer->meshes, true);

    if (renderer->raster) {
        glDeleteTextures(1, &renderer->raster->textureId);
        free(renderer->raster);
    }
}

int GLRenderer_getParamsv(GLRenderer* renderer, GLenum pname, GLenum type, void* params) {
    int paramSize = type == GL_BOOL ? sizeof(GLboolean) : sizeof(GLint);

    switch (pname) {
        case GL_PROJECTION_MATRIX:
            if (params) memcpy(params, GLRenderer_getMatrixFromStack(renderer, PROJECTION_MATRIX_INDEX), MAT4_SIZE);
            paramSize = MAT4_SIZE;
            break;
        case GL_MODELVIEW_MATRIX:
            if (params) memcpy(params, GLRenderer_getMatrixFromStack(renderer, MODEL_VIEW_MATRIX_INDEX), MAT4_SIZE);
            paramSize = MAT4_SIZE;
            break;
        case GL_TEXTURE_MATRIX:
            if (params) memcpy(params, GLRenderer_getMatrixFromStack(renderer, TEXTURE_MATRIX_INDEX), MAT4_SIZE);
            paramSize = MAT4_SIZE;
            break;
        case GL_MAX_PROJECTION_STACK_DEPTH:
        case GL_MAX_TEXTURE_STACK_DEPTH:
            if (params) *(GLint*)params = 2;
            break;
        case GL_MAX_MODELVIEW_STACK_DEPTH:
            if (params) *(GLint*)params = 32;
            break;
        case GL_MAX_LIGHTS:
            if (params) *(GLint*)params = MAX_LIGHTS;
            break;
        case GL_MAX_CLIP_PLANES:
            if (params) *(GLint*)params = 6;
            break;
        case GL_NUM_EXTENSIONS:
            if (params) getGLExtensions(params);
            break;
        case GL_STEREO:
            if (params) *(GLboolean*)params = 0;
            paramSize = sizeof(GLboolean);
            break;
        case GL_AUX_BUFFERS:
            if (params) *(GLint*)params = 0;
            break;
        case GL_MAX_TEXTURE_UNITS:
        case GL_MAX_TEXTURE_COORDS:
            if (params) *(GLint*)params = MAX_TEXCOORDS;
            break;
        case GL_MAX_RECTANGLE_TEXTURE_SIZE:
        case GL_MAX_TEXTURE_SIZE:
            if (params) *(GLint*)params = 4096;
            break;
        case GL_FOG:
            if (params) *(GLboolean*)params = renderer->state.fog.enabled;
            paramSize = sizeof(GLboolean);
            break;
        case GL_FOG_COLOR:
            if (params) memcpy(params, renderer->state.fog.color, sizeof(renderer->state.fog.color));
            paramSize = sizeof(renderer->state.fog.color);
            break;
        case GL_FOG_DENSITY:
            if (params) *(GLfloat*)params = renderer->state.fog.density;
            break;
        case GL_FOG_START:
            if (params) *(GLfloat*)params = renderer->state.fog.start;
            break;
        case GL_FOG_END:
            if (params) *(GLfloat*)params = renderer->state.fog.end;
            break;
        case GL_FOG_MODE:
            if (params) *(GLfloat*)params = renderer->state.fog.mode;
            break;
        case GL_CONTEXT_PROFILE_MASK:
            if (params) *(GLint*)params = (GL_CONTEXT_CORE_PROFILE_BIT | GL_CONTEXT_COMPATIBILITY_PROFILE_BIT);
            break;
        case GL_SHADE_MODEL:
            if (params) *(GLint*)params = renderer->state.shadeModel;
            break;
        case GL_LIGHTING:
            if (params) *(GLboolean*)params = renderer->state.lighting;
            paramSize = sizeof(GLboolean);
            break;
        case GL_COLOR_MATERIAL:
            if (params) *(GLboolean*)params = renderer->state.colorMaterial.enabled;
            paramSize = sizeof(GLboolean);
            break;
        case GL_MAJOR_VERSION:
        case GL_MINOR_VERSION:
            if (params) {
                int version[0];
                sscanf(GL_STRING_VERSION, "%d.%d", &version[0], &version[1]);
                *(GLint*)params = version[pname - GL_MAJOR_VERSION];
            }
            break;
        case GL_LINE_SMOOTH:
        case GL_LINE_STIPPLE:
        case GL_POLYGON_STIPPLE:
        case GL_POLYGON_OFFSET_LINE:
        case GL_POINT_SPRITE:
        case GL_POINT_SMOOTH:
        case GL_VERTEX_PROGRAM_POINT_SIZE:
        case GL_NORMALIZE:
            if (params) *(GLboolean*)params = 0;
            paramSize = sizeof(GLboolean);
            break;
        case GL_ALPHA_TEST:
            if (params) *(GLboolean*)params = renderer->state.alphaTest.enabled;
            paramSize = sizeof(GLboolean);
            break;
        case GL_ALPHA_TEST_REF:
            if (params) *(GLfloat*)params = renderer->state.alphaTest.ref;
            break;
        case GL_ALPHA_TEST_FUNC:
            if (params) *(GLint*)params = renderer->state.alphaTest.func;
            break;
        case GL_CURRENT_COLOR:
            if (params) memcpy(params, renderer->state.color, sizeof(renderer->state.color));
            paramSize = sizeof(renderer->state.color);
            break;
        case GL_CURRENT_NORMAL:
            if (params) memcpy(params, renderer->state.normal, sizeof(renderer->state.normal));
            paramSize = sizeof(renderer->state.normal);
            break;
        case GL_CURRENT_TEXTURE_COORDS:
            if (params) {
                uint8_t activeTexCoord = renderer->clientState.activeTexCoord;
                memcpy(params, renderer->state.texCoords[activeTexCoord], sizeof(renderer->state.texCoords[0]));
            }
            paramSize = sizeof(renderer->state.texCoords[0]);
            break;
        case GL_CLAMP_READ_COLOR:
            if (params) *(GLint*)params = renderer->state.clampReadColor ? GL_TRUE : GL_FALSE;
            break;
        case GL_POINT_SIZE_RANGE:
            pname = GL_ALIASED_POINT_SIZE_RANGE;
        default:
            if (pname >= GL_LIGHT0 && pname <= GL_LIGHT7) {
                int index = pname - GL_LIGHT0;
                if (params) *(GLboolean*)params = index < MAX_LIGHTS ? renderer->lights[index].enabled : 0;
                paramSize = sizeof(GLboolean);
                break;
            }

            switch (pname) {
                case GL_COLOR_WRITEMASK:
                    paramSize = 4 * sizeof(GLboolean);
                    break;
                case GL_DEPTH_RANGE:
                case GL_ALIASED_POINT_SIZE_RANGE:
                case GL_ALIASED_LINE_WIDTH_RANGE:
                    paramSize = 2 * sizeof(GLfloat);
                    break;
                case GL_SCISSOR_BOX:
                case GL_VIEWPORT:
                case GL_BLEND_COLOR:
                case GL_COLOR_CLEAR_VALUE:
                    paramSize = 4 * sizeof(GLint);
                    break;
            }

            switch (type) {
                case GL_FLOAT:
                    if (params) glGetFloatv(pname, params);
                    break;
                case GL_BOOL:
                    if (params) glGetBooleanv(pname, params);
                    break;
                case GL_INT:
                    if (params) glGetIntegerv(pname, params);
                    break;
            }

            if (params && (pname == GL_FRAMEBUFFER_BINDING || pname == GL_DRAW_FRAMEBUFFER_BINDING || pname == GL_READ_FRAMEBUFFER_BINDING)) {
                int framebuffer = *(GLint*)params;
                if (framebuffer == renderer->displayBuffers[0] || framebuffer == renderer->displayBuffers[1]) {
                    *(GLint*)params = 0;
                }
            }
            break;
    }

    return paramSize;
}

void GLRenderer_setPixelZoom(GLRenderer* renderer, GLfloat xfactor, GLfloat yfactor) {
    initRaster(renderer);

    renderer->raster->pixelZoom[0] = xfactor;
    renderer->raster->pixelZoom[1] = yfactor;
}

void GLRenderer_setRasterPos(GLRenderer* renderer, bool transform, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    initRaster(renderer);

    float* projMatrix = GLRenderer_getMatrixFromStack(renderer, PROJECTION_MATRIX_INDEX);
    float* modelViewMatrix = GLRenderer_getMatrixFromStack(renderer, MODEL_VIEW_MATRIX_INDEX);
    float* rasterPos = renderer->raster->position;

    if (transform) {
        rasterPos[0] = x;
        rasterPos[1] = y;
        rasterPos[2] = z;
        rasterPos[3] = w;

        vec3_apply_mat4(rasterPos, modelViewMatrix);
        vec3_apply_mat4(rasterPos, projMatrix);

        float dist = (rasterPos[3] == 0.0F) ? 1.0f : 1.0f / rasterPos[3];
        rasterPos[0] *= dist;
        rasterPos[1] *= dist;
        rasterPos[2] *= dist;

        float halfWidth = renderer->displaySize[0] * 0.5f;
        float halfHeight = renderer->displaySize[1] * 0.5f;
        rasterPos[0] = rasterPos[0] * halfWidth + halfWidth;
        rasterPos[1] = rasterPos[1] * halfHeight + halfHeight;
    }
    else {
        rasterPos[0] = x;
        rasterPos[1] = y;
        rasterPos[2] = z;
        rasterPos[3] = w;
    }
}

void GLRenderer_drawPixels(GLRenderer* renderer, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) {
    GLint internalformat = GL_NONE;
    convertTexImageFormat(GL_TEXTURE_2D, &internalformat, &format, &type, NULL, 0);

    initRaster(renderer);

    GLRaster* raster = renderer->raster;
    glBindTexture(GL_TEXTURE_2D, raster->textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, type, pixels);

    ArrayBuffer_rewind(&raster->quad.vertices);
    ArrayBuffer_rewind(&raster->quad.texCoords);

    const float* rasterPos = renderer->raster->position;
    const float x0 = rasterPos[0] / (float)renderer->displaySize[0] * 2.0f - 1.0f;
    const float y0 = rasterPos[1] / (float)renderer->displaySize[1] * 2.0f - 1.0f;
    const float x1 = (rasterPos[0] + width * raster->pixelZoom[0]) / (float)renderer->displaySize[0] * 2.0f - 1.0f;
    const float y1 = (rasterPos[1] + height * raster->pixelZoom[1]) / (float)renderer->displaySize[1] * 2.0f - 1.0f;

    ArrayBuffer_putFloat4(&raster->quad.vertices, x0, y0, 0.0f, 1.0f);
    ArrayBuffer_putFloat4(&raster->quad.texCoords, 0.0f, 0.0f, 0.0f, 1.0f);
    ArrayBuffer_putFloat4(&raster->quad.vertices, x0, y1, 0.0f, 1.0f);
    ArrayBuffer_putFloat4(&raster->quad.texCoords, 0.0f, 1.0f, 0.0f, 1.0f);
    ArrayBuffer_putFloat4(&raster->quad.vertices, x1, y0, 0.0f, 1.0f);
    ArrayBuffer_putFloat4(&raster->quad.texCoords, 1.0f, 0.0f, 0.0f, 1.0f);
    ArrayBuffer_putFloat4(&raster->quad.vertices, x1, y1, 0.0f, 1.0f);
    ArrayBuffer_putFloat4(&raster->quad.texCoords, 1.0f, 1.0f, 0.0f, 1.0f);

    MaterialOptions options = {renderer->state.lighting, false, false, false, false, 1, NULL, NULL};
    ShaderMaterial* material = setCurrentMaterial(renderer, &options);
    bindVertexBuffer(renderer, renderer->bufferIds[2], material->location.attributes[POSITION_ARRAY_INDEX], 4, raster->quad.vertices.size, raster->quad.vertices.buffer);
    bindVertexBuffer(renderer, renderer->bufferIds[4], material->location.attributes[TEXCOORD_ARRAY_INDEX], 4, raster->quad.texCoords.size, raster->quad.texCoords.buffer);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->bufferIds[3]);
    GLRenderer_disableVertexAttribute(renderer, material->location.attributes[COLOR_ARRAY_INDEX]);
    glVertexAttrib4fv(material->location.attributes[COLOR_ARRAY_INDEX], renderer->state.color);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    GLRenderer_disableUnusedVertexAttributes(renderer);

    GLTexture* texture = GLTexture_getBound(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture ? texture->id : 0);
}

void GLRenderer_setSamplerParameter(GLRenderer* renderer, GLuint sampler, GLenum pname, GLfloat* params) {
    switch (pname) {
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        case GL_TEXTURE_WRAP_R:
        case GL_TEXTURE_COMPARE_FUNC:
        case GL_TEXTURE_COMPARE_MODE:
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_MAG_FILTER:
            glSamplerParameteri(sampler, pname, (GLint)*params);
            break;
        case GL_TEXTURE_MAX_ANISOTROPY:
            glSamplerParameterf(sampler, pname, MAX(*params, 1.0f));
            break;
        case GL_TEXTURE_MIN_LOD:
        case GL_TEXTURE_MAX_LOD:
            glSamplerParameterf(sampler, pname, *params);
            break;
        case GL_TEXTURE_BORDER_COLOR:
            glSamplerParameterfv(sampler, pname, params);
            break;
    }
}

void GLRenderer_getSamplerParameter(GLRenderer* renderer, GLuint sampler, GLenum pname, GLfloat* params) {
    switch (pname) {
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        case GL_TEXTURE_WRAP_R:
        case GL_TEXTURE_COMPARE_FUNC:
        case GL_TEXTURE_COMPARE_MODE:
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_MAG_FILTER: {
            GLint paramsi;
            glGetSamplerParameteriv(sampler, pname, &paramsi);
            *params = paramsi;
            break;
        }
        case GL_TEXTURE_MAX_ANISOTROPY:
        case GL_TEXTURE_MIN_LOD:
        case GL_TEXTURE_MAX_LOD:
        case GL_TEXTURE_BORDER_COLOR:
            glGetSamplerParameterfv(sampler, pname, params);
            break;
        default:
            println("gladio:getSamplerParameter: unimplemented pname %x", pname);
            break;
    }
}

void GLRenderer_setTexParameter(GLRenderer* renderer, GLenum target, GLenum pname, GLfloat* params) {
    target = parseTexTarget(target);
    switch (pname) {
        case GL_GENERATE_MIPMAP: {
            GLint param = (GLint)*params;
            GLTexture* texture = GLTexture_getBound(target);
            if (texture) texture->generateMipmap = param ? true : false;
            break;
        }
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        case GL_TEXTURE_WRAP_R: {
            GLint param = (GLint)*params;
            if (param == GL_CLAMP) param = GL_CLAMP_TO_EDGE;
            glTexParameteri(target, pname, param);
            break;
        }
        case GL_DEPTH_STENCIL_TEXTURE_MODE:
        case GL_TEXTURE_BASE_LEVEL:
        case GL_TEXTURE_COMPARE_FUNC:
        case GL_TEXTURE_COMPARE_MODE:
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_MAG_FILTER:
        case GL_TEXTURE_MAX_LEVEL:
        case GL_TEXTURE_SWIZZLE_R:
        case GL_TEXTURE_SWIZZLE_G:
        case GL_TEXTURE_SWIZZLE_B:
        case GL_TEXTURE_SWIZZLE_A:
            glTexParameteri(target, pname, (GLint)*params);
            break;
        case GL_TEXTURE_MAX_ANISOTROPY:
            glTexParameterf(target, pname, MAX(*params, 1.0f));
            break;
        case GL_TEXTURE_MIN_LOD:
        case GL_TEXTURE_MAX_LOD:
            glTexParameterf(target, pname, *params);
            break;
        case GL_TEXTURE_BORDER_COLOR:
            glTexParameterfv(target, pname, params);
            break;
        case GL_DEPTH_TEXTURE_MODE: {
            GLint mode = (GLint)*params;
            switch (mode) {
                case GL_LUMINANCE:
                    glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_RED);
                    glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
                    glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
                    glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_ONE);
                    break;
                case GL_INTENSITY:
                    glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_RED);
                    glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
                    glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
                    glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
                    break;
                case GL_ALPHA:
                    glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_ZERO);
                    glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_ZERO);
                    glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_ZERO);
                    glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
                    break;
            }
            break;
        }
    }
}

void GLRenderer_getTexParameter(GLRenderer* renderer, GLenum target, GLint level, GLenum pname, GLfloat* params) {
    GLTexture* texture = GLTexture_getBound(target);
    *params = 0;
    if (!texture) return;

    switch (pname) {
        case GL_TEXTURE_WIDTH:
            *params = TEXLEVEL(texture->width, level);
            break;
        case GL_TEXTURE_HEIGHT:
            *params = TEXLEVEL(texture->height, level);
            break;
        case GL_TEXTURE_DEPTH:
            *params = 0;
            break;
        case GL_TEXTURE_RED_TYPE:
        case GL_TEXTURE_GREEN_TYPE:
        case GL_TEXTURE_BLUE_TYPE:
        case GL_TEXTURE_ALPHA_TYPE:
        case GL_TEXTURE_DEPTH_TYPE:
            *params = GL_FLOAT;
            break;
        case GL_TEXTURE_RED_SIZE:
        case GL_TEXTURE_GREEN_SIZE:
        case GL_TEXTURE_BLUE_SIZE:
        case GL_TEXTURE_ALPHA_SIZE:
            *params = 8;
            break;
        case GL_TEXTURE_DEPTH_SIZE:
            *params = 0;
            break;
        case GL_TEXTURE_COMPRESSED:
            *params = isCompressedFormat(texture->originFormat);
            break;
        case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
            if (isCompressedFormat(texture->originFormat)) {
                *params = getCompressedImageSize(texture->originFormat, texture->width, texture->height, level);
            }
            else *params = texture->width * texture->height * 4;
            break;
        case GL_TEXTURE_INTERNAL_FORMAT:
            *params = texture->originFormat;
            break;
        case GL_TEXTURE_RESIDENT:
            *params = GL_TRUE;
            break;
        case GL_DEPTH_STENCIL_TEXTURE_MODE:
        case GL_IMAGE_FORMAT_COMPATIBILITY_TYPE:
        case GL_TEXTURE_BASE_LEVEL:
        case GL_TEXTURE_COMPARE_FUNC:
        case GL_TEXTURE_COMPARE_MODE:
        case GL_TEXTURE_IMMUTABLE_FORMAT:
        case GL_TEXTURE_IMMUTABLE_LEVELS:
        case GL_TEXTURE_MAG_FILTER:
        case GL_TEXTURE_MAX_LEVEL:
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_SWIZZLE_R:
        case GL_TEXTURE_SWIZZLE_G:
        case GL_TEXTURE_SWIZZLE_B:
        case GL_TEXTURE_SWIZZLE_A: {
            GLint paramsi;
            glGetTexParameteriv(target, pname, &paramsi);
            *params = paramsi;
            break;
        }
        case GL_TEXTURE_MAX_ANISOTROPY:
        case GL_TEXTURE_MIN_LOD:
        case GL_TEXTURE_MAX_LOD:
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        case GL_TEXTURE_WRAP_R:
        case GL_TEXTURE_BORDER_COLOR:
            glGetTexParameterfv(target, pname, params);
            break;
        default:
            println("gladio:getTexParameter: unimplemented pname %x", pname);
            break;
    }
}

void* GLRenderer_getTexImage(GLRenderer* renderer, GLenum target, GLint level, GLenum format, GLenum type, int* imageSize) {
    target = parseTexTarget(target);
    GLTexture* texture = GLTexture_getBound(target);
    *imageSize = texture ? computeTexImageDataSize(format, type, texture->width, texture->height, 1) : 0;
    if (*imageSize == 0) return NULL;

    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, texture->id, 0);

    void* pixels = malloc(*imageSize);
    GLRenderer_readPixels(0, 0, texture->width, texture->height, format, type, pixels);

    glBindFramebuffer(GL_FRAMEBUFFER, renderer->clientState.framebuffer[indexOfGLTarget(GL_FRAMEBUFFER)]);
    glDeleteFramebuffers(1, &framebuffer);
    return pixels;
}

void* GLRenderer_getCompressedTexImage(GLRenderer* renderer, GLenum target, GLint level, int* compressedSize) {
    target = parseTexTarget(target);
    GLTexture* texture = GLTexture_getBound(target);
    *compressedSize = texture ? getCompressedImageSize(texture->originFormat, texture->width, texture->height, level) : 0;
    if (*compressedSize == 0) return NULL;

    int imageSize;
    void* imageData = GLRenderer_getTexImage(renderer, target, level, GL_RGBA, GL_UNSIGNED_BYTE, &imageSize);

    void* compressedData = malloc(*compressedSize);
    compressTexImage2D(texture->originFormat, texture->width, texture->height, imageData, compressedData);
    free(imageData);
    return compressedData;
}

void GLRenderer_setDrawBuffer(GLRenderer* renderer, GLenum drawBuffer) {
    if (drawBuffer == GL_FRONT_LEFT || drawBuffer == GL_FRONT_RIGHT) {
        drawBuffer = GL_FRONT;
    }
    else if (drawBuffer == GL_BACK_LEFT || drawBuffer == GL_BACK_RIGHT) {
        drawBuffer = GL_BACK;
    }
    else if (drawBuffer == GL_NONE || (drawBuffer >= GL_COLOR_ATTACHMENT0 && drawBuffer <= GL_COLOR_ATTACHMENT31)) {
        GLFramebuffer_setDrawBuffers(1, &drawBuffer);
        return;
    }

    int framebuffer;
    if (drawBuffer == GL_FRONT) {
        framebuffer = renderer->displayBuffers[0];
        renderer->swapBuffers = false;
    }
    else framebuffer = renderer->displayBuffers[1];

    if (framebuffer != renderer->clientState.framebuffer[indexOfGLTarget(GL_FRAMEBUFFER)]) {
        GLFramebuffer_bind(GL_FRAMEBUFFER, framebuffer);
    }
}

void GLRenderer_enableVertexAttribute(GLRenderer* renderer, int location) {
    if (location >= 0 && !renderer->enabledVertexAttribs[location]) {
        glEnableVertexAttribArray(location);
        renderer->enabledVertexAttribs[location] = true;
    }
}

void GLRenderer_disableVertexAttribute(GLRenderer* renderer, int location) {
    if (location >= 0 && renderer->enabledVertexAttribs[location]) {
        glDisableVertexAttribArray(location);
        renderer->enabledVertexAttribs[location] = false;
    }
}

void GLRenderer_disableUnusedVertexAttributes(GLRenderer* renderer) {
    for (int i = 0; i < ARRAY_SIZE(renderer->enabledVertexAttribs); i++) {
        if (renderer->enabledVertexAttribs[i]) {
            glDisableVertexAttribArray(i);
            renderer->enabledVertexAttribs[i] = false;
        }
    }
}

void GLRenderer_readPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) {
    GLBuffer* pixelPackBuffer = GLBuffer_getBound(GL_PIXEL_PACK_BUFFER);
    bool convert = format != GL_RGBA && format != GL_BGRA;
    if (pixelPackBuffer && convert) glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    int srcDataSize;
    char* srcData = convert ? malloc(srcDataSize = (width * height * 4)) : pixels;
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, srcData);

    if (pixelPackBuffer) {
        if (!convert) {
            srcData = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, pixelPackBuffer->size, GL_MAP_READ_BIT);
            memcpy(pixelPackBuffer->mappedData, srcData, pixelPackBuffer->size);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        }
        else glBindBuffer(GL_PIXEL_PACK_BUFFER, pixelPackBuffer->id);
    }

    if (type == GL_BYTE ||
        type == GL_UNSIGNED_BYTE ||
        type == GL_UNSIGNED_INT_8_8_8_8 ||
        type == GL_UNSIGNED_INT_8_8_8_8_REV ||
        type == GL_UNSIGNED_SHORT_5_6_5) {
        char* dstData = pixelPackBuffer ? pixelPackBuffer->mappedData : pixels;

        switch (format) {
            case GL_RED:
            case GL_GREEN:
            case GL_BLUE:
                for (int i = 0, j = format - GL_RED, k = 0; i < srcDataSize; i += 4, k++) dstData[k] = srcData[i+j];
                break;
            case GL_RGB:
                if (type == GL_UNSIGNED_SHORT_5_6_5) {
                    for (int i = 0, j = 0; i < srcDataSize; i += 4, j += 2) {
                        uint8_t r = srcData[i+0];
                        uint8_t g = srcData[i+1];
                        uint8_t b = srcData[i+2];

                        dstData[j+0] = ((g & 0x1c) << 3) | (b >> 3);
                        dstData[j+1] = (r & 0xf8) | (g >> 5);
                    }
                }
                else {
                    for (int i = 0, j = 0; i < srcDataSize; i += 4, j += 3) {
                        dstData[j+0] = srcData[i+0];
                        dstData[j+1] = srcData[i+1];
                        dstData[j+2] = srcData[i+2];
                    }
                }
                break;
            case GL_BGR:
                for (int i = 0, j = 0; i < srcDataSize; i += 4, j += 3) {
                    dstData[j+0] = srcData[i+2];
                    dstData[j+1] = srcData[i+1];
                    dstData[j+2] = srcData[i+0];
                }
                break;
            case GL_BGRA:
                for (int i = 0; i < srcDataSize; i += 4) {
                    dstData[i+0] = srcData[i+2];
                    dstData[i+1] = srcData[i+1];
                    dstData[i+2] = srcData[i+0];
                    dstData[i+3] = srcData[i+3];
                }
                break;
            case GL_RGBA:
                break;
            default:
                println("gladio:readPixelData: unimplemented format %x", format);
                break;
        }
    }
    else println("gladio:readPixelData: unimplemented type %x", type);

    if (convert) free(srcData);
}