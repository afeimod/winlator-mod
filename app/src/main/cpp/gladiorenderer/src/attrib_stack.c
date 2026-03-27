#include "attrib_stack.h"
#include "gl_renderer.h"

void AttribStack_push(GLbitfield mask) {
    AttribStack* stack = calloc(1, sizeof(AttribStack));
    stack->mask = mask;

    if (mask & GL_COLOR_BUFFER_BIT) {
        GLRenderer_getParamsv(currentRenderer, GL_ALPHA_TEST, GL_BOOL, &stack->alphaTest);
        GLRenderer_getParamsv(currentRenderer, GL_ALPHA_TEST_FUNC, GL_INT, &stack->alphaTestFunc);
        GLRenderer_getParamsv(currentRenderer, GL_ALPHA_TEST_REF, GL_FLOAT, &stack->alphaTestRef);

        GLRenderer_getParamsv(currentRenderer, GL_BLEND, GL_BOOL, &stack->blend);
        GLRenderer_getParamsv(currentRenderer, GL_BLEND_SRC, GL_INT, &stack->blendSrc);
        GLRenderer_getParamsv(currentRenderer, GL_BLEND_DST, GL_INT, &stack->blendDst);

        GLRenderer_getParamsv(currentRenderer, GL_COLOR_CLEAR_VALUE, GL_FLOAT, &stack->clearColor);
        GLRenderer_getParamsv(currentRenderer, GL_COLOR_WRITEMASK, GL_BOOL, &stack->colorMask);
    }

    if (mask & GL_CURRENT_BIT) {
        stack->activeTexCoord = currentRenderer->clientState.activeTexCoord;
        GLRenderer_getParamsv(currentRenderer, GL_CURRENT_COLOR, GL_FLOAT, &stack->color);
        GLRenderer_getParamsv(currentRenderer, GL_CURRENT_NORMAL, GL_FLOAT, &stack->normal);
        GLRenderer_getParamsv(currentRenderer, GL_CURRENT_TEXTURE_COORDS, GL_FLOAT, &stack->texCoord);
    }

    if (mask & GL_DEPTH_BUFFER_BIT) {
        GLRenderer_getParamsv(currentRenderer, GL_DEPTH_TEST, GL_BOOL, &stack->depthTest);
        GLRenderer_getParamsv(currentRenderer, GL_DEPTH_FUNC, GL_INT, &stack->depthFunc);
        GLRenderer_getParamsv(currentRenderer, GL_DEPTH_CLEAR_VALUE, GL_FLOAT, &stack->clearDepth);
        GLRenderer_getParamsv(currentRenderer, GL_DEPTH_WRITEMASK, GL_INT, &stack->depthMask);
    }

    if (mask & GL_ENABLE_BIT) {
        GLRenderer_getParamsv(currentRenderer, GL_ALPHA_TEST, GL_BOOL, &stack->alphaTest);
        GLRenderer_getParamsv(currentRenderer, GL_BLEND, GL_BOOL, &stack->blend);
        GLRenderer_getParamsv(currentRenderer, GL_CULL_FACE, GL_BOOL, &stack->cullFace);
        GLRenderer_getParamsv(currentRenderer, GL_DEPTH_TEST, GL_BOOL, &stack->depthTest);
        GLRenderer_getParamsv(currentRenderer, GL_FOG, GL_BOOL, &stack->fog);
        GLRenderer_getParamsv(currentRenderer, GL_SCISSOR_TEST, GL_BOOL, &stack->scissorTest);
        GLRenderer_getParamsv(currentRenderer, GL_STENCIL_TEST, GL_BOOL, &stack->stencilTest);
    }

    if (mask & GL_FOG_BIT) {
        GLRenderer_getParamsv(currentRenderer, GL_FOG, GL_BOOL, &stack->fog);
        GLRenderer_getParamsv(currentRenderer, GL_FOG_COLOR, GL_FLOAT, &stack->fogColor);
        GLRenderer_getParamsv(currentRenderer, GL_FOG_DENSITY, GL_FLOAT, &stack->fogDensity);
        GLRenderer_getParamsv(currentRenderer, GL_FOG_START, GL_FLOAT, &stack->fogStart);
        GLRenderer_getParamsv(currentRenderer, GL_FOG_END, GL_FLOAT, &stack->fogEnd);
        GLRenderer_getParamsv(currentRenderer, GL_FOG_MODE, GL_FLOAT, &stack->fogMode);
    }

    if (mask & GL_SCISSOR_BIT) {
        GLRenderer_getParamsv(currentRenderer, GL_SCISSOR_TEST, GL_BOOL, &stack->scissorTest);
        GLRenderer_getParamsv(currentRenderer, GL_SCISSOR_BOX, GL_FLOAT, &stack->scissorBox);
    }

    if (mask & GL_STENCIL_BUFFER_BIT) {
        GLRenderer_getParamsv(currentRenderer, GL_STENCIL_TEST, GL_BOOL, &stack->stencilTest);
        GLRenderer_getParamsv(currentRenderer, GL_STENCIL_FUNC, GL_INT, &stack->stencilFunc);
        GLRenderer_getParamsv(currentRenderer, GL_STENCIL_VALUE_MASK, GL_INT, &stack->stencilMask);
        GLRenderer_getParamsv(currentRenderer, GL_STENCIL_REF, GL_INT, &stack->stencilRef);
        GLRenderer_getParamsv(currentRenderer, GL_STENCIL_FAIL, GL_INT, &stack->stencilFail);
        GLRenderer_getParamsv(currentRenderer, GL_STENCIL_PASS_DEPTH_FAIL, GL_INT, &stack->stencilPassDepthFail);
        GLRenderer_getParamsv(currentRenderer, GL_STENCIL_PASS_DEPTH_PASS, GL_INT, &stack->stencilPassDepthPass);
        GLRenderer_getParamsv(currentRenderer, GL_STENCIL_CLEAR_VALUE, GL_INT, &stack->clearStencil);
    }

    if (mask & GL_TEXTURE_BIT) {
        stack->activeTexture = currentRenderer->clientState.activeTexture;
        for (int i = 0; i < MAX_TEXTURE_TARGETS; i++) {
            GLTexture* texture = currentRenderer->clientState.texture[i];
            stack->boundTexture[i] = texture ? texture->id : 0;
        }
    }

    if (mask & GL_VIEWPORT_BIT) {
        GLRenderer_getParamsv(currentRenderer, GL_VIEWPORT, GL_INT, &stack->viewportSize);
        GLRenderer_getParamsv(currentRenderer, GL_DEPTH_RANGE, GL_FLOAT, &stack->depthRange);
    }

    ArrayList_add(&currentRenderer->attribStacks, stack);
}

void AttribStack_pop() {
    if (currentRenderer->attribStacks.size == 0) return;
    AttribStack* stack = ArrayList_removeAt(&currentRenderer->attribStacks, currentRenderer->attribStacks.size-1);

    if (stack->mask & GL_COLOR_BUFFER_BIT) {
        GLRenderer_setCapabilityState(currentRenderer, GL_ALPHA_TEST, stack->alphaTest, -1);
        currentRenderer->state.alphaTest.func = stack->alphaTestFunc;
        currentRenderer->state.alphaTest.ref = stack->alphaTestRef;

        GLRenderer_setCapabilityState(currentRenderer, GL_BLEND, stack->blend, -1);
        glBlendFunc(stack->blendSrc, stack->blendDst);

        glClearColor(stack->clearColor[0], stack->clearColor[1], stack->clearColor[2], stack->clearColor[3]);
        glColorMask(stack->colorMask[0], stack->colorMask[1], stack->colorMask[2], stack->colorMask[3]);
    }

    if (stack->mask & GL_CURRENT_BIT) {
        memcpy(currentRenderer->state.color, stack->color, sizeof(stack->color));
        memcpy(currentRenderer->state.normal, stack->normal, sizeof(stack->normal));
        memcpy(&currentRenderer->state.texCoords[stack->activeTexCoord], stack->texCoord, sizeof(stack->texCoord));
    }

    if (stack->mask & GL_DEPTH_BUFFER_BIT) {
        GLRenderer_setCapabilityState(currentRenderer, GL_DEPTH_TEST, stack->depthTest, -1);
        glDepthFunc(stack->depthFunc);
        glClearDepthf(stack->clearDepth);
        glDepthMask(stack->depthMask);
    }

    if (stack->mask & GL_ENABLE_BIT) {
        GLRenderer_setCapabilityState(currentRenderer, GL_ALPHA_TEST, stack->alphaTest, -1);
        GLRenderer_setCapabilityState(currentRenderer, GL_BLEND, stack->blend, -1);
        GLRenderer_setCapabilityState(currentRenderer, GL_CULL_FACE, stack->cullFace, -1);
        GLRenderer_setCapabilityState(currentRenderer, GL_DEPTH_TEST, stack->depthTest, -1);
        GLRenderer_setCapabilityState(currentRenderer, GL_FOG, stack->fog, -1);
        GLRenderer_setCapabilityState(currentRenderer, GL_SCISSOR_TEST, stack->scissorTest, -1);
        GLRenderer_setCapabilityState(currentRenderer, GL_STENCIL_TEST, stack->stencilTest, -1);
    }

    if (stack->mask & GL_FOG_BIT) {
        GLRenderer_setCapabilityState(currentRenderer, GL_FOG, stack->fog, -1);
        GLRenderer_setFogParams(currentRenderer, GL_FOG_COLOR, stack->fogColor);
        GLRenderer_setFogParams(currentRenderer, GL_FOG_DENSITY, &stack->fogDensity);
        GLRenderer_setFogParams(currentRenderer, GL_FOG_START, &stack->fogStart);
        GLRenderer_setFogParams(currentRenderer, GL_FOG_END, &stack->fogEnd);
        GLRenderer_setFogParams(currentRenderer, GL_FOG_MODE, &stack->fogMode);
    }

    if (stack->mask & GL_SCISSOR_BIT) {
        GLRenderer_setCapabilityState(currentRenderer, GL_SCISSOR_TEST, stack->scissorTest, -1);
        glScissor(stack->scissorBox[0], stack->scissorBox[1], stack->scissorBox[2], stack->scissorBox[3]);
    }

    if (stack->mask & GL_STENCIL_BUFFER_BIT) {
        GLRenderer_setCapabilityState(currentRenderer, GL_STENCIL_TEST, stack->stencilTest, -1);
        glStencilFunc(stack->stencilFunc, stack->stencilRef, stack->stencilMask);
        glStencilOp(stack->stencilFail, stack->stencilPassDepthFail, stack->stencilPassDepthPass);
        glClearStencil(stack->clearStencil);
    }

    if (stack->mask & GL_TEXTURE_BIT) {
        glActiveTexture(GL_TEXTURE0 + stack->activeTexture);
        for (int i = 0; i < MAX_TEXTURE_TARGETS; i++) GLTexture_bind(getTexTargetAt(i), stack->boundTexture[i]);
    }

    if (stack->mask & GL_VIEWPORT_BIT) {
        glViewport(stack->viewportSize[0], stack->viewportSize[1], stack->viewportSize[2], stack->viewportSize[3]);
        glDepthRangef(stack->depthRange[0], stack->depthRange[1]);
    }

    free(stack);
}