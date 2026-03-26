#include "request_handler.h"
#include "shader_converter.h"
#include "gl_formats.h"
#include "attrib_stack.h"
#include "gl_query.h"
#include "arb_program.h"

#define MSG_DEBUG_UNIMPLEMENTED_FUNC "%s not implemented yet"

void gd_handle_glAccum(GLContext* context) {
    GLenum op = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat value = ArrayBuffer_getFloat(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glAccum");
}

void gd_handle_glActiveTexture(GLContext* context) {
    GLenum texture = ArrayBuffer_getInt(&context->inputBuffer);

    GLTexture_setActiveUnit(texture);
}

void gd_handle_glAlphaFunc(GLContext* context) {
    GLenum func = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat ref = ArrayBuffer_getFloat(&context->inputBuffer);

    currentRenderer->state.alphaTest.func = func;
    currentRenderer->state.alphaTest.ref = ref;
}

void gd_handle_glArrayElement(GLContext* context) {
    GLint i = ArrayBuffer_getInt(&context->inputBuffer);

    for (int j = 0; j < VERTEX_ATTRIB_COUNT; j++) readVertexArrayElement(context, j, i);

    GLClientState* clientState = &currentRenderer->clientState;
    if (clientState->vao->attribs[POSITION_ARRAY_INDEX].state) {
        GLRenderer_addArrayElement(currentRenderer, currentRenderer->geometry.vertices.position-1);
    }
}

void gd_handle_glAttachShader(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint shader = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderConverter_attachShader(program, shader);
}

void gd_handle_glBegin(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);

    GLRenderer_beginImmediate(currentRenderer, mode);
}

void gd_handle_glBeginConditionalRender(GLContext* context) {
    GLuint id = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glBeginConditionalRender");
}

void gd_handle_glBeginQuery(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint id = ArrayBuffer_getInt(&context->inputBuffer);

    GLQuery_begin(target, id);
}

void gd_handle_glBeginTransformFeedback(GLContext* context) {
    GLenum primitiveMode = ArrayBuffer_getInt(&context->inputBuffer);

    glBeginTransformFeedback(primitiveMode);
}

void gd_handle_glBindAttribLocation(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLchar* name = context->inputBuffer.buffer + context->inputBuffer.position;

    glBindAttribLocation(program, index, name);
}

void gd_handle_glBindBuffer(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint buffer = ArrayBuffer_getInt(&context->inputBuffer);

    GLBuffer_bind(target, buffer);
}

void gd_handle_glBindBufferBase(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint buffer = ArrayBuffer_getInt(&context->inputBuffer);

    glBindBufferBase(target, index, buffer);
}

void gd_handle_glBindBufferRange(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint buffer = ArrayBuffer_getInt(&context->inputBuffer);
    GLintptr offset = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizeiptr size = ArrayBuffer_getInt(&context->inputBuffer);

    glBindBufferRange(target, index, buffer, offset, size);
}

void gd_handle_glBindFragDataLocation(GLContext* context) {
    GLuint programId = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint color = ArrayBuffer_getInt(&context->inputBuffer);
    GLchar* name = context->inputBuffer.buffer + context->inputBuffer.position;

    ShaderProgram* program = ShaderConverter_getProgram(programId);
    if (program) SparseArray_put(&program->fragDataLocations, color, strdup(name));
}

void gd_handle_glBindFragDataLocationIndexed(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glBindFragDataLocationIndexed");
}

void gd_handle_glBindFramebuffer(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint framebuffer = ArrayBuffer_getInt(&context->inputBuffer);

    GLFramebuffer_bind(target, framebuffer);
}

void gd_handle_glBindProgramARB(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);

    ARBProgram_bind(target, program);
}

void gd_handle_glBindRenderbuffer(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint renderbuffer = ArrayBuffer_getInt(&context->inputBuffer);

    currentRenderer->clientState.renderbuffer = renderbuffer;
    glBindRenderbuffer(target, renderbuffer);
}

void gd_handle_glBindSampler(GLContext* context) {
    GLuint unit = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint sampler = ArrayBuffer_getInt(&context->inputBuffer);

    glBindSampler(unit, sampler);
}

void gd_handle_glBindTexture(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint texture = ArrayBuffer_getInt(&context->inputBuffer);

    GLTexture_bind(parseTexTarget(target), texture);
}

void gd_handle_glBindVertexArray(GLContext* context) {
    GLuint array = ArrayBuffer_getInt(&context->inputBuffer);

    GLVertexArrayObject_setBound(&currentRenderer->clientState, array);
    glBindVertexArray(array);
}

void gd_handle_glBitmap(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glBitmap");
}

void gd_handle_glBlendColor(GLContext* context) {
    GLclampf red = ArrayBuffer_getFloat(&context->inputBuffer);
    GLclampf green = ArrayBuffer_getFloat(&context->inputBuffer);
    GLclampf blue = ArrayBuffer_getFloat(&context->inputBuffer);
    GLclampf alpha = ArrayBuffer_getFloat(&context->inputBuffer);

    glBlendColor(red, green, blue, alpha);
}

void gd_handle_glBlendEquation(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);

    glBlendEquation(mode);
}

void gd_handle_glBlendEquationSeparate(GLContext* context) {
    GLenum modeRGB = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum modeAlpha = ArrayBuffer_getInt(&context->inputBuffer);

    glBlendEquationSeparate(modeRGB, modeAlpha);
}

void gd_handle_glBlendEquationSeparatei(GLContext* context) {
    GLuint buf = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum modeRGB = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum modeAlpha = ArrayBuffer_getInt(&context->inputBuffer);

    glBlendEquationSeparatei(buf, modeRGB, modeAlpha);
}

void gd_handle_glBlendEquationi(GLContext* context) {
    GLuint buf = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);

    glBlendEquationi(buf, mode);
}

void gd_handle_glBlendFunc(GLContext* context) {
    GLenum sfactor = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum dfactor = ArrayBuffer_getInt(&context->inputBuffer);

    glBlendFunc(sfactor, dfactor);
}

void gd_handle_glBlendFuncSeparate(GLContext* context) {
    GLenum sfactorRGB = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum dfactorRGB = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum sfactorAlpha = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum dfactorAlpha = ArrayBuffer_getInt(&context->inputBuffer);

    glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

void gd_handle_glBlendFuncSeparatei(GLContext* context) {
    GLuint buf = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum srcRGB = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum dstRGB = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum srcAlpha = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum dstAlpha = ArrayBuffer_getInt(&context->inputBuffer);

    glBlendFuncSeparatei(buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

void gd_handle_glBlendFunci(GLContext* context) {
    GLuint buf = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum src = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum dst = ArrayBuffer_getInt(&context->inputBuffer);

    glBlendFunci(buf, src, dst);
}

void gd_handle_glBlitFramebuffer(GLContext* context) {
    GLint srcX0 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint srcY0 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint srcX1 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint srcY1 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint dstX0 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint dstY0 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint dstX1 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint dstY1 = ArrayBuffer_getInt(&context->inputBuffer);
    GLbitfield mask = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum filter = ArrayBuffer_getInt(&context->inputBuffer);

    glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void gd_handle_glBufferData(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizeiptr size = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum usage = ArrayBuffer_getInt(&context->inputBuffer);

    GLBuffer* buffer = GLBuffer_getBound(target);
    if (buffer && buffer->mappedData) {
        glBufferData(target, size, buffer->mappedData, usage);
    }
    gl_send(context->clientRing, REQUEST_CODE_GL_BUFFER_DATA, NULL, 0);
}

void gd_handle_glBufferStorage(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizeiptr size = ArrayBuffer_getInt(&context->inputBuffer);
    GLbitfield flags = ArrayBuffer_getInt(&context->inputBuffer);

    GLBuffer* buffer = GLBuffer_getBound(target);
    int fd = GLBuffer_allocateMemory(buffer, size);

    bool success = fd > 0;
    if (success && target != GL_PIXEL_UNPACK_BUFFER) {
        GLenum usage = (flags & GL_DYNAMIC_STORAGE_BIT) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
        glBufferData(target, size, NULL, usage);
    }

    send_fds(context->clientFd, &fd, 1, &success, sizeof(bool));
    CLOSEFD(fd);
}

void gd_handle_glBufferSubData(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLintptr offset = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizeiptr size = ArrayBuffer_getInt(&context->inputBuffer);

    GLBuffer* buffer = GLBuffer_getBound(target);
    if (buffer && buffer->mappedData) {
        glBufferSubData(target, offset, size, buffer->mappedData + offset);
    }
    gl_send(context->clientRing, REQUEST_CODE_GL_BUFFER_SUB_DATA, NULL, 0);
}

void gd_handle_glCallList(GLContext* context) {
    GLuint list = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glCallList");
}

void gd_handle_glCallLists(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glCallLists");
}

void gd_handle_glCheckFramebufferStatus(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);

    GLenum result = glCheckFramebufferStatus(target);
    gl_send(context->clientRing, REQUEST_CODE_GL_CHECK_FRAMEBUFFER_STATUS, &result, sizeof(GLenum));
}

void gd_handle_glClampColor(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum clamp = ArrayBuffer_getInt(&context->inputBuffer);

    if (target == GL_CLAMP_READ_COLOR) {
        currentRenderer->state.clampReadColor = clamp == GL_TRUE ? true : false;
    }
}

void gd_handle_glClear(GLContext* context) {
    GLbitfield mask = ArrayBuffer_getInt(&context->inputBuffer);

    glClear(mask);
}

void gd_handle_glClearAccum(GLContext* context) {
    GLfloat red = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat green = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat blue = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat alpha = ArrayBuffer_getFloat(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glClearAccum");
}

void gd_handle_glClearBufferfi(GLContext* context) {
    GLenum buffer = ArrayBuffer_getInt(&context->inputBuffer);
    GLint drawbuffer = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat depth = ArrayBuffer_getFloat(&context->inputBuffer);
    GLint stencil = ArrayBuffer_getInt(&context->inputBuffer);

    glClearBufferfi(buffer, drawbuffer, depth, stencil);
}

void gd_handle_glClearBufferfv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glClearBufferfv");
}

void gd_handle_glClearBufferiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glClearBufferiv");
}

void gd_handle_glClearBufferuiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glClearBufferuiv");
}

void gd_handle_glClearColor(GLContext* context) {
    GLclampf red = ArrayBuffer_getFloat(&context->inputBuffer);
    GLclampf green = ArrayBuffer_getFloat(&context->inputBuffer);
    GLclampf blue = ArrayBuffer_getFloat(&context->inputBuffer);
    GLclampf alpha = ArrayBuffer_getFloat(&context->inputBuffer);

    glClearColor(red, green, blue, alpha);
}

void gd_handle_glClearDepthf(GLContext* context) {
    GLfloat d = ArrayBuffer_getFloat(&context->inputBuffer);

    glClearDepthf(d);
}

void gd_handle_glClearIndex(GLContext* context) {
    GLfloat c = ArrayBuffer_getFloat(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glClearIndex");
}

void gd_handle_glClearStencil(GLContext* context) {
    GLint s = ArrayBuffer_getInt(&context->inputBuffer);

    glClearStencil(s);
}

void gd_handle_glClientActiveTexture(GLContext* context) {
    GLenum texture = ArrayBuffer_getInt(&context->inputBuffer);

    GLTexture_setActiveUnit(texture);
}

void gd_handle_glClientWaitSync(GLContext* context) {
    GLsync sync = (GLsync)ArrayBuffer_getLong(&context->inputBuffer);
    GLbitfield flags = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint64 timeout = ArrayBuffer_getLong(&context->inputBuffer);

    GLenum result = glClientWaitSync(sync, flags, timeout);
    gl_send(context->clientRing, REQUEST_CODE_GL_CLIENT_WAIT_SYNC, &result, sizeof(GLenum));
}

void gd_handle_glClipPlane(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glClipPlane");
}

void gd_handle_glColor4f(GLContext* context) {
    GLfloat red = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat green = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat blue = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat alpha = ArrayBuffer_getFloat(&context->inputBuffer);

    currentRenderer->state.color[0] = red;
    currentRenderer->state.color[1] = green;
    currentRenderer->state.color[2] = blue;
    currentRenderer->state.color[3] = alpha;

    currentRenderer->geometry.colors.position++;
    if (currentRenderer->state.colorMaterial.enabled) {
        float color[] = {red, green, blue, alpha};
        GLRenderer_setMaterialParams(currentRenderer, currentRenderer->state.colorMaterial.face, currentRenderer->state.colorMaterial.mode, color);
    }
}

void gd_handle_glColorMask(GLContext* context) {
    GLboolean red = ArrayBuffer_get(&context->inputBuffer);
    GLboolean green = ArrayBuffer_get(&context->inputBuffer);
    GLboolean blue = ArrayBuffer_get(&context->inputBuffer);
    GLboolean alpha = ArrayBuffer_get(&context->inputBuffer);

    glColorMask(red, green, blue, alpha);
}

void gd_handle_glColorMaski(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLboolean r = ArrayBuffer_get(&context->inputBuffer);
    GLboolean g = ArrayBuffer_get(&context->inputBuffer);
    GLboolean b = ArrayBuffer_get(&context->inputBuffer);
    GLboolean a = ArrayBuffer_get(&context->inputBuffer);

    glColorMaski(index, r, g, b, a);
}

void gd_handle_glColorMaterial(GLContext* context) {
    GLenum face = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);

    currentRenderer->state.colorMaterial.face = face;
    currentRenderer->state.colorMaterial.mode = mode;
}

void gd_handle_glColorPointer(GLContext* context) {
    GL_READ_VERTEX_ARRAY(COLOR_ARRAY_INDEX);
}

void gd_handle_glCompileShader(GLContext* context) {
    GLuint shaderId = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderObject* shader = ShaderConverter_getShader(shaderId);
    if (shader) shader->compileStatus = COMPILE_STATUS_PENDING;
}

void gd_handle_glCompressedTexImage2D(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);
    GLint internalformat = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);
    GLint border = ArrayBuffer_getInt(&context->inputBuffer);
    GLint imageSize = ArrayBuffer_getInt(&context->inputBuffer);

    void* decompressedData = NULL;
    if (imageSize > 0) {
        GLBuffer* pixelUnpackBuffer = GLBuffer_getBound(GL_PIXEL_UNPACK_BUFFER);
        if (pixelUnpackBuffer) {
            uint64_t pointer = ArrayBuffer_getInt(&context->inputBuffer);
            decompressedData = decompressTexImage2D(internalformat, width, height, pixelUnpackBuffer->mappedData + pointer, context->threadPool);
            gl_send(context->clientRing, REQUEST_CODE_GL_COMPRESSED_TEX_IMAGE2D, NULL, 0);
        }
        else {
            void* imageData = NULL;
            RING_READ_BEGIN(context->serverRing, imageData, imageSize);
            decompressedData = decompressTexImage2D(internalformat, width, height, imageData, context->threadPool);
            RING_READ_END(context->serverRing);
        }
    }

    target = parseTexTarget(target);
    glTexImage2D(target, level, GL_BGRA, width, height, border, GL_BGRA, GL_UNSIGNED_BYTE, decompressedData);
    MEMFREE(decompressedData);

    GLTexture* texture = GLTexture_getBound(target);
    if (texture && level == 0) {
        texture->width = width;
        texture->height = height;
        texture->originFormat = internalformat;
        if (texture->generateMipmap) glGenerateMipmap(target);
    }
}

void gd_handle_glCompressedTexSubImage2D(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);
    GLint xoffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLint yoffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum format = ArrayBuffer_getInt(&context->inputBuffer);
    GLint imageSize = ArrayBuffer_getInt(&context->inputBuffer);

    void* decompressedData = NULL;
    if (imageSize > 0) {
        GLBuffer* pixelUnpackBuffer = GLBuffer_getBound(GL_PIXEL_UNPACK_BUFFER);
        if (pixelUnpackBuffer) {
            uint64_t pointer = ArrayBuffer_getInt(&context->inputBuffer);
            decompressedData = decompressTexImage2D(format, width, height, pixelUnpackBuffer->mappedData + pointer, context->threadPool);
            gl_send(context->clientRing, REQUEST_CODE_GL_COMPRESSED_TEX_SUB_IMAGE2D, NULL, 0);
        }
        else {
            void* imageData = NULL;
            RING_READ_BEGIN(context->serverRing, imageData, imageSize);
            decompressedData = decompressTexImage2D(format, width, height, imageData, context->threadPool);
            RING_READ_END(context->serverRing);
        }
    }

    glTexSubImage2D(parseTexTarget(target), level, xoffset, yoffset, width, height, GL_BGRA, GL_UNSIGNED_BYTE, decompressedData);
    MEMFREE(decompressedData);
}

void gd_handle_glCopyBufferSubData(GLContext* context) {
    GLenum readTarget = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum writeTarget = ArrayBuffer_getInt(&context->inputBuffer);
    GLintptr readOffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLintptr writeOffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizeiptr size = ArrayBuffer_getInt(&context->inputBuffer);

    glCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
}

void gd_handle_glCopyPixels(GLContext* context) {
    GLint x = ArrayBuffer_getInt(&context->inputBuffer);
    GLint y = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glCopyPixels");
}

void gd_handle_glCopyTexImage2D(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum internalformat = ArrayBuffer_getInt(&context->inputBuffer);
    GLint x = ArrayBuffer_getInt(&context->inputBuffer);
    GLint y = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);
    GLint border = ArrayBuffer_getInt(&context->inputBuffer);

    glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

void gd_handle_glCopyTexSubImage2D(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);
    GLint xoffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLint yoffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLint x = ArrayBuffer_getInt(&context->inputBuffer);
    GLint y = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);

    glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

void gd_handle_glCopyTexSubImage3D(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);
    GLint xoffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLint yoffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLint zoffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLint x = ArrayBuffer_getInt(&context->inputBuffer);
    GLint y = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);

    glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

void gd_handle_glCreateProgram(GLContext* context) {
    GLuint result = ShaderConverter_createProgram();

    gl_send(context->clientRing, REQUEST_CODE_GL_CREATE_PROGRAM, &result, sizeof(GLuint));
}

void gd_handle_glCreateShader(GLContext* context) {
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);

    GLuint result = ShaderConverter_createShader(type);
    gl_send(context->clientRing, REQUEST_CODE_GL_CREATE_SHADER, &result, sizeof(GLuint));
}

void gd_handle_glCullFace(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);

    glCullFace(mode);
}

void gd_handle_glDeleteBuffers(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint* buffers = ArrayBuffer_getBytes(&context->inputBuffer, n * sizeof(GLuint));

    for (int i = 0, j; i < n; i++) GLBuffer_delete(buffers[i]);
}

void gd_handle_glDeleteFramebuffers(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint* framebuffers = ArrayBuffer_getBytes(&context->inputBuffer, n * sizeof(GLuint));

    for (int i = 0, j; i < n; i++) GLFramebuffer_delete(framebuffers[i]);
}

void gd_handle_glDeleteLists(GLContext* context) {
    GLuint list = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei range = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glDeleteLists");
}

void gd_handle_glDeleteObjectARB(GLContext* context) {
    GLuint obj = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glDeleteObjectARB");
}

void gd_handle_glDeleteProgram(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);

    if (currentRenderer->clientState.program &&
        currentRenderer->clientState.program->id == program) {
        currentRenderer->clientState.program = NULL;
    }

    ShaderConverter_deleteProgram(program);
}

void gd_handle_glDeleteProgramsARB(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint* programs = ArrayBuffer_getBytes(&context->inputBuffer, n * sizeof(GLuint));

    for (int i = 0; i < n; i++) ARBProgram_delete(programs[i]);
}

void gd_handle_glDeleteQueries(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint* ids = ArrayBuffer_getBytes(&context->inputBuffer, n * sizeof(GLuint));

    for (int i = 0; i < n; i++) GLQuery_delete(ids[i]);
}

void gd_handle_glDeleteRenderbuffers(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint* renderbuffers = ArrayBuffer_getBytes(&context->inputBuffer, n * sizeof(GLuint));

    for (int i = 0; i < n; i++) {
        if (renderbuffers[i] == currentRenderer->clientState.renderbuffer) {
            currentRenderer->clientState.renderbuffer = 0;
            break;
        }
    }

    glDeleteRenderbuffers(n, renderbuffers);
}

void gd_handle_glDeleteSamplers(GLContext* context) {
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint* samplers = ArrayBuffer_getBytes(&context->inputBuffer, count * sizeof(GLuint));

    glDeleteSamplers(count, samplers);
}

void gd_handle_glDeleteShader(GLContext* context) {
    GLuint shader = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderConverter_deleteShader(shader);
}

void gd_handle_glDeleteSync(GLContext* context) {
    long sync = ArrayBuffer_getLong(&context->inputBuffer);

    glDeleteSync((GLsync)sync);
}

void gd_handle_glDeleteTextures(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint* textures = ArrayBuffer_getBytes(&context->inputBuffer, n * sizeof(GLuint));

    for (int i = 0; i < n; i++) GLTexture_delete(textures[i]);
}

void gd_handle_glDeleteVertexArrays(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint* arrays = ArrayBuffer_getBytes(&context->inputBuffer, n * sizeof(GLuint));

    for (int i = 0; i < n; i++) GLVertexArrayObject_delete(&currentRenderer->clientState, arrays[i]);
    glDeleteVertexArrays(n, arrays);
}

void gd_handle_glDepthFunc(GLContext* context) {
    GLenum func = ArrayBuffer_getInt(&context->inputBuffer);

    glDepthFunc(func);
}

void gd_handle_glDepthMask(GLContext* context) {
    GLboolean flag = ArrayBuffer_get(&context->inputBuffer);

    glDepthMask(flag);
}

void gd_handle_glDepthRangef(GLContext* context) {
    GLfloat n = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat f = ArrayBuffer_getFloat(&context->inputBuffer);

    glDepthRangef(n, f);
}

void gd_handle_glDetachShader(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint shader = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderConverter_detachShader(program, shader);
}

void gd_handle_glDisable(GLContext* context) {
    GLenum cap = ArrayBuffer_getInt(&context->inputBuffer);

    GLRenderer_setCapabilityState(currentRenderer, cap, false, -1);
}

void gd_handle_glDisableClientState(GLContext* context) {
    GLenum array = ArrayBuffer_getInt(&context->inputBuffer);

    GLVertexArrayObject_setAttribState(&currentRenderer->clientState, array, VERTEX_ATTRIB_DISABLED, false);
}

void gd_handle_glDisableVertexAttribArray(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);

    GLVertexArrayObject_setAttribState(&currentRenderer->clientState, index, VERTEX_ATTRIB_DISABLED, false);
    glDisableVertexAttribArray(index);
}

void gd_handle_glDisablei(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);

    GLRenderer_setCapabilityState(currentRenderer, target, false, index);
}

void gd_handle_glDispatchCompute(GLContext* context) {
    GLuint num_groups_x = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint num_groups_y = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint num_groups_z = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glDispatchCompute");
}

void gd_handle_glDispatchComputeIndirect(GLContext* context) {
    GLintptr indirect = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glDispatchComputeIndirect");
}

void gd_handle_glDrawArrays(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);
    GLint first = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderConverter_updateBoundProgram();
    if (readUnboundVertexArrays(context, mode, count, NULL, GL_NONE)) return;
    glDrawArrays(mode, first, count);
}

void gd_handle_glDrawArraysInstanced(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);
    GLint first = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei instancecount = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderConverter_updateBoundProgram();
    if (readUnboundVertexArrays(context, mode, count, NULL, GL_NONE)) return;
    glDrawArraysInstanced(mode, first, count, instancecount);
}

void gd_handle_glDrawBuffer(GLContext* context) {
    GLenum buf = ArrayBuffer_getInt(&context->inputBuffer);

    GLRenderer_setDrawBuffer(currentRenderer, buf);
}

void gd_handle_glDrawBuffers(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum* bufs = ArrayBuffer_getBytes(&context->inputBuffer, n * sizeof(GLenum));

    GLFramebuffer_setDrawBuffers(n, bufs);
}

void gd_handle_glDrawElements(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderConverter_updateBoundProgram();
    void* indices = NULL;
    if (readUnboundVertexArrays(context, mode, count, &indices, type)) return;
    glDrawElements(mode, count, type, indices);
}

void gd_handle_glDrawElementsBaseVertex(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);
    GLint basevertex = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderConverter_updateBoundProgram();
    void* indices = NULL;
    if (readUnboundVertexArrays(context, mode, count, &indices, type)) return;
    glDrawElementsBaseVertex(mode, count, type, indices, basevertex);
}

void gd_handle_glDrawElementsInstanced(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei instancecount = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderConverter_updateBoundProgram();
    void* indices = NULL;
    if (readUnboundVertexArrays(context, mode, count, &indices, type)) return;
    glDrawElementsInstanced(mode, count, type, indices, instancecount);
}

void gd_handle_glDrawElementsInstancedBaseVertex(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei instancecount = ArrayBuffer_getInt(&context->inputBuffer);
    GLint basevertex = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderConverter_updateBoundProgram();
    void* indices = NULL;
    if (readUnboundVertexArrays(context, mode, count, &indices, type)) return;
    glDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex);
}

void gd_handle_glDrawPixels(GLContext* context) {
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum format = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);
    GLint imageSize = ArrayBuffer_getInt(&context->inputBuffer);

    void* pixels = NULL;
    RING_READ_BEGIN(context->serverRing, pixels, imageSize);
    GLRenderer_drawPixels(currentRenderer, width, height, format, type, pixels);
    RING_READ_END(context->serverRing);
}

void gd_handle_glDrawRangeElements(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint start = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint end = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderConverter_updateBoundProgram();
    void* indices = NULL;
    if (readUnboundVertexArrays(context, mode, count, &indices, type)) return;
    glDrawRangeElements(mode, start, end, count, type, indices);
}

void gd_handle_glDrawRangeElementsBaseVertex(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint start = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint end = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);
    GLint basevertex = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderConverter_updateBoundProgram();
    void* indices = NULL;
    if (readUnboundVertexArrays(context, mode, count, &indices, type)) return;
    glDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
}

void gd_handle_glEdgeFlag(GLContext* context) {
    GLboolean flag = ArrayBuffer_get(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glEdgeFlag");
}

void gd_handle_glEdgeFlagPointer(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glEdgeFlagPointer");
}

void gd_handle_glEdgeFlagv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glEdgeFlagv");
}

void gd_handle_glEnable(GLContext* context) {
    GLenum cap = ArrayBuffer_getInt(&context->inputBuffer);

    GLRenderer_setCapabilityState(currentRenderer, cap, true, -1);
}

void gd_handle_glEnableClientState(GLContext* context) {
    GLenum array = ArrayBuffer_getInt(&context->inputBuffer);

    GLVertexArrayObject_setAttribState(&currentRenderer->clientState, array, VERTEX_ATTRIB_LEGACY_ENABLED, false);
}

void gd_handle_glEnableVertexAttribArray(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);

    glEnableVertexAttribArray(index);
}

void gd_handle_glEnablei(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);

    GLRenderer_setCapabilityState(currentRenderer, target, true, index);
}

void gd_handle_glEnd(GLContext* context) {
    readCommandBuffer(context);
}

void gd_handle_glEndConditionalRender(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glEndConditionalRender");
}

void gd_handle_glEndList(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glEndList");
}

void gd_handle_glEndQuery(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);

    GLQuery_end(target);
}

void gd_handle_glEndTransformFeedback(GLContext* context) {
    glEndTransformFeedback();
}

void gd_handle_glEvalCoord2f(GLContext* context) {
    GLfloat u = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat v = ArrayBuffer_getFloat(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glEvalCoord2f");
}

void gd_handle_glEvalMesh1(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);
    GLint i1 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint i2 = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glEvalMesh1");
}

void gd_handle_glEvalMesh2(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);
    GLint i1 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint i2 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint j1 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint j2 = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glEvalMesh2");
}

void gd_handle_glEvalPoint1(GLContext* context) {
    GLint i = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glEvalPoint1");
}

void gd_handle_glEvalPoint2(GLContext* context) {
    GLint i = ArrayBuffer_getInt(&context->inputBuffer);
    GLint j = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glEvalPoint2");
}

void gd_handle_glFeedbackBuffer(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glFeedbackBuffer");
}

void gd_handle_glFenceSync(GLContext* context) {
    GLenum condition = ArrayBuffer_getInt(&context->inputBuffer);
    GLbitfield flags = ArrayBuffer_getInt(&context->inputBuffer);

    GLsync sync = glFenceSync(condition, flags);
    gl_send(context->clientRing, REQUEST_CODE_GL_FENCE_SYNC, &sync, sizeof(long));
}

void gd_handle_glFinish(GLContext* context) {
    glFinish();

    gl_send(context->clientRing, REQUEST_CODE_GL_FINISH, NULL, 0);
}

void gd_handle_glFlush(GLContext* context) {
    glFlush();
}

void gd_handle_glFlushMappedBufferRange(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLintptr offset = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizeiptr length = ArrayBuffer_getInt(&context->inputBuffer);

    GLBuffer* buffer = GLBuffer_getBound(target);
    if (buffer) {
        if (buffer->mappedData) {
            void* dstData = glMapBufferRange(target, offset, length, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
            if (dstData) {
                memcpy(dstData, buffer->mappedData + offset, length);
                glUnmapBuffer(target);
            }
        }
    }

    gl_send(context->clientRing, REQUEST_CODE_GL_FLUSH_MAPPED_BUFFER_RANGE, NULL, 0);
}

void gd_handle_glFogCoordPointer(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glFogCoordPointer");
}

void gd_handle_glFogCoordf(GLContext* context) {
    GLfloat coord = ArrayBuffer_getFloat(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glFogCoordf");
}

void gd_handle_glFogf(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat param = ArrayBuffer_getFloat(&context->inputBuffer);

    GLRenderer_setFogParams(currentRenderer, pname, &param);
}

void gd_handle_glFogfv(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    int paramCount = getGLCallParamsCount(pname, NULL);
    GLfloat* params = ArrayBuffer_getBytes(&context->inputBuffer, paramCount * sizeof(float));

    GLRenderer_setFogParams(currentRenderer, pname, params);
}

void gd_handle_glFramebufferRenderbuffer(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum attachment = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum renderbuffertarget = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint renderbuffer = ArrayBuffer_getInt(&context->inputBuffer);

    GLFramebuffer_setAttachment(target, attachment, renderbuffertarget, renderbuffer, 0);
}

void gd_handle_glFramebufferTexture(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum attachment = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint texture = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);

    GLenum textarget = GLTexture_getType(texture);
    GLFramebuffer_setAttachment(target, attachment, textarget, texture, level);
}

void gd_handle_glFramebufferTexture2D(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum attachment = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum textarget = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint texture = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);

    GLFramebuffer_setAttachment(target, attachment, parseTexTarget(textarget), texture, level);
}

void gd_handle_glFramebufferTexture3D(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum attachment = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum textarget = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint texture = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);
    GLint zoffset = ArrayBuffer_getInt(&context->inputBuffer);

    GLFramebuffer_setAttachment(target, attachment, parseTexTarget(textarget), texture, level);
}

void gd_handle_glFramebufferTextureLayer(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum attachment = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint texture = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);
    GLint layer = ArrayBuffer_getInt(&context->inputBuffer);

    GLenum textarget = GLTexture_getType(texture);
    GLFramebuffer_setAttachment(target, attachment, textarget, texture, level);
}

void gd_handle_glFrontFace(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);

    glFrontFace(mode);
}

void gd_handle_glFrustum(GLContext* context) {
    GLdouble left = ArrayBuffer_getDouble(&context->inputBuffer);
    GLdouble right = ArrayBuffer_getDouble(&context->inputBuffer);
    GLdouble bottom = ArrayBuffer_getDouble(&context->inputBuffer);
    GLdouble top = ArrayBuffer_getDouble(&context->inputBuffer);
    GLdouble near_val = ArrayBuffer_getDouble(&context->inputBuffer);
    GLdouble far_val = ArrayBuffer_getDouble(&context->inputBuffer);

    float* matrix = GLRenderer_getCurrentMatrix(currentRenderer);
    float tmpMatrix[16];
    mat4_frustum(tmpMatrix, left, right, bottom, top, near_val, far_val);
    mat4_multiply(matrix, matrix, tmpMatrix);
}

void gd_handle_glGenBuffers(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);

    GLuint buffers[n];
    glGenBuffers(n, buffers);
    gl_send(context->clientRing, REQUEST_CODE_GL_GEN_BUFFERS, buffers, sizeof(buffers));
}

void gd_handle_glGenFramebuffers(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);

    GLuint framebuffers[n];
    for (int i = 0; i < n; i++) framebuffers[i] = GLFramebuffer_create();
    gl_send(context->clientRing, REQUEST_CODE_GL_GEN_FRAMEBUFFERS, framebuffers, sizeof(framebuffers));
}

void gd_handle_glGenLists(GLContext* context) {
    GLsizei range = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGenLists");
}

void gd_handle_glGenProgramsARB(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);

    GLuint programs[n];
    for (int i = 0; i < n; i++) programs[i] = ARBProgram_create()->id;
    gl_send(context->clientRing, REQUEST_CODE_GL_GEN_PROGRAMS_ARB, programs, sizeof(programs));
}

void gd_handle_glGenQueries(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);

    GLuint ids[n];
    for (int i = 0; i < n; i++) ids[i] = GLQuery_create();
    gl_send(context->clientRing, REQUEST_CODE_GL_GEN_QUERIES, ids, sizeof(ids));
}

void gd_handle_glGenRenderbuffers(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);

    GLuint renderbuffers[n];
    glGenRenderbuffers(n, renderbuffers);
    gl_send(context->clientRing, REQUEST_CODE_GL_GEN_RENDERBUFFERS, renderbuffers, sizeof(renderbuffers));
}

void gd_handle_glGenSamplers(GLContext* context) {
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);

    GLuint samplers[count];
    glGenSamplers(count, samplers);
    gl_send(context->clientRing, REQUEST_CODE_GL_GEN_SAMPLERS, samplers, sizeof(samplers));
}

void gd_handle_glGenTextures(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);

    GLuint textures[n];
    glGenTextures(n, textures);
    gl_send(context->clientRing, REQUEST_CODE_GL_GEN_TEXTURES, textures, sizeof(textures));
}

void gd_handle_glGenVertexArrays(GLContext* context) {
    GLsizei n = ArrayBuffer_getInt(&context->inputBuffer);

    GLuint arrays[n];
    glGenVertexArrays(n, arrays);
    gl_send(context->clientRing, REQUEST_CODE_GL_GEN_VERTEX_ARRAYS, arrays, sizeof(arrays));
}

void gd_handle_glGenerateMipmap(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);

    glGenerateMipmap(target);
}

void gd_handle_glGetActiveAttrib(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei bufSize = ArrayBuffer_getInt(&context->inputBuffer);

    GLsizei length;
    GLint size;
    GLenum type;
    GLchar name[bufSize];
    glGetActiveAttrib(program, index, bufSize, &length, &size, &type, name);

    ArrayBuffer_rewind(&context->outputBuffer);
    ArrayBuffer_putInt(&context->outputBuffer, length);
    ArrayBuffer_putInt(&context->outputBuffer, size);
    ArrayBuffer_putInt(&context->outputBuffer, type);
    ArrayBuffer_putBytes(&context->outputBuffer, name, bufSize);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_ACTIVE_ATTRIB, context->outputBuffer.buffer, context->outputBuffer.size);
}

void gd_handle_glGetActiveUniform(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei bufSize = ArrayBuffer_getInt(&context->inputBuffer);

    GLsizei length;
    GLint size;
    GLenum type;
    GLchar name[bufSize];
    glGetActiveUniform(program, index, bufSize, &length, &size, &type, name);

    ArrayBuffer_rewind(&context->outputBuffer);
    ArrayBuffer_putInt(&context->outputBuffer, length);
    ArrayBuffer_putInt(&context->outputBuffer, size);
    ArrayBuffer_putInt(&context->outputBuffer, type);
    ArrayBuffer_putBytes(&context->outputBuffer, name, bufSize);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_ACTIVE_UNIFORM, context->outputBuffer.buffer, context->outputBuffer.size);
}

void gd_handle_glGetActiveUniformBlockName(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetActiveUniformBlockName");
}

void gd_handle_glGetActiveUniformBlockiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetActiveUniformBlockiv");
}

void gd_handle_glGetActiveUniformName(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetActiveUniformName");
}

void gd_handle_glGetActiveUniformsiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetActiveUniformsiv");
}

void gd_handle_glGetAttachedShaders(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetAttachedShaders");
}

void gd_handle_glGetAttribLocation(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);
    GLchar* name = context->inputBuffer.buffer + context->inputBuffer.position;

    GLint result = glGetAttribLocation(program, name);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_ATTRIB_LOCATION, &result, sizeof(GLint));
}

void gd_handle_glGetBooleani_v(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetBooleani_v");
}

void gd_handle_glGetBooleanv(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);

    int paramSize = GLRenderer_getParamsv(currentRenderer, pname, GL_BOOL, NULL);
    char params[paramSize];
    GLRenderer_getParamsv(currentRenderer, pname, GL_BOOL, params);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_BOOLEANV, params, paramSize);
}

void gd_handle_glGetClipPlane(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetClipPlane");
}

void gd_handle_glGetCompressedTexImage(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);

    int compressedSize;
    void* compressedData = GLRenderer_getCompressedTexImage(currentRenderer, target, level, &compressedSize);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_COMPRESSED_TEX_IMAGE, &compressedSize, sizeof(int));

    if (compressedSize > 0) {
        RingBuffer_write(context->clientRing, compressedData, compressedSize);
        free(compressedData);
    }
}

void gd_handle_glGetError(GLContext* context) {
    GLenum error = glGetError();

    gl_send(context->clientRing, REQUEST_CODE_GL_GET_ERROR, &error, sizeof(GLenum));
}

void gd_handle_glGetFloatv(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);

    int paramSize = GLRenderer_getParamsv(currentRenderer, pname, GL_FLOAT, NULL);
    char params[paramSize];
    GLRenderer_getParamsv(currentRenderer, pname, GL_FLOAT, params);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_FLOATV, params, paramSize);
}

void gd_handle_glGetFragDataIndex(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetFragDataIndex");
}

void gd_handle_glGetFragDataLocation(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetFragDataLocation");
}

void gd_handle_glGetFramebufferAttachmentParameteriv(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum attachment = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);

    GLint params = 0;
    glGetFramebufferAttachmentParameteriv(target, attachment, pname, &params);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_FRAMEBUFFER_ATTACHMENT_PARAMETERIV, &params, sizeof(GLint));
}

void gd_handle_glGetHandleARB(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetHandleARB");
}

void gd_handle_glGetInfoLogARB(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetInfoLogARB");
}

void gd_handle_glGetInteger64i_v(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetInteger64i_v");
}

void gd_handle_glGetInteger64v(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetInteger64v");
}

void gd_handle_glGetIntegeri_v(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetIntegeri_v");
}

void gd_handle_glGetIntegerv(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);

    int paramSize = GLRenderer_getParamsv(currentRenderer, pname, GL_INT, NULL);
    char params[paramSize];
    GLRenderer_getParamsv(currentRenderer, pname, GL_INT, params);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_INTEGERV, params, paramSize);
}

void gd_handle_glGetInternalformativ(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum internalformat = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);

    GLint params[count];
    GLFormats_queryInternalformat(parseTexTarget(target), internalformat, pname, count, params);

    gl_send(context->clientRing, REQUEST_CODE_GL_GET_INTERNALFORMATIV, params, count * sizeof(GLint));
}

void gd_handle_glGetLightfv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetLightfv");
}

void gd_handle_glGetLightiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetLightiv");
}

void gd_handle_glGetMapdv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetMapdv");
}

void gd_handle_glGetMapfv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetMapfv");
}

void gd_handle_glGetMapiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetMapiv");
}

void gd_handle_glGetMaterialfv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetMaterialfv");
}

void gd_handle_glGetMaterialiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetMaterialiv");
}

void gd_handle_glGetMultisamplefv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetMultisamplefv");
}

void gd_handle_glGetObjectParameterfvARB(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetObjectParameterfvARB");
}

void gd_handle_glGetObjectParameterivARB(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetObjectParameterivARB");
}

void gd_handle_glGetPixelMapfv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetPixelMapfv");
}

void gd_handle_glGetPixelMapuiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetPixelMapuiv");
}

void gd_handle_glGetPixelMapusv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetPixelMapusv");
}

void gd_handle_glGetPointerv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetPointerv");
}

void gd_handle_glGetPolygonStipple(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetPolygonStipple");
}

void gd_handle_glGetProgramEnvParameterdvARB(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetProgramEnvParameterdvARB");
}

void gd_handle_glGetProgramEnvParameterfvARB(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetProgramEnvParameterfvARB");
}

void gd_handle_glGetProgramInfoLog(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei bufSize = ArrayBuffer_getInt(&context->inputBuffer);

    GLchar infoLog[bufSize];
    GLsizei length = 0;
    glGetProgramInfoLog(program, bufSize, &length, infoLog);

    ArrayBuffer_rewind(&context->outputBuffer);
    ArrayBuffer_putInt(&context->outputBuffer, length);
    ArrayBuffer_putBytes(&context->outputBuffer, infoLog, length);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_PROGRAM_INFO_LOG, context->outputBuffer.buffer, context->outputBuffer.size);
}

void gd_handle_glGetProgramLocalParameterdvARB(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetProgramLocalParameterdvARB");
}

void gd_handle_glGetProgramLocalParameterfvARB(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetProgramLocalParameterfvARB");
}

void gd_handle_glGetProgramStringARB(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetProgramStringARB");
}

void gd_handle_glGetProgramiv(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);

    GLint params;
    ShaderConverter_getProgramiv(program, pname, &params);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_PROGRAMIV, &params, sizeof(GLint));
}

void gd_handle_glGetQueryObjectuiv(GLContext* context) {
    GLuint id = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);

    GLint params = 0;
    GLQuery_getObjectParamsv(id, pname, &params);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_QUERY_OBJECTUIV, &params, sizeof(GLint));
}

void gd_handle_glGetQueryiv(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);

    GLint params = 0;
    GLQuery_getParamsv(target, pname, &params);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_QUERYIV, &params, sizeof(GLint));
}

void gd_handle_glGetRenderbufferParameteriv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetRenderbufferParameteriv");
}

void gd_handle_glGetSamplerParameterfv(GLContext* context) {
    GLuint sampler = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);

    int paramCount = getGLCallParamsCount(pname, NULL);
    GLfloat params[paramCount];
    GLRenderer_getSamplerParameter(currentRenderer, sampler, pname, params);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_SAMPLER_PARAMETERFV, params, paramCount * sizeof(GLfloat));
}

void gd_handle_glGetShaderInfoLog(GLContext* context) {
    GLuint shader = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei bufSize = ArrayBuffer_getInt(&context->inputBuffer);

    GLchar infoLog[bufSize];
    GLsizei length = 0;
    glGetShaderInfoLog(shader, bufSize, &length, infoLog);

    ArrayBuffer_rewind(&context->outputBuffer);
    ArrayBuffer_putInt(&context->outputBuffer, length);
    ArrayBuffer_putBytes(&context->outputBuffer, infoLog, length);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_SHADER_INFO_LOG, context->outputBuffer.buffer, context->outputBuffer.size);
}

void gd_handle_glGetShaderPrecisionFormat(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetShaderPrecisionFormat");
}

void gd_handle_glGetShaderSource(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetShaderSource");
}

void gd_handle_glGetShaderiv(GLContext* context) {
    GLuint shader = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);

    GLint params;
    ShaderConverter_getShaderiv(shader, pname, &params);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_SHADERIV, &params, sizeof(GLint));
}

void gd_handle_glGetString(GLContext* context) {
    GLenum name = ArrayBuffer_getInt(&context->inputBuffer);
    const char* string;

    switch (name) {
        case GL_VERSION:
            string = GL_STRING_VERSION;
            break;
        case GL_EXTENSIONS:
            string = getGLExtensions(NULL);
            break;
        case GL_VENDOR:
            string = (const char*)glGetString(name);
            break;
        case GL_RENDERER:
            string = GL_STRING_RENDERER;
            break;
        case GL_SHADING_LANGUAGE_VERSION:
            string = GL_STRING_SHADING_LANGUAGE_VERSION;
            break;
        default:
            string = "";
            break;
    }

    ArrayBuffer_rewind(&context->outputBuffer);
    ArrayBuffer_putBytes(&context->outputBuffer, string, strlen(string));
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_STRING, context->outputBuffer.buffer, context->outputBuffer.size);
}

void gd_handle_glGetSynciv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetSynciv");
}

void gd_handle_glGetTexEnvfv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetTexEnvfv");
}

void gd_handle_glGetTexEnviv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetTexEnviv");
}

void gd_handle_glGetTexGendv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetTexGendv");
}

void gd_handle_glGetTexGenfv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetTexGenfv");
}

void gd_handle_glGetTexGeniv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetTexGeniv");
}

void gd_handle_glGetTexImage(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum format = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);

    int imageSize;
    void* pixels = GLRenderer_getTexImage(currentRenderer, target, level, format, type, &imageSize);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_TEX_IMAGE, &imageSize, sizeof(int));

    if (imageSize > 0) {
        RingBuffer_write(context->clientRing, pixels, imageSize);
        free(pixels);
    }
}

void gd_handle_glGetTexLevelParameterfv(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);

    GLfloat result;
    GLRenderer_getTexParameter(currentRenderer, target, level, pname, &result);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_TEX_LEVEL_PARAMETERFV, &result, sizeof(GLfloat));
}

void gd_handle_glGetTexParameterfv(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);

    int paramCount = getGLCallParamsCount(pname, NULL);
    GLfloat params[paramCount];
    GLRenderer_getTexParameter(currentRenderer, target, 0, pname, params);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_TEX_PARAMETERFV, params, paramCount * sizeof(GLfloat));
}

void gd_handle_glGetTransformFeedbackVarying(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetTransformFeedbackVarying");
}

void gd_handle_glGetUniformBlockIndex(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);
    GLchar* uniformBlockName = context->inputBuffer.buffer + context->inputBuffer.position;

    GLuint result = glGetUniformBlockIndex(program, uniformBlockName);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_UNIFORM_BLOCK_INDEX, &result, sizeof(GLuint));
}

void gd_handle_glGetUniformIndices(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetUniformIndices");
}

void gd_handle_glGetUniformLocation(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);
    GLchar* name = context->inputBuffer.buffer + context->inputBuffer.position;

    GLint result = glGetUniformLocation(program, name);
    gl_send(context->clientRing, REQUEST_CODE_GL_GET_UNIFORM_LOCATION, &result, sizeof(GLint));
}

void gd_handle_glGetUniformfv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetUniformfv");
}

void gd_handle_glGetUniformiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetUniformiv");
}

void gd_handle_glGetUniformuiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetUniformuiv");
}

void gd_handle_glGetVertexAttribIiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetVertexAttribIiv");
}

void gd_handle_glGetVertexAttribIuiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetVertexAttribIuiv");
}

void gd_handle_glGetVertexAttribPointerv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetVertexAttribPointerv");
}

void gd_handle_glGetVertexAttribdv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetVertexAttribdv");
}

void gd_handle_glGetVertexAttribfv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetVertexAttribfv");
}

void gd_handle_glGetVertexAttribiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glGetVertexAttribiv");
}

void gd_handle_glHint(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);

    if (target ==  GL_FRAGMENT_SHADER_DERIVATIVE_HINT || target ==  GL_GENERATE_MIPMAP_HINT) {
        glHint(target, mode);
    }
}

void gd_handle_glIndexMask(GLContext* context) {
    GLuint mask = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glIndexMask");
}

void gd_handle_glIndexPointer(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glIndexPointer");
}

void gd_handle_glIndexf(GLContext* context) {
    GLfloat c = ArrayBuffer_getFloat(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glIndexf");
}

void gd_handle_glInitNames(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glInitNames");
}

void gd_handle_glInterleavedArrays(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glInterleavedArrays");
}

void gd_handle_glIsBuffer(GLContext* context) {
    GLuint buffer = ArrayBuffer_getInt(&context->inputBuffer);

    GLboolean result = glIsBuffer(buffer);
    gl_send(context->clientRing, REQUEST_CODE_GL_IS_BUFFER, &result, sizeof(GLboolean));
}

void gd_handle_glIsEnabled(GLContext* context) {
    GLenum cap = ArrayBuffer_getInt(&context->inputBuffer);

    GLboolean result;
    GLRenderer_getParamsv(currentRenderer, cap, GL_BOOL, &result);
    gl_send(context->clientRing, REQUEST_CODE_GL_IS_ENABLED, &result, sizeof(GLboolean));
}

void gd_handle_glIsEnabledi(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);

    GLboolean result = glIsEnabledi(target, index);
    gl_send(context->clientRing, REQUEST_CODE_GL_IS_ENABLEDI, &result, sizeof(GLboolean));
}

void gd_handle_glIsFramebuffer(GLContext* context) {
    GLuint framebuffer = ArrayBuffer_getInt(&context->inputBuffer);

    GLboolean result = glIsFramebuffer(framebuffer);
    gl_send(context->clientRing, REQUEST_CODE_GL_IS_FRAMEBUFFER, &result, sizeof(GLboolean));
}

void gd_handle_glIsList(GLContext* context) {
    GLuint list = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glIsList");
}

void gd_handle_glIsProgram(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);

    GLboolean result = glIsProgram(program);
    gl_send(context->clientRing, REQUEST_CODE_GL_IS_PROGRAM, &result, sizeof(GLboolean));
}

void gd_handle_glIsQuery(GLContext* context) {
    GLuint id = ArrayBuffer_getInt(&context->inputBuffer);

    GLboolean result = glIsQuery(id);
    gl_send(context->clientRing, REQUEST_CODE_GL_IS_QUERY, &result, sizeof(GLboolean));
}

void gd_handle_glIsRenderbuffer(GLContext* context) {
    GLuint renderbuffer = ArrayBuffer_getInt(&context->inputBuffer);

    GLboolean result = glIsRenderbuffer(renderbuffer);
    gl_send(context->clientRing, REQUEST_CODE_GL_IS_RENDERBUFFER, &result, sizeof(GLboolean));
}

void gd_handle_glIsSampler(GLContext* context) {
    GLuint sampler = ArrayBuffer_getInt(&context->inputBuffer);

    GLboolean result = glIsSampler(sampler);
    gl_send(context->clientRing, REQUEST_CODE_GL_IS_SAMPLER, &result, sizeof(GLboolean));
}

void gd_handle_glIsShader(GLContext* context) {
    GLuint shader = ArrayBuffer_getInt(&context->inputBuffer);

    GLboolean result = glIsShader(shader);
    gl_send(context->clientRing, REQUEST_CODE_GL_IS_SHADER, &result, sizeof(GLboolean));
}

void gd_handle_glIsSync(GLContext* context) {
    GLsync sync = (GLsync)ArrayBuffer_getLong(&context->inputBuffer);

    GLboolean result = glIsSync(sync);
    gl_send(context->clientRing, REQUEST_CODE_GL_IS_SYNC, &result, sizeof(GLboolean));
}

void gd_handle_glIsTexture(GLContext* context) {
    GLuint texture = ArrayBuffer_getInt(&context->inputBuffer);

    GLboolean result = glIsTexture(texture);
    gl_send(context->clientRing, REQUEST_CODE_GL_IS_TEXTURE, &result, sizeof(GLboolean));
}

void gd_handle_glIsVertexArray(GLContext* context) {
    GLuint array = ArrayBuffer_getInt(&context->inputBuffer);

    GLboolean result = glIsVertexArray(array);
    gl_send(context->clientRing, REQUEST_CODE_GL_IS_VERTEX_ARRAY, &result, sizeof(GLboolean));
}

void gd_handle_glLightModelf(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat param = ArrayBuffer_getFloat(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glLightModelf");
}

void gd_handle_glLightModelfv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glLightModelfv");
}

void gd_handle_glLightModeli(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLint param = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glLightModeli");
}

void gd_handle_glLightModeliv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glLightModeliv");
}

void gd_handle_glLightf(GLContext* context) {
    GLenum light = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat param = ArrayBuffer_getFloat(&context->inputBuffer);

    GLRenderer_setLightParams(currentRenderer, light, pname, &param);
}

void gd_handle_glLightfv(GLContext* context) {
    GLenum light = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat* params = ArrayBuffer_getBytes(&context->inputBuffer, 4 * sizeof(float));

    GLRenderer_setLightParams(currentRenderer, light, pname, params);
}

void gd_handle_glLineStipple(GLContext* context) {
    GLint factor = ArrayBuffer_getInt(&context->inputBuffer);
    GLushort pattern = ArrayBuffer_getShort(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glLineStipple");
}

void gd_handle_glLineWidth(GLContext* context) {
    GLfloat width = ArrayBuffer_getFloat(&context->inputBuffer);

    glLineWidth(width);
}

void gd_handle_glLinkProgram(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderConverter_linkProgram(program);
}

void gd_handle_glListBase(GLContext* context) {
    GLuint base = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glListBase");
}

void gd_handle_glLoadIdentity(GLContext* context) {
    float* matrix = GLRenderer_getCurrentMatrix(currentRenderer);
    mat4_identity(matrix);
}

void gd_handle_glLoadMatrixf(GLContext* context) {
    GLfloat* srcMatrix = ArrayBuffer_getBytes(&context->inputBuffer, 16 * sizeof(GLfloat));

    float* dstMatrix = GLRenderer_getCurrentMatrix(currentRenderer);
    memcpy(dstMatrix, srcMatrix, MAT4_SIZE);
}

void gd_handle_glLoadName(GLContext* context) {
    GLuint name = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glLoadName");
}

void gd_handle_glLoadTransposeMatrixf(GLContext* context) {
    GLfloat* srcMatrix = ArrayBuffer_getBytes(&context->inputBuffer, 16 * sizeof(GLfloat));

    float* dstMatrix = GLRenderer_getCurrentMatrix(currentRenderer);
    mat4_transpose(srcMatrix);
    memcpy(dstMatrix, srcMatrix, MAT4_SIZE);
}

void gd_handle_glLogicOp(GLContext* context) {
    GLenum opcode = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glLogicOp");
}

void gd_handle_glMap1d(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glMap1d");
}

void gd_handle_glMap1f(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glMap1f");
}

void gd_handle_glMap2d(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glMap2d");
}

void gd_handle_glMap2f(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glMap2f");
}

void gd_handle_glMapGrid1d(GLContext* context) {
    GLint un = ArrayBuffer_getInt(&context->inputBuffer);
    GLdouble u1 = ArrayBuffer_getDouble(&context->inputBuffer);
    GLdouble u2 = ArrayBuffer_getDouble(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glMapGrid1d");
}

void gd_handle_glMapGrid1f(GLContext* context) {
    GLint un = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat u1 = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat u2 = ArrayBuffer_getFloat(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glMapGrid1f");
}

void gd_handle_glMapGrid2d(GLContext* context) {
    GLint un = ArrayBuffer_getInt(&context->inputBuffer);
    GLdouble u1 = ArrayBuffer_getDouble(&context->inputBuffer);
    GLdouble u2 = ArrayBuffer_getDouble(&context->inputBuffer);
    GLint vn = ArrayBuffer_getInt(&context->inputBuffer);
    GLdouble v1 = ArrayBuffer_getDouble(&context->inputBuffer);
    GLdouble v2 = ArrayBuffer_getDouble(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glMapGrid2d");
}

void gd_handle_glMapGrid2f(GLContext* context) {
    GLint un = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat u1 = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat u2 = ArrayBuffer_getFloat(&context->inputBuffer);
    GLint vn = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat v1 = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat v2 = ArrayBuffer_getFloat(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glMapGrid2f");
}

void gd_handle_glMaterialf(GLContext* context) {
    GLenum face = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat param = ArrayBuffer_getFloat(&context->inputBuffer);

    GLRenderer_setMaterialParams(currentRenderer, face, pname, &param);
}

void gd_handle_glMaterialfv(GLContext* context) {
    GLenum face = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    int paramCount = getGLCallParamsCount(pname, NULL);
    GLfloat* params = ArrayBuffer_getBytes(&context->inputBuffer, paramCount * sizeof(float));

    GLRenderer_setMaterialParams(currentRenderer, face, pname, params);
}

void gd_handle_glMatrixMode(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);
    currentRenderer->matrixIndex = mode - GL_MODELVIEW;
}

void gd_handle_glMinSampleShading(GLContext* context) {
    GLfloat value = ArrayBuffer_getFloat(&context->inputBuffer);

    glMinSampleShading(value);
}

void gd_handle_glMultMatrixf(GLContext* context) {
    GLfloat* m = ArrayBuffer_getBytes(&context->inputBuffer, 16 * sizeof(GLfloat));

    float* currentMatrix = GLRenderer_getCurrentMatrix(currentRenderer);
    mat4_multiply(currentMatrix, currentMatrix, m);
}

void gd_handle_glMultTransposeMatrixf(GLContext* context) {
    GLfloat* m = ArrayBuffer_getBytes(&context->inputBuffer, 16 * sizeof(GLfloat));

    mat4_transpose(m);
    float* currentMatrix = GLRenderer_getCurrentMatrix(currentRenderer);
    mat4_multiply(currentMatrix, currentMatrix, m);
}

void gd_handle_glMultiDrawElementsBaseVertex(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glMultiDrawElementsBaseVertex");
}

void gd_handle_glMultiTexCoord4f(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat s = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat t = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat r = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat q = ArrayBuffer_getFloat(&context->inputBuffer);

    int index = target - GL_TEXTURE0;
    if (index < MAX_TEXCOORDS) {
        float* texCoord = currentRenderer->state.texCoords[index];
        texCoord[0] = s;
        texCoord[1] = t;
        texCoord[2] = r;
        texCoord[3] = q;
        currentRenderer->geometry.texCoords[index].position++;
    }
}

void gd_handle_glNewList(GLContext* context) {
    GLuint list = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glNewList");
}

void gd_handle_glNormal3f(GLContext* context) {
    GLfloat nx = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat ny = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat nz = ArrayBuffer_getFloat(&context->inputBuffer);

    currentRenderer->state.normal[0] = nx;
    currentRenderer->state.normal[1] = ny;
    currentRenderer->state.normal[2] = nz;

    currentRenderer->geometry.normals.position++;
}

void gd_handle_glNormalPointer(GLContext* context) {
    GL_READ_VERTEX_ARRAY(NORMAL_ARRAY_INDEX);
}

void gd_handle_glOrtho(GLContext* context) {
    GLdouble left = ArrayBuffer_getDouble(&context->inputBuffer);
    GLdouble right = ArrayBuffer_getDouble(&context->inputBuffer);
    GLdouble bottom = ArrayBuffer_getDouble(&context->inputBuffer);
    GLdouble top = ArrayBuffer_getDouble(&context->inputBuffer);
    GLdouble near_val = ArrayBuffer_getDouble(&context->inputBuffer);
    GLdouble far_val = ArrayBuffer_getDouble(&context->inputBuffer);

    float* matrix = GLRenderer_getCurrentMatrix(currentRenderer);
    float tmpMatrix[16];
    mat4_ortho(tmpMatrix, left, right, top, bottom, near_val, far_val);
    mat4_multiply(matrix, matrix, tmpMatrix);
}

void gd_handle_glPassThrough(GLContext* context) {
    GLfloat token = ArrayBuffer_getFloat(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPassThrough");
}

void gd_handle_glPatchParameterfv(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    int paramCount = getGLCallParamsCount(pname, NULL);
    GLfloat* values = ArrayBuffer_getBytes(&context->inputBuffer, paramCount * sizeof(float));

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPatchParameterfv");
}

void gd_handle_glPatchParameteri(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLint value = ArrayBuffer_getInt(&context->inputBuffer);

    glPatchParameteri(pname, value);
}

void gd_handle_glPixelMapfv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPixelMapfv");
}

void gd_handle_glPixelMapuiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPixelMapuiv");
}

void gd_handle_glPixelMapusv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPixelMapusv");
}

void gd_handle_glPixelStorei(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLint param = ArrayBuffer_getInt(&context->inputBuffer);

    if (pname != GL_PACK_ALIGNMENT && pname != GL_UNPACK_ALIGNMENT) glPixelStorei(pname, param);
}

void gd_handle_glPixelTransferf(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat param = ArrayBuffer_getFloat(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPixelTransferf");
}

void gd_handle_glPixelTransferi(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLint param = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPixelTransferi");
}

void gd_handle_glPixelZoom(GLContext* context) {
    GLfloat xfactor = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat yfactor = ArrayBuffer_getFloat(&context->inputBuffer);

    GLRenderer_setPixelZoom(currentRenderer, xfactor, yfactor);
}

void gd_handle_glPointParameterf(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat param = ArrayBuffer_getFloat(&context->inputBuffer);

    GLRenderer_setPointParams(currentRenderer, pname, &param);
}

void gd_handle_glPointParameterfv(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    int paramCount = getGLCallParamsCount(pname, NULL);
    GLfloat* params = ArrayBuffer_getBytes(&context->inputBuffer, paramCount * sizeof(float));

    GLRenderer_setPointParams(currentRenderer, pname, params);
}

void gd_handle_glPointParameteri(GLContext* context) {
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLint param = ArrayBuffer_getInt(&context->inputBuffer);

    GLRenderer_setPointParams(currentRenderer, pname, &param);
}

void gd_handle_glPointParameteriv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPointParameteriv");
}

void gd_handle_glPointSize(GLContext* context) {
    GLfloat size = ArrayBuffer_getFloat(&context->inputBuffer);

    currentRenderer->state.point.size = size;
}

void gd_handle_glPolygonMode(GLContext* context) {
    GLenum face = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);

    currentRenderer->state.polygonMode = mode;
}

void gd_handle_glPolygonOffset(GLContext* context) {
    GLfloat factor = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat units = ArrayBuffer_getFloat(&context->inputBuffer);

    glPolygonOffset(factor, units);
}

void gd_handle_glPolygonStipple(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPolygonStipple");
}

void gd_handle_glPopAttrib(GLContext* context) {
    AttribStack_pop();
}

void gd_handle_glPopClientAttrib(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPopClientAttrib");
}

void gd_handle_glPopMatrix(GLContext* context) {
    GLRenderer_popMatrix(currentRenderer);
}

void gd_handle_glPopName(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPopName");
}

void gd_handle_glPrioritizeTextures(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPrioritizeTextures");
}

void gd_handle_glProgramEnvParameter4fARB(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat x = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat y = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat z = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat w = ArrayBuffer_getFloat(&context->inputBuffer);

    ARBProgram_setEnvParameter(target, index, x, y, z, w);
}

void gd_handle_glProgramLocalParameter4fARB(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat x = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat y = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat z = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat w = ArrayBuffer_getFloat(&context->inputBuffer);

    ARBProgram* program = ARBProgram_getBound(target);
    ARBProgram_setLocalParameter(program, index, x, y, z, w);
}

void gd_handle_glProgramStringARB(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum format = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei len = ArrayBuffer_getInt(&context->inputBuffer);
    char* string = context->inputBuffer.buffer + context->inputBuffer.position;

    ARBProgram* program = ARBProgram_getBound(target);
    ARBProgram_setSource(program, format, string, len);
}

void gd_handle_glProvokingVertex(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glProvokingVertex");
}

void gd_handle_glPushAttrib(GLContext* context) {
    GLbitfield mask = ArrayBuffer_getInt(&context->inputBuffer);

    AttribStack_push(mask);
}

void gd_handle_glPushClientAttrib(GLContext* context) {
    GLbitfield mask = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPushClientAttrib");
}

void gd_handle_glPushMatrix(GLContext* context) {
    GLRenderer_pushMatrix(currentRenderer);
}

void gd_handle_glPushName(GLContext* context) {
    GLuint name = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glPushName");
}

void gd_handle_glQueryCounter(GLContext* context) {
    GLuint id = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);

    GLQuery_queryCounter(id, target);
}

void gd_handle_glRasterPos4f(GLContext* context) {
    GLfloat x = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat y = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat z = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat w = ArrayBuffer_getFloat(&context->inputBuffer);

    GLRenderer_setRasterPos(currentRenderer, true, x, y, z, w);
}

void gd_handle_glReadBuffer(GLContext* context) {
    GLenum src = ArrayBuffer_getInt(&context->inputBuffer);

    GLFramebuffer_setReadBuffer(src);
}

void gd_handle_glReadPixels(GLContext* context) {
    GLint x = ArrayBuffer_getInt(&context->inputBuffer);
    GLint y = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum format = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);

    GLBuffer* pixelPackBuffer = GLBuffer_getBound(GL_PIXEL_PACK_BUFFER);
    if (pixelPackBuffer) {
        uint64_t pointer = ArrayBuffer_getInt(&context->inputBuffer);
        GLRenderer_readPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (void*)pointer);
    }
    else {
        int imageSize = computeTexImageDataSize(format, type, width, height, 1);
        gl_send(context->clientRing, REQUEST_CODE_GL_READ_PIXELS, &imageSize, sizeof(int));

        if (imageSize > 0) {
            RING_WRITE_BEGIN(context->clientRing, imageSize);
            GLRenderer_readPixels(x, y, width, height, format, type, ringData);
            RING_WRITE_END(context->clientRing);
        }
    }
}

void gd_handle_glReleaseShaderCompiler(GLContext* context) {
    glReleaseShaderCompiler();
}

void gd_handle_glRenderMode(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glRenderMode");
}

void gd_handle_glRenderbufferStorage(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum internalformat = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);

    if (internalformat == GL_DEPTH_COMPONENT) internalformat = GL_DEPTH_COMPONENT24;
    glRenderbufferStorage(target, internalformat, width, height);
}

void gd_handle_glRenderbufferStorageMultisample(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei samples = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum internalformat = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);

    if (internalformat == GL_DEPTH_COMPONENT) internalformat = GL_DEPTH_COMPONENT24;
    glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

void gd_handle_glRotatef(GLContext* context) {
    GLfloat angle = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat x = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat y = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat z = ArrayBuffer_getFloat(&context->inputBuffer);

    float* matrix = GLRenderer_getCurrentMatrix(currentRenderer);
    mat4_rotate(matrix, matrix, TO_RADIANS(angle), x, y, z);
}

void gd_handle_glSampleCoverage(GLContext* context) {
    GLfloat value = ArrayBuffer_getFloat(&context->inputBuffer);
    GLboolean invert = ArrayBuffer_get(&context->inputBuffer);

    glSampleCoverage(value, invert);
}

void gd_handle_glSampleMaski(GLContext* context) {
    GLuint maskNumber = ArrayBuffer_getInt(&context->inputBuffer);
    GLbitfield mask = ArrayBuffer_getInt(&context->inputBuffer);

    glSampleMaski(maskNumber, mask);
}

void gd_handle_glSamplerParameterf(GLContext* context) {
    GLuint sampler = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat param = ArrayBuffer_getFloat(&context->inputBuffer);

    GLRenderer_setSamplerParameter(currentRenderer, sampler, pname, &param);
}

void gd_handle_glSamplerParameterfv(GLContext* context) {
    GLuint sampler = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    int paramCount = getGLCallParamsCount(pname, NULL);
    GLfloat* param = ArrayBuffer_getBytes(&context->inputBuffer, paramCount * sizeof(GLfloat));

    GLRenderer_setSamplerParameter(currentRenderer, sampler, pname, param);
}

void gd_handle_glScalef(GLContext* context) {
    GLfloat x = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat y = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat z = ArrayBuffer_getFloat(&context->inputBuffer);

    float* matrix = GLRenderer_getCurrentMatrix(currentRenderer);
    mat4_scale(matrix, matrix, x, y, z);
}

void gd_handle_glScissor(GLContext* context) {
    GLint x = ArrayBuffer_getInt(&context->inputBuffer);
    GLint y = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);

    glScissor(x, y, width, height);
}

void gd_handle_glSecondaryColor3f(GLContext* context) {
    GLfloat red = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat green = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat blue = ArrayBuffer_getFloat(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glSecondaryColor3f");
}

void gd_handle_glSecondaryColorPointer(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glSecondaryColorPointer");
}

void gd_handle_glSelectBuffer(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glSelectBuffer");
}

void gd_handle_glShadeModel(GLContext* context) {
    GLenum mode = ArrayBuffer_getInt(&context->inputBuffer);

    currentRenderer->state.shadeModel = mode;
}

void gd_handle_glShaderBinary(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glShaderBinary");
}

void gd_handle_glShaderSource(GLContext* context) {
    GLuint shader = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);

    ShaderConverter_setShaderSource(shader, count, &context->inputBuffer);
}

void gd_handle_glStencilFunc(GLContext* context) {
    GLenum func = ArrayBuffer_getInt(&context->inputBuffer);
    GLint ref = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint mask = ArrayBuffer_getInt(&context->inputBuffer);

    glStencilFunc(func, ref, mask);
}

void gd_handle_glStencilFuncSeparate(GLContext* context) {
    GLenum face = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum func = ArrayBuffer_getInt(&context->inputBuffer);
    GLint ref = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint mask = ArrayBuffer_getInt(&context->inputBuffer);

    glStencilFuncSeparate(face, func, ref, mask);
}

void gd_handle_glStencilMask(GLContext* context) {
    GLuint mask = ArrayBuffer_getInt(&context->inputBuffer);

    glStencilMask(mask);
}

void gd_handle_glStencilMaskSeparate(GLContext* context) {
    GLenum face = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint mask = ArrayBuffer_getInt(&context->inputBuffer);

    glStencilMaskSeparate(face, mask);
}

void gd_handle_glStencilOp(GLContext* context) {
    GLenum fail = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum zfail = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum zpass = ArrayBuffer_getInt(&context->inputBuffer);

    glStencilOp(fail, zfail, zpass);
}

void gd_handle_glStencilOpSeparate(GLContext* context) {
    GLenum face = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum sfail = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum dpfail = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum dppass = ArrayBuffer_getInt(&context->inputBuffer);

    glStencilOpSeparate(face, sfail, dpfail, dppass);
}

void gd_handle_glTexBuffer(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum internalformat = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint buffer = ArrayBuffer_getInt(&context->inputBuffer);

    glTexBuffer(target, internalformat, buffer);
}

void gd_handle_glTexCoord4f(GLContext* context) {
    GLfloat s = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat t = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat r = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat q = ArrayBuffer_getFloat(&context->inputBuffer);

    float* texCoord = currentRenderer->state.texCoords[0];
    texCoord[0] = s;
    texCoord[1] = t;
    texCoord[2] = r;
    texCoord[3] = q;
    currentRenderer->geometry.texCoords[0].position++;
}

void gd_handle_glTexCoordPointer(GLContext* context) {
    GL_READ_VERTEX_ARRAY(TEXCOORD_ARRAY_INDEX + currentRenderer->clientState.activeTexture);
}

void gd_handle_glTexEnvf(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat param = ArrayBuffer_getFloat(&context->inputBuffer);

    GLRenderer_setTexEnvParams(currentRenderer, target, pname, &param);
}

void gd_handle_glTexEnvfv(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    int paramCount = getGLCallParamsCount(pname, NULL);
    GLfloat* params = ArrayBuffer_getBytes(&context->inputBuffer, paramCount * sizeof(float));

    GLRenderer_setTexEnvParams(currentRenderer, target, pname, params);
}

void gd_handle_glTexGend(GLContext* context) {
    GLenum coord = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLdouble param = ArrayBuffer_getDouble(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glTexGend");
}

void gd_handle_glTexGendv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glTexGendv");
}

void gd_handle_glTexGenf(GLContext* context) {
    GLenum coord = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat param = ArrayBuffer_getFloat(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glTexGenf");
}

void gd_handle_glTexGenfv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glTexGenfv");
}

void gd_handle_glTexGeni(GLContext* context) {
    GLenum coord = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLint param = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glTexGeni");
}

void gd_handle_glTexGeniv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glTexGeniv");
}

void gd_handle_glTexImage2D(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);
    GLint internalformat = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);
    GLint border = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum format = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);
    GLint imageSize = ArrayBuffer_getInt(&context->inputBuffer);

    target = parseTexTarget(target);
    int originFormat = internalformat;
    GLBuffer* pixelUnpackBuffer = GLBuffer_getBound(GL_PIXEL_UNPACK_BUFFER);
    if (pixelUnpackBuffer) {
        uint64_t pointer = ArrayBuffer_getInt(&context->inputBuffer);
        convertTexImageFormat(target, &internalformat, &format, &type, NULL, 0);
        glTexImage2D(target, level, internalformat, width, height, border, format, type, pixelUnpackBuffer->mappedData + pointer);
        gl_send(context->clientRing, REQUEST_CODE_GL_TEX_IMAGE2D, NULL, 0);
    }
    else {
        void* imageData = NULL;
        RING_READ_BEGIN(context->serverRing, imageData, imageSize);
        convertTexImageFormat(target, &internalformat, &format, &type, &imageData, imageSize);
        glTexImage2D(target, level, internalformat, width, height, border, format, type, imageData);
        RING_READ_END(context->serverRing);
    }

    GLTexture* texture = GLTexture_getBound(target);
    if (texture && level == 0) {
        texture->width = width;
        texture->height = height;
        texture->originFormat = originFormat;
        if (texture->generateMipmap) glGenerateMipmap(target);
    }
}

void gd_handle_glTexImage2DMultisample(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei samples = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum internalformat = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);
    GLboolean fixedsamplelocations = ArrayBuffer_get(&context->inputBuffer);

    glTexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

void gd_handle_glTexImage3D(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);
    GLint internalformat = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei depth = ArrayBuffer_getInt(&context->inputBuffer);
    GLint border = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum format = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);
    GLint imageSize = ArrayBuffer_getInt(&context->inputBuffer);

    target = parseTexTarget(target);
    GLBuffer* pixelUnpackBuffer = GLBuffer_getBound(GL_PIXEL_UNPACK_BUFFER);
    if (pixelUnpackBuffer) {
        uint64_t pointer = ArrayBuffer_getInt(&context->inputBuffer);
        convertTexImageFormat(target, &internalformat, &format, &type, NULL, 0);
        glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixelUnpackBuffer->mappedData + pointer);
        gl_send(context->clientRing, REQUEST_CODE_GL_TEX_IMAGE3D, NULL, 0);
    }
    else {
        void* imageData = NULL;
        RING_READ_BEGIN(context->serverRing, imageData, imageSize);
        convertTexImageFormat(target, &internalformat, &format, &type, &imageData, imageSize);
        glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, imageData);
        RING_READ_END(context->serverRing);
    }
}

void gd_handle_glTexImage3DMultisample(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei samples = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum internalformat = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei depth = ArrayBuffer_getInt(&context->inputBuffer);
    GLboolean fixedsamplelocations = ArrayBuffer_get(&context->inputBuffer);

    glTexStorage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

void gd_handle_glTexParameterf(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat param = ArrayBuffer_getFloat(&context->inputBuffer);

    GLRenderer_setTexParameter(currentRenderer, target, pname, &param);
}

void gd_handle_glTexParameterfv(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum pname = ArrayBuffer_getInt(&context->inputBuffer);
    int paramCount = getGLCallParamsCount(pname, NULL);
    GLfloat* params = ArrayBuffer_getBytes(&context->inputBuffer, paramCount * sizeof(GLfloat));

    GLRenderer_setTexParameter(currentRenderer, target, pname, params);
}

void gd_handle_glTexSubImage2D(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);
    GLint xoffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLint yoffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum format = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);
    GLint imageSize = ArrayBuffer_getInt(&context->inputBuffer);

    target = parseTexTarget(target);
    GLBuffer* pixelUnpackBuffer = GLBuffer_getBound(GL_PIXEL_UNPACK_BUFFER);
    if (pixelUnpackBuffer) {
        uint64_t pointer = ArrayBuffer_getInt(&context->inputBuffer);
        convertTexImageFormat(target, NULL, &format, &type, NULL, 0);
        glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixelUnpackBuffer->mappedData + pointer);
        gl_send(context->clientRing, REQUEST_CODE_GL_TEX_SUB_IMAGE2D, NULL, 0);
    }
    else {
        void* imageData = NULL;
        RING_READ_BEGIN(context->serverRing, imageData, imageSize);
        convertTexImageFormat(target, NULL, &format, &type, &imageData, imageSize);
        glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, imageData);
        RING_READ_END(context->serverRing);
    }
}

void gd_handle_glTexSubImage3D(GLContext* context) {
    GLenum target = ArrayBuffer_getInt(&context->inputBuffer);
    GLint level = ArrayBuffer_getInt(&context->inputBuffer);
    GLint xoffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLint yoffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLint zoffset = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei depth = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum format = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);
    GLint imageSize = ArrayBuffer_getInt(&context->inputBuffer);

    target = parseTexTarget(target);
    GLBuffer* pixelUnpackBuffer = GLBuffer_getBound(GL_PIXEL_UNPACK_BUFFER);
    if (pixelUnpackBuffer) {
        uint64_t pointer = ArrayBuffer_getInt(&context->inputBuffer);
        convertTexImageFormat(target, NULL, &format, &type, NULL, 0);
        glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixelUnpackBuffer->mappedData + pointer);
        gl_send(context->clientRing, REQUEST_CODE_GL_TEX_SUB_IMAGE3D, NULL, 0);
    }
    else {
        void* imageData = NULL;
        RING_READ_BEGIN(context->serverRing, imageData, imageSize);
        convertTexImageFormat(target, NULL, &format, &type, &imageData, imageSize);
        glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, imageData);
        RING_READ_END(context->serverRing);
    }
}

void gd_handle_glTransformFeedbackVaryings(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glTransformFeedbackVaryings");
}

void gd_handle_glTranslatef(GLContext* context) {
    GLfloat x = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat y = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat z = ArrayBuffer_getFloat(&context->inputBuffer);

    float* matrix = GLRenderer_getCurrentMatrix(currentRenderer);
    mat4_translate(matrix, matrix, x, y, z);
}

void gd_handle_glUniform1f(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat v0 = ArrayBuffer_getFloat(&context->inputBuffer);

    glUniform1f(location, v0);
}

void gd_handle_glUniform1fv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat* value = ArrayBuffer_getBytes(&context->inputBuffer, count * sizeof(GLfloat));

    glUniform1fv(location, count, value);
}

void gd_handle_glUniform1i(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLint v0 = ArrayBuffer_getInt(&context->inputBuffer);

    glUniform1i(location, v0);
}

void gd_handle_glUniform1iv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLint* value = ArrayBuffer_getBytes(&context->inputBuffer, count * sizeof(GLint));

    glUniform1iv(location, count, value);
}

void gd_handle_glUniform1ui(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint v0 = ArrayBuffer_getInt(&context->inputBuffer);

    glUniform1ui(location, v0);
}

void gd_handle_glUniform1uiv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint* value = ArrayBuffer_getBytes(&context->inputBuffer, count * sizeof(GLuint));

    glUniform1uiv(location, count, value);
}

void gd_handle_glUniform2f(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat v0 = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat v1 = ArrayBuffer_getFloat(&context->inputBuffer);

    glUniform2f(location, v0, v1);
}

void gd_handle_glUniform2fv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 2 * sizeof(GLfloat));

    glUniform2fv(location, count, value);
}

void gd_handle_glUniform2i(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLint v0 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint v1 = ArrayBuffer_getInt(&context->inputBuffer);

    glUniform2i(location, v0, v1);
}

void gd_handle_glUniform2iv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLint* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 2 * sizeof(GLint));

    glUniform2iv(location, count, value);
}

void gd_handle_glUniform2ui(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint v0 = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint v1 = ArrayBuffer_getInt(&context->inputBuffer);

    glUniform2ui(location, v0, v1);
}

void gd_handle_glUniform2uiv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 2 * sizeof(GLuint));

    glUniform2uiv(location, count, value);
}

void gd_handle_glUniform3f(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat v0 = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat v1 = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat v2 = ArrayBuffer_getFloat(&context->inputBuffer);

    glUniform3f(location, v0, v1, v2);
}

void gd_handle_glUniform3fv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 3 * sizeof(GLfloat));

    glUniform3fv(location, count, value);
}

void gd_handle_glUniform3i(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLint v0 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint v1 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint v2 = ArrayBuffer_getInt(&context->inputBuffer);

    glUniform3i(location, v0, v1, v2);
}

void gd_handle_glUniform3iv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLint* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 3 * sizeof(GLint));

    glUniform3iv(location, count, value);
}

void gd_handle_glUniform3ui(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint v0 = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint v1 = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint v2 = ArrayBuffer_getInt(&context->inputBuffer);

    glUniform3ui(location, v0, v1, v2);
}

void gd_handle_glUniform3uiv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 3 * sizeof(GLuint));

    glUniform3uiv(location, count, value);
}

void gd_handle_glUniform4f(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat v0 = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat v1 = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat v2 = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat v3 = ArrayBuffer_getFloat(&context->inputBuffer);

    glUniform4f(location, v0, v1, v2, v3);
}

void gd_handle_glUniform4fv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 4 * sizeof(GLfloat));

    glUniform4fv(location, count, value);
}

void gd_handle_glUniform4i(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLint v0 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint v1 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint v2 = ArrayBuffer_getInt(&context->inputBuffer);
    GLint v3 = ArrayBuffer_getInt(&context->inputBuffer);

    glUniform4i(location, v0, v1, v2, v3);
}

void gd_handle_glUniform4iv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLint* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 4 * sizeof(GLint));

    glUniform4iv(location, count, value);
}

void gd_handle_glUniform4ui(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint v0 = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint v1 = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint v2 = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint v3 = ArrayBuffer_getInt(&context->inputBuffer);

    glUniform4ui(location, v0, v1, v2, v3);
}

void gd_handle_glUniform4uiv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 4 * sizeof(GLuint));

    glUniform4uiv(location, count, value);
}

void gd_handle_glUniformBlockBinding(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint uniformBlockIndex = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint uniformBlockBinding = ArrayBuffer_getInt(&context->inputBuffer);

    glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
}

void gd_handle_glUniformMatrix2fv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLboolean transpose = ArrayBuffer_get(&context->inputBuffer);
    GLfloat* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 4 * sizeof(GLfloat));

    glUniformMatrix2fv(location, count, transpose, value);
}

void gd_handle_glUniformMatrix2x3fv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLboolean transpose = ArrayBuffer_get(&context->inputBuffer);
    GLfloat* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 6 * sizeof(GLfloat));

    glUniformMatrix2x3fv(location, count, transpose, value);
}

void gd_handle_glUniformMatrix2x4fv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLboolean transpose = ArrayBuffer_get(&context->inputBuffer);
    GLfloat* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 8 * sizeof(GLfloat));

    glUniformMatrix2x4fv(location, count, transpose, value);
}

void gd_handle_glUniformMatrix3fv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLboolean transpose = ArrayBuffer_get(&context->inputBuffer);
    GLfloat* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 9 * sizeof(GLfloat));

    glUniformMatrix3fv(location, count, transpose, value);
}

void gd_handle_glUniformMatrix3x2fv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLboolean transpose = ArrayBuffer_get(&context->inputBuffer);
    GLfloat* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 6 * sizeof(GLfloat));

    glUniformMatrix3x2fv(location, count, transpose, value);
}

void gd_handle_glUniformMatrix3x4fv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLboolean transpose = ArrayBuffer_get(&context->inputBuffer);
    GLfloat* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 12 * sizeof(GLfloat));

    glUniformMatrix3x4fv(location, count, transpose, value);
}

void gd_handle_glUniformMatrix4fv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLboolean transpose = ArrayBuffer_get(&context->inputBuffer);
    GLfloat* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 16 * sizeof(GLfloat));

    glUniformMatrix4fv(location, count, transpose, value);
}

void gd_handle_glUniformMatrix4x2fv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLboolean transpose = ArrayBuffer_get(&context->inputBuffer);
    GLfloat* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 8 * sizeof(GLfloat));

    glUniformMatrix4x2fv(location, count, transpose, value);
}

void gd_handle_glUniformMatrix4x3fv(GLContext* context) {
    GLint location = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei count = ArrayBuffer_getInt(&context->inputBuffer);
    GLboolean transpose = ArrayBuffer_get(&context->inputBuffer);
    GLfloat* value = ArrayBuffer_getBytes(&context->inputBuffer, count * 12 * sizeof(GLfloat));

    glUniformMatrix4x3fv(location, count, transpose, value);
}

void gd_handle_glUseProgram(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);

    currentRenderer->clientState.program = ShaderConverter_getProgram(program);
    glUseProgram(program);
}

void gd_handle_glValidateProgram(GLContext* context) {
    GLuint program = ArrayBuffer_getInt(&context->inputBuffer);

    glValidateProgram(program);
}

void gd_handle_glVertex4f(GLContext* context) {
    GLfloat x = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat y = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat z = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat w = ArrayBuffer_getFloat(&context->inputBuffer);

    GLRenderer_addVertex(currentRenderer, x, y, z, w);
}

void gd_handle_glVertexAttrib1f(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat x = ArrayBuffer_getFloat(&context->inputBuffer);

    glVertexAttrib1f(index, x);
}

void gd_handle_glVertexAttrib2f(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat x = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat y = ArrayBuffer_getFloat(&context->inputBuffer);

    glVertexAttrib2f(index, x, y);
}

void gd_handle_glVertexAttrib3f(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat x = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat y = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat z = ArrayBuffer_getFloat(&context->inputBuffer);

    glVertexAttrib3f(index, x, y, z);
}

void gd_handle_glVertexAttrib4Nbv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttrib4Nbv");
}

void gd_handle_glVertexAttrib4Niv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttrib4Niv");
}

void gd_handle_glVertexAttrib4Nsv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttrib4Nsv");
}

void gd_handle_glVertexAttrib4Nub(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLubyte x = ArrayBuffer_get(&context->inputBuffer);
    GLubyte y = ArrayBuffer_get(&context->inputBuffer);
    GLubyte z = ArrayBuffer_get(&context->inputBuffer);
    GLubyte w = ArrayBuffer_get(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttrib4Nub");
}

void gd_handle_glVertexAttrib4Nubv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttrib4Nubv");
}

void gd_handle_glVertexAttrib4Nuiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttrib4Nuiv");
}

void gd_handle_glVertexAttrib4Nusv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttrib4Nusv");
}

void gd_handle_glVertexAttrib4f(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLfloat x = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat y = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat z = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat w = ArrayBuffer_getFloat(&context->inputBuffer);

    glVertexAttrib4f(index, x, y, z, w);
}

void gd_handle_glVertexAttribDivisor(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint divisor = ArrayBuffer_getInt(&context->inputBuffer);

    glVertexAttribDivisor(index, divisor);
}

void gd_handle_glVertexAttribI1i(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLint x = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI1i");
}

void gd_handle_glVertexAttribI1iv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI1iv");
}

void gd_handle_glVertexAttribI1ui(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint x = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI1ui");
}

void gd_handle_glVertexAttribI1uiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI1uiv");
}

void gd_handle_glVertexAttribI2i(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLint x = ArrayBuffer_getInt(&context->inputBuffer);
    GLint y = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI2i");
}

void gd_handle_glVertexAttribI2iv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI2iv");
}

void gd_handle_glVertexAttribI2ui(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint x = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint y = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI2ui");
}

void gd_handle_glVertexAttribI2uiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI2uiv");
}

void gd_handle_glVertexAttribI3i(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLint x = ArrayBuffer_getInt(&context->inputBuffer);
    GLint y = ArrayBuffer_getInt(&context->inputBuffer);
    GLint z = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI3i");
}

void gd_handle_glVertexAttribI3iv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI3iv");
}

void gd_handle_glVertexAttribI3ui(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint x = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint y = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint z = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI3ui");
}

void gd_handle_glVertexAttribI3uiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI3uiv");
}

void gd_handle_glVertexAttribI4bv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI4bv");
}

void gd_handle_glVertexAttribI4i(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLint x = ArrayBuffer_getInt(&context->inputBuffer);
    GLint y = ArrayBuffer_getInt(&context->inputBuffer);
    GLint z = ArrayBuffer_getInt(&context->inputBuffer);
    GLint w = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI4i");
}

void gd_handle_glVertexAttribI4iv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI4iv");
}

void gd_handle_glVertexAttribI4sv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI4sv");
}

void gd_handle_glVertexAttribI4ubv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI4ubv");
}

void gd_handle_glVertexAttribI4ui(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint x = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint y = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint z = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint w = ArrayBuffer_getInt(&context->inputBuffer);

    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI4ui");
}

void gd_handle_glVertexAttribI4uiv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI4uiv");
}

void gd_handle_glVertexAttribI4usv(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribI4usv");
}

void gd_handle_glVertexAttribIPointer(GLContext* context) {
    println(MSG_DEBUG_UNIMPLEMENTED_FUNC, "glVertexAttribIPointer");
}

void gd_handle_glVertexAttribPointer(GLContext* context) {
    GLuint index = ArrayBuffer_getInt(&context->inputBuffer);
    GLint size = ArrayBuffer_getInt(&context->inputBuffer);
    GLenum type = ArrayBuffer_getInt(&context->inputBuffer);
    GLboolean normalized = ArrayBuffer_get(&context->inputBuffer);
    GLsizei stride = ArrayBuffer_getInt(&context->inputBuffer);
    uint64_t pointer = ArrayBuffer_getInt(&context->inputBuffer);

    GLClientState* clientState = &currentRenderer->clientState;
    if (index < VERTEX_ATTRIB_COUNT) {
        GLVertexAttrib* vertexAttrib = &clientState->vao->attribs[index];
        if (!GLBuffer_getBound(GL_ARRAY_BUFFER) || size == GL_BGRA) {
            vertexAttrib->type = type;
            vertexAttrib->size = size;
            vertexAttrib->normalized = normalized;
            vertexAttrib->stride = stride;
            vertexAttrib->pointer = size == GL_BGRA ? (void*)pointer : NULL;
            vertexAttrib->boundArrayBuffer = 0;
            GLVertexArrayObject_setAttribState(clientState, index, VERTEX_ATTRIB_ENABLED, false);
            return;
        }
        else GLVertexArrayObject_setAttribState(clientState, index, VERTEX_ATTRIB_DISABLED, true);
    }
    else if (index >= INT32_MAX) {
        int arrayIdx = index - INT32_MAX;
        ShaderProgram* program = currentRenderer->clientState.program;
        index = program->location.attributes[arrayIdx];
        GLRenderer_enableVertexAttribute(currentRenderer, index);

        GLVertexArrayObject_setAttribState(clientState, arrayIdx, VERTEX_ATTRIB_DISABLED, true);
        GLVertexAttrib* vertexAttrib = &clientState->vao->attribs[arrayIdx];
        vertexAttrib->type = type;
        vertexAttrib->size = size;
        vertexAttrib->normalized = normalized;
        vertexAttrib->stride = stride;
        vertexAttrib->pointer = (void*)pointer;
        vertexAttrib->boundArrayBuffer = GLBuffer_getBound(GL_ARRAY_BUFFER)->id;
    }

    glVertexAttribPointer(index, size, type, normalized, stride, (const void*)pointer);
}

void gd_handle_glVertexPointer(GLContext* context) {
    GL_READ_VERTEX_ARRAY(POSITION_ARRAY_INDEX);
}

void gd_handle_glViewport(GLContext* context) {
    GLint x = ArrayBuffer_getInt(&context->inputBuffer);
    GLint y = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei width = ArrayBuffer_getInt(&context->inputBuffer);
    GLsizei height = ArrayBuffer_getInt(&context->inputBuffer);

    glViewport(x, y, width, height);
}

void gd_handle_glWaitSync(GLContext* context) {
    GLsync sync = (GLsync)ArrayBuffer_getLong(&context->inputBuffer);
    GLbitfield flags = ArrayBuffer_getInt(&context->inputBuffer);
    GLuint64 timeout = ArrayBuffer_getLong(&context->inputBuffer);

    glWaitSync(sync, flags, timeout);
}

void gd_handle_glWindowPos3f(GLContext* context) {
    GLfloat x = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat y = ArrayBuffer_getFloat(&context->inputBuffer);
    GLfloat z = ArrayBuffer_getFloat(&context->inputBuffer);

    GLRenderer_setRasterPos(currentRenderer, false, x, y, z, 1.0f);
}

HandleRequestFunc handleRequestFuncs[] = {
    gd_handle_glAccum,
    gd_handle_glActiveTexture,
    gd_handle_glAlphaFunc,
    gd_handle_glArrayElement,
    gd_handle_glAttachShader,
    gd_handle_glBegin,
    gd_handle_glBeginConditionalRender,
    gd_handle_glBeginQuery,
    gd_handle_glBeginTransformFeedback,
    gd_handle_glBindAttribLocation,
    gd_handle_glBindBuffer,
    gd_handle_glBindBufferBase,
    gd_handle_glBindBufferRange,
    gd_handle_glBindFragDataLocation,
    gd_handle_glBindFragDataLocationIndexed,
    gd_handle_glBindFramebuffer,
    gd_handle_glBindProgramARB,
    gd_handle_glBindRenderbuffer,
    gd_handle_glBindSampler,
    gd_handle_glBindTexture,
    gd_handle_glBindVertexArray,
    gd_handle_glBitmap,
    gd_handle_glBlendColor,
    gd_handle_glBlendEquation,
    gd_handle_glBlendEquationSeparate,
    gd_handle_glBlendEquationSeparatei,
    gd_handle_glBlendEquationi,
    gd_handle_glBlendFunc,
    gd_handle_glBlendFuncSeparate,
    gd_handle_glBlendFuncSeparatei,
    gd_handle_glBlendFunci,
    gd_handle_glBlitFramebuffer,
    gd_handle_glBufferData,
    gd_handle_glBufferStorage,
    gd_handle_glBufferSubData,
    gd_handle_glCallList,
    gd_handle_glCallLists,
    gd_handle_glCheckFramebufferStatus,
    gd_handle_glClampColor,
    gd_handle_glClear,
    gd_handle_glClearAccum,
    gd_handle_glClearBufferfi,
    gd_handle_glClearBufferfv,
    gd_handle_glClearBufferiv,
    gd_handle_glClearBufferuiv,
    gd_handle_glClearColor,
    gd_handle_glClearDepthf,
    gd_handle_glClearIndex,
    gd_handle_glClearStencil,
    gd_handle_glClientActiveTexture,
    gd_handle_glClientWaitSync,
    gd_handle_glClipPlane,
    gd_handle_glColor4f,
    gd_handle_glColorMask,
    gd_handle_glColorMaski,
    gd_handle_glColorMaterial,
    gd_handle_glColorPointer,
    gd_handle_glCompileShader,
    gd_handle_glCompressedTexImage2D,
    gd_handle_glCompressedTexSubImage2D,
    gd_handle_glCopyBufferSubData,
    gd_handle_glCopyPixels,
    gd_handle_glCopyTexImage2D,
    gd_handle_glCopyTexSubImage2D,
    gd_handle_glCopyTexSubImage3D,
    gd_handle_glCreateProgram,
    gd_handle_glCreateShader,
    gd_handle_glCullFace,
    gd_handle_glDeleteBuffers,
    gd_handle_glDeleteFramebuffers,
    gd_handle_glDeleteLists,
    gd_handle_glDeleteObjectARB,
    gd_handle_glDeleteProgram,
    gd_handle_glDeleteProgramsARB,
    gd_handle_glDeleteQueries,
    gd_handle_glDeleteRenderbuffers,
    gd_handle_glDeleteSamplers,
    gd_handle_glDeleteShader,
    gd_handle_glDeleteSync,
    gd_handle_glDeleteTextures,
    gd_handle_glDeleteVertexArrays,
    gd_handle_glDepthFunc,
    gd_handle_glDepthMask,
    gd_handle_glDepthRangef,
    gd_handle_glDetachShader,
    gd_handle_glDisable,
    gd_handle_glDisableClientState,
    gd_handle_glDisableVertexAttribArray,
    gd_handle_glDisablei,
    gd_handle_glDispatchCompute,
    gd_handle_glDispatchComputeIndirect,
    gd_handle_glDrawArrays,
    gd_handle_glDrawArraysInstanced,
    gd_handle_glDrawBuffer,
    gd_handle_glDrawBuffers,
    gd_handle_glDrawElements,
    gd_handle_glDrawElementsBaseVertex,
    gd_handle_glDrawElementsInstanced,
    gd_handle_glDrawElementsInstancedBaseVertex,
    gd_handle_glDrawPixels,
    gd_handle_glDrawRangeElements,
    gd_handle_glDrawRangeElementsBaseVertex,
    gd_handle_glEdgeFlag,
    gd_handle_glEdgeFlagPointer,
    gd_handle_glEdgeFlagv,
    gd_handle_glEnable,
    gd_handle_glEnableClientState,
    gd_handle_glEnableVertexAttribArray,
    gd_handle_glEnablei,
    gd_handle_glEnd,
    gd_handle_glEndConditionalRender,
    gd_handle_glEndList,
    gd_handle_glEndQuery,
    gd_handle_glEndTransformFeedback,
    gd_handle_glEvalCoord2f,
    gd_handle_glEvalMesh1,
    gd_handle_glEvalMesh2,
    gd_handle_glEvalPoint1,
    gd_handle_glEvalPoint2,
    gd_handle_glFeedbackBuffer,
    gd_handle_glFenceSync,
    gd_handle_glFinish,
    gd_handle_glFlush,
    gd_handle_glFlushMappedBufferRange,
    gd_handle_glFogCoordPointer,
    gd_handle_glFogCoordf,
    gd_handle_glFogf,
    gd_handle_glFogfv,
    gd_handle_glFramebufferRenderbuffer,
    gd_handle_glFramebufferTexture,
    gd_handle_glFramebufferTexture2D,
    gd_handle_glFramebufferTexture3D,
    gd_handle_glFramebufferTextureLayer,
    gd_handle_glFrontFace,
    gd_handle_glFrustum,
    gd_handle_glGenBuffers,
    gd_handle_glGenFramebuffers,
    gd_handle_glGenLists,
    gd_handle_glGenProgramsARB,
    gd_handle_glGenQueries,
    gd_handle_glGenRenderbuffers,
    gd_handle_glGenSamplers,
    gd_handle_glGenTextures,
    gd_handle_glGenVertexArrays,
    gd_handle_glGenerateMipmap,
    gd_handle_glGetActiveAttrib,
    gd_handle_glGetActiveUniform,
    gd_handle_glGetActiveUniformBlockName,
    gd_handle_glGetActiveUniformBlockiv,
    gd_handle_glGetActiveUniformName,
    gd_handle_glGetActiveUniformsiv,
    gd_handle_glGetAttachedShaders,
    gd_handle_glGetAttribLocation,
    gd_handle_glGetBooleani_v,
    gd_handle_glGetBooleanv,
    gd_handle_glGetClipPlane,
    gd_handle_glGetCompressedTexImage,
    gd_handle_glGetError,
    gd_handle_glGetFloatv,
    gd_handle_glGetFragDataIndex,
    gd_handle_glGetFragDataLocation,
    gd_handle_glGetFramebufferAttachmentParameteriv,
    gd_handle_glGetHandleARB,
    gd_handle_glGetInfoLogARB,
    gd_handle_glGetInteger64i_v,
    gd_handle_glGetInteger64v,
    gd_handle_glGetIntegeri_v,
    gd_handle_glGetIntegerv,
    gd_handle_glGetInternalformativ,
    gd_handle_glGetLightfv,
    gd_handle_glGetLightiv,
    gd_handle_glGetMapdv,
    gd_handle_glGetMapfv,
    gd_handle_glGetMapiv,
    gd_handle_glGetMaterialfv,
    gd_handle_glGetMaterialiv,
    gd_handle_glGetMultisamplefv,
    gd_handle_glGetObjectParameterfvARB,
    gd_handle_glGetObjectParameterivARB,
    gd_handle_glGetPixelMapfv,
    gd_handle_glGetPixelMapuiv,
    gd_handle_glGetPixelMapusv,
    gd_handle_glGetPointerv,
    gd_handle_glGetPolygonStipple,
    gd_handle_glGetProgramEnvParameterdvARB,
    gd_handle_glGetProgramEnvParameterfvARB,
    gd_handle_glGetProgramInfoLog,
    gd_handle_glGetProgramLocalParameterdvARB,
    gd_handle_glGetProgramLocalParameterfvARB,
    gd_handle_glGetProgramStringARB,
    gd_handle_glGetProgramiv,
    gd_handle_glGetQueryObjectuiv,
    gd_handle_glGetQueryiv,
    gd_handle_glGetRenderbufferParameteriv,
    gd_handle_glGetSamplerParameterfv,
    gd_handle_glGetShaderInfoLog,
    gd_handle_glGetShaderPrecisionFormat,
    gd_handle_glGetShaderSource,
    gd_handle_glGetShaderiv,
    gd_handle_glGetString,
    gd_handle_glGetSynciv,
    gd_handle_glGetTexEnvfv,
    gd_handle_glGetTexEnviv,
    gd_handle_glGetTexGendv,
    gd_handle_glGetTexGenfv,
    gd_handle_glGetTexGeniv,
    gd_handle_glGetTexImage,
    gd_handle_glGetTexLevelParameterfv,
    gd_handle_glGetTexParameterfv,
    gd_handle_glGetTransformFeedbackVarying,
    gd_handle_glGetUniformBlockIndex,
    gd_handle_glGetUniformIndices,
    gd_handle_glGetUniformLocation,
    gd_handle_glGetUniformfv,
    gd_handle_glGetUniformiv,
    gd_handle_glGetUniformuiv,
    gd_handle_glGetVertexAttribIiv,
    gd_handle_glGetVertexAttribIuiv,
    gd_handle_glGetVertexAttribPointerv,
    gd_handle_glGetVertexAttribdv,
    gd_handle_glGetVertexAttribfv,
    gd_handle_glGetVertexAttribiv,
    gd_handle_glHint,
    gd_handle_glIndexMask,
    gd_handle_glIndexPointer,
    gd_handle_glIndexf,
    gd_handle_glInitNames,
    gd_handle_glInterleavedArrays,
    gd_handle_glIsBuffer,
    gd_handle_glIsEnabled,
    gd_handle_glIsEnabledi,
    gd_handle_glIsFramebuffer,
    gd_handle_glIsList,
    gd_handle_glIsProgram,
    gd_handle_glIsQuery,
    gd_handle_glIsRenderbuffer,
    gd_handle_glIsSampler,
    gd_handle_glIsShader,
    gd_handle_glIsSync,
    gd_handle_glIsTexture,
    gd_handle_glIsVertexArray,
    gd_handle_glLightModelf,
    gd_handle_glLightModelfv,
    gd_handle_glLightModeli,
    gd_handle_glLightModeliv,
    gd_handle_glLightf,
    gd_handle_glLightfv,
    gd_handle_glLineStipple,
    gd_handle_glLineWidth,
    gd_handle_glLinkProgram,
    gd_handle_glListBase,
    gd_handle_glLoadIdentity,
    gd_handle_glLoadMatrixf,
    gd_handle_glLoadName,
    gd_handle_glLoadTransposeMatrixf,
    gd_handle_glLogicOp,
    gd_handle_glMap1d,
    gd_handle_glMap1f,
    gd_handle_glMap2d,
    gd_handle_glMap2f,
    gd_handle_glMapGrid1d,
    gd_handle_glMapGrid1f,
    gd_handle_glMapGrid2d,
    gd_handle_glMapGrid2f,
    gd_handle_glMaterialf,
    gd_handle_glMaterialfv,
    gd_handle_glMatrixMode,
    gd_handle_glMinSampleShading,
    gd_handle_glMultMatrixf,
    gd_handle_glMultTransposeMatrixf,
    gd_handle_glMultiDrawElementsBaseVertex,
    gd_handle_glMultiTexCoord4f,
    gd_handle_glNewList,
    gd_handle_glNormal3f,
    gd_handle_glNormalPointer,
    gd_handle_glOrtho,
    gd_handle_glPassThrough,
    gd_handle_glPatchParameterfv,
    gd_handle_glPatchParameteri,
    gd_handle_glPixelMapfv,
    gd_handle_glPixelMapuiv,
    gd_handle_glPixelMapusv,
    gd_handle_glPixelStorei,
    gd_handle_glPixelTransferf,
    gd_handle_glPixelTransferi,
    gd_handle_glPixelZoom,
    gd_handle_glPointParameterf,
    gd_handle_glPointParameterfv,
    gd_handle_glPointParameteri,
    gd_handle_glPointParameteriv,
    gd_handle_glPointSize,
    gd_handle_glPolygonMode,
    gd_handle_glPolygonOffset,
    gd_handle_glPolygonStipple,
    gd_handle_glPopAttrib,
    gd_handle_glPopClientAttrib,
    gd_handle_glPopMatrix,
    gd_handle_glPopName,
    gd_handle_glPrioritizeTextures,
    gd_handle_glProgramEnvParameter4fARB,
    gd_handle_glProgramLocalParameter4fARB,
    gd_handle_glProgramStringARB,
    gd_handle_glProvokingVertex,
    gd_handle_glPushAttrib,
    gd_handle_glPushClientAttrib,
    gd_handle_glPushMatrix,
    gd_handle_glPushName,
    gd_handle_glQueryCounter,
    gd_handle_glRasterPos4f,
    gd_handle_glReadBuffer,
    gd_handle_glReadPixels,
    gd_handle_glReleaseShaderCompiler,
    gd_handle_glRenderMode,
    gd_handle_glRenderbufferStorage,
    gd_handle_glRenderbufferStorageMultisample,
    gd_handle_glRotatef,
    gd_handle_glSampleCoverage,
    gd_handle_glSampleMaski,
    gd_handle_glSamplerParameterf,
    gd_handle_glSamplerParameterfv,
    gd_handle_glScalef,
    gd_handle_glScissor,
    gd_handle_glSecondaryColor3f,
    gd_handle_glSecondaryColorPointer,
    gd_handle_glSelectBuffer,
    gd_handle_glShadeModel,
    gd_handle_glShaderBinary,
    gd_handle_glShaderSource,
    gd_handle_glStencilFunc,
    gd_handle_glStencilFuncSeparate,
    gd_handle_glStencilMask,
    gd_handle_glStencilMaskSeparate,
    gd_handle_glStencilOp,
    gd_handle_glStencilOpSeparate,
    gd_handle_glTexBuffer,
    gd_handle_glTexCoord4f,
    gd_handle_glTexCoordPointer,
    gd_handle_glTexEnvf,
    gd_handle_glTexEnvfv,
    gd_handle_glTexGend,
    gd_handle_glTexGendv,
    gd_handle_glTexGenf,
    gd_handle_glTexGenfv,
    gd_handle_glTexGeni,
    gd_handle_glTexGeniv,
    gd_handle_glTexImage2D,
    gd_handle_glTexImage2DMultisample,
    gd_handle_glTexImage3D,
    gd_handle_glTexImage3DMultisample,
    gd_handle_glTexParameterf,
    gd_handle_glTexParameterfv,
    gd_handle_glTexSubImage2D,
    gd_handle_glTexSubImage3D,
    gd_handle_glTransformFeedbackVaryings,
    gd_handle_glTranslatef,
    gd_handle_glUniform1f,
    gd_handle_glUniform1fv,
    gd_handle_glUniform1i,
    gd_handle_glUniform1iv,
    gd_handle_glUniform1ui,
    gd_handle_glUniform1uiv,
    gd_handle_glUniform2f,
    gd_handle_glUniform2fv,
    gd_handle_glUniform2i,
    gd_handle_glUniform2iv,
    gd_handle_glUniform2ui,
    gd_handle_glUniform2uiv,
    gd_handle_glUniform3f,
    gd_handle_glUniform3fv,
    gd_handle_glUniform3i,
    gd_handle_glUniform3iv,
    gd_handle_glUniform3ui,
    gd_handle_glUniform3uiv,
    gd_handle_glUniform4f,
    gd_handle_glUniform4fv,
    gd_handle_glUniform4i,
    gd_handle_glUniform4iv,
    gd_handle_glUniform4ui,
    gd_handle_glUniform4uiv,
    gd_handle_glUniformBlockBinding,
    gd_handle_glUniformMatrix2fv,
    gd_handle_glUniformMatrix2x3fv,
    gd_handle_glUniformMatrix2x4fv,
    gd_handle_glUniformMatrix3fv,
    gd_handle_glUniformMatrix3x2fv,
    gd_handle_glUniformMatrix3x4fv,
    gd_handle_glUniformMatrix4fv,
    gd_handle_glUniformMatrix4x2fv,
    gd_handle_glUniformMatrix4x3fv,
    gd_handle_glUseProgram,
    gd_handle_glValidateProgram,
    gd_handle_glVertex4f,
    gd_handle_glVertexAttrib1f,
    gd_handle_glVertexAttrib2f,
    gd_handle_glVertexAttrib3f,
    gd_handle_glVertexAttrib4Nbv,
    gd_handle_glVertexAttrib4Niv,
    gd_handle_glVertexAttrib4Nsv,
    gd_handle_glVertexAttrib4Nub,
    gd_handle_glVertexAttrib4Nubv,
    gd_handle_glVertexAttrib4Nuiv,
    gd_handle_glVertexAttrib4Nusv,
    gd_handle_glVertexAttrib4f,
    gd_handle_glVertexAttribDivisor,
    gd_handle_glVertexAttribI1i,
    gd_handle_glVertexAttribI1iv,
    gd_handle_glVertexAttribI1ui,
    gd_handle_glVertexAttribI1uiv,
    gd_handle_glVertexAttribI2i,
    gd_handle_glVertexAttribI2iv,
    gd_handle_glVertexAttribI2ui,
    gd_handle_glVertexAttribI2uiv,
    gd_handle_glVertexAttribI3i,
    gd_handle_glVertexAttribI3iv,
    gd_handle_glVertexAttribI3ui,
    gd_handle_glVertexAttribI3uiv,
    gd_handle_glVertexAttribI4bv,
    gd_handle_glVertexAttribI4i,
    gd_handle_glVertexAttribI4iv,
    gd_handle_glVertexAttribI4sv,
    gd_handle_glVertexAttribI4ubv,
    gd_handle_glVertexAttribI4ui,
    gd_handle_glVertexAttribI4uiv,
    gd_handle_glVertexAttribI4usv,
    gd_handle_glVertexAttribIPointer,
    gd_handle_glVertexAttribPointer,
    gd_handle_glVertexPointer,
    gd_handle_glViewport,
    gd_handle_glWaitSync,
    gd_handle_glWindowPos3f,
};

HandleRequestFunc getHandleRequestFunc(short requestCode) {
    int index = requestCode - REQUEST_CODE_GL_CALL_START;
    return index >= 0 && index < REQUEST_CODE_GL_CALL_COUNT ? handleRequestFuncs[index] : NULL;
}
