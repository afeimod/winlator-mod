#include "gl_context.h"
#include "gl_vao.h"
#include "gl_client_state.h"

void GLVertexArrayObject_setAttribState(GLClientState* clientState, GLuint index, uint8_t state, bool clear) {
    switch (index) {
        case GL_VERTEX_ARRAY:
            index = POSITION_ARRAY_INDEX;
            if (state) state = VERTEX_ATTRIB_LEGACY_ENABLED;
            break;
        case GL_COLOR_ARRAY:
            index = COLOR_ARRAY_INDEX;
            if (state) state = VERTEX_ATTRIB_LEGACY_ENABLED;
            break;
        case GL_NORMAL_ARRAY:
            index = NORMAL_ARRAY_INDEX;
            if (state) state = VERTEX_ATTRIB_LEGACY_ENABLED;
            break;
        case GL_TEXTURE_COORD_ARRAY:
            index = TEXCOORD_ARRAY_INDEX;
            if (state) state = VERTEX_ATTRIB_LEGACY_ENABLED;
            break;
    }

    if (index < VERTEX_ATTRIB_COUNT) {
        if (clear) {
            GLVertexAttrib* vertexAttrib = &clientState->vao->attribs[index];
#ifdef GL_SERVER
            vertexAttrib->normalized = false;
            vertexAttrib->type = 0;
            vertexAttrib->size = 0;
            vertexAttrib->boundArrayBuffer = 0;
#endif
            vertexAttrib->pointer = NULL;
            vertexAttrib->stride = 0;
        }
        clientState->vao->attribs[index].state = state;
        if (state) {
            clientState->vao->maxEnabledAttribs = MAX(index+1, clientState->vao->maxEnabledAttribs);
        }
        else if (clientState->vao->maxEnabledAttribs > 0 && index >= clientState->vao->maxEnabledAttribs-1) {
            clientState->vao->maxEnabledAttribs = 0;
            for (int i = 0; i < VERTEX_ATTRIB_COUNT; i++) if (clientState->vao->attribs[i].state) clientState->vao->maxEnabledAttribs = i+1;
        }
    }
}

void GLVertexArrayObject_setBound(GLClientState* clientState, GLuint id) {
    if (id > 0) {
        GLVertexArrayObject* vao = SparseArray_get(&clientState->vertexArrays, id);
        if (!vao) {
            vao = calloc(1, sizeof(GLVertexArrayObject));
            SparseArray_put(&clientState->vertexArrays, id, vao);
        }
        clientState->vao = vao;
    }
    else clientState->vao = &clientState->defaultVAO;
}

void GLVertexArrayObject_delete(GLClientState* clientState, GLuint id) {
    GLVertexArrayObject* vao;
    if (id > 0) {
        vao = SparseArray_get(&clientState->vertexArrays, id);
        if (vao == clientState->vao) clientState->vao = &clientState->defaultVAO;
    }
    else vao = &clientState->defaultVAO;
    if (!vao) return;

    if (vao->bgraBuffer > 0) {
        glDeleteBuffers(1, &vao->bgraBuffer);
        vao->bgraBuffer = 0;
    }

    if (id > 0) {
        MEMFREE(vao);
        SparseArray_remove(&clientState->vertexArrays, id);
    }
}