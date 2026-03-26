#include "shader_material.h"
#include "shader_converter.h"
#include "gl_renderer.h"

static void setupMaterial(ShaderMaterial* material, MaterialOptions* options) {
    for (int i = 0; i < VERTEX_ATTRIB_COUNT; i++) material->location.attributes[i] = -1;

    const char* attribNames[] = {"gd_Vertex", "gd_Color", "gd_Normal"};
    for (int i = 0; i < ARRAY_SIZE(attribNames); i++) {
        material->location.attributes[i] = glGetAttribLocation(material->program, attribNames[i]);
    }

    if (options->transformVertex) {
        material->location.modelViewMatrix = glGetUniformLocation(material->program, "gd_ModelViewMatrix");
        material->location.projectionMatrix = glGetUniformLocation(material->program, "gd_ProjectionMatrix");
    }
    else {
        material->location.modelViewMatrix = -1;
        material->location.projectionMatrix = -1;
    }

    if (options->numTextures >= 1) {
        char uniformName[32];
        material->location.textureMatrix = glGetUniformLocation(material->program, "gd_TextureMatrix");

        const char* texEnvStructNames[] = {"mode", "color", "combineRGBA", "rgbaScale", "sourceRGBA", "operandRGBA"};
        for (int i = 0; i < MAX_TEXCOORDS; i++) {
            material->location.texture[i] = -1;
            for (int j = 0; j < ARRAY_SIZE(texEnvStructNames); j++) material->location.texEnv[i][j] = -1;
        }

        for (int i = 0; i < MAX_TEXCOORDS; i++) {
            sprintf(uniformName, "gd_Texture%d", i);
            material->location.texture[i] = glGetUniformLocation(material->program, uniformName);

            for (int j = 0; j < ARRAY_SIZE(texEnvStructNames); j++) {
                sprintf(uniformName, "gd_TexEnv%d.%s", i, texEnvStructNames[j]);
                material->location.texEnv[i][j] = glGetUniformLocation(material->program, uniformName);
            }
        }

        for (int i = ARRAY_SIZE(attribNames), j = 0; j < MAX_TEXCOORDS; j++, i++) {
            char attribName[32];
            sprintf(attribName, "gd_MultiTexCoord%d", j);
            material->location.attributes[i] = glGetAttribLocation(material->program, attribName);
        }
    }

    if (options->alphaTest) {
        material->location.alphaTest = glGetUniformLocation(material->program, "gd_AlphaTest");
    }

    if (options->fog) ShaderMaterial_getFogUniformLocations(material->program, material->location.fog);
    ShaderMaterial_getPointUniformLocations(material->program, material->location.point);

    if (options->lighting) {
        char uniformName[32] = {0};
        const char* lightStructNames[] = {"ambient", "diffuse", "specular", "position", "attenuation", "spotCutoffExponent", "spotDirection"};
        for (int i = 0; i < MAX_LIGHTS; i++) {
            for (int j = 0; j < ARRAY_SIZE(lightStructNames); j++) {
                sprintf(uniformName, "lights[%d].%s", i, lightStructNames[j]);
                material->location.lights[i][j] = glGetUniformLocation(material->program, uniformName);
            }
        }
        material->location.numLights = glGetUniformLocation(material->program, "numLights");

        const char* materialStructNames[] = {"ambient", "diffuse", "specular", "emission"};
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < ARRAY_SIZE(materialStructNames); j++) {
                sprintf(uniformName, "materials[%d].%s", i, materialStructNames[j]);
                material->location.materials[i][j] = glGetUniformLocation(material->program, uniformName);
            }
        }
    }
}

static const char* getVertexShaderHead() {
    return
        "#version 320 es\n"

        "#define GD_MAX_LIGHTS 0\n"
        "#define GD_USE_TEXTURE 0\n"
        "#define GD_FOG 0\n"
        "#define GD_TRANSFORM_VERTEX 0\n"

    "#if GD_TRANSFORM_VERTEX\n"
        "uniform mat4 gd_ProjectionMatrix;\n"
        "uniform mat4 gd_ModelViewMatrix;\n"
    "#endif\n"

    "#if GD_MAX_LIGHTS > 0\n"
        "struct Light {\n"
            "vec3 ambient;\n"
            "vec3 diffuse;\n"
            "vec3 specular;\n"
            "vec4 position;\n"
            "vec3 attenuation;\n"
            "vec2 spotCutoffExponent;\n"
            "vec3 spotDirection;\n"
        "};\n"

        "struct Material {\n"
            "vec3 ambient;\n"
            "vec3 diffuse;\n"
            "vec4 specular;\n"
            "vec3 emission;\n"
        "};\n"

        "uniform Light lights[GD_MAX_LIGHTS];\n"
        "uniform int numLights;\n"
        "uniform Material materials[2];\n"

        "out vec3 gd_TotalDiffuseLight;\n"
        "out vec3 gd_TotalSpecularLight;\n"

        "float getLightAttenuation(Light light, vec3 viewPosition) {\n"
            "float distance = length(light.position.xyz - viewPosition);\n"
            "return 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));\n"
        "}\n"

        "float computeSpotIntensity(Light light, vec3 lightDirection) {\n"
            "if (light.spotCutoffExponent.y == 0.0) return 1.0;\n"
            "float angleCos = dot(-light.spotDirection, lightDirection);\n"
            "float spotCutoff = cos(light.spotCutoffExponent.x);\n"
            "return angleCos > spotCutoff ? pow(angleCos, light.spotCutoffExponent.y) : 0.0;\n"
        "}\n"

        "vec3 getLightDirection(Light light, vec3 viewPosition) {\n"
            "return light.position.w > 0.0 ? normalize(light.position.xyz - viewPosition) : light.position.xyz;\n"
        "}\n"
    "#endif\n"

        SHADER_CHUNK_POINT

        "float getPointSizeAttenuation(gd_PointParameters point, vec3 viewPosition) {\n"
            "float distance = length(viewPosition);\n"
            "return inversesqrt(point.distanceConstantAttenuation + point.distanceLinearAttenuation * distance + point.distanceQuadraticAttenuation * (distance * distance));\n"
        "}\n"

        "in vec4 gd_Vertex;\n"
        "in vec4 gd_Color;\n"
        "in vec3 gd_Normal;\n"
        "in vec4 gd_MultiTexCoord;\n"

        "out vec4 gd_FrontColor;\n"

    "#if GD_USE_TEXTURE\n"
        "uniform mat4 gd_TextureMatrix;\n"
        "out vec4 gd_TexCoord;\n"
    "#endif\n"

    "#if GD_FOG\n"
        "out float gd_FogFragCoord;\n"
    "#endif\n"
    ;
}

static const char* getVertexShaderBody() {
    return
        "void main() {\n"
            "gd_FrontColor = gd_Color;\n"

        "#if GD_USE_TEXTURE\n"
            "gd_TexCoord = gd_TextureMatrix * gd_MultiTexCoord;\n"
        "#endif\n"

        "#if GD_TRANSFORM_VERTEX\n"
            "vec4 position = gd_ModelViewMatrix * gd_Vertex;\n"
        "#else\n"
            "vec4 position = gd_Vertex;\n"
        "#endif\n"

        "#if GD_MAX_LIGHTS > 0\n"
            "vec3 mvNormal = vec3(gd_ModelViewMatrix * vec4(gd_Normal, 0.0));\n"
            "int face = int(dot(mvNormal, position.xyz) >= 0.0);\n"
            "gd_TotalDiffuseLight = vec3(step(float(numLights), 0.0));\n"
            "gd_TotalSpecularLight = numLights > 0 ? materials[face].emission : vec3(0.0);\n"
            "vec3 totalAmbientLight = vec3(0.0);\n"
            "vec3 viewDirection = normalize(-position.xyz);\n"
            "float shininess = materials[face].specular.w;\n"

            "for (int i = 0; i < numLights; i++) {\n"
                "totalAmbientLight += lights[i].ambient * materials[face].ambient;\n"
                "vec3 lightDirection = getLightDirection(lights[i], position.xyz);\n"
                "float spotIntensity = computeSpotIntensity(lights[i], lightDirection);\n"
                "float attenuation = getLightAttenuation(lights[i], position.xyz);\n"

                "float diffuse = clamp(dot(mvNormal, lightDirection), 0.0, 1.0);\n"
                "gd_TotalDiffuseLight += diffuse * lights[i].diffuse * materials[face].diffuse * spotIntensity * attenuation;\n"

                "vec3 lightVector = normalize(lightDirection + viewDirection);\n"
                "float specular = pow(clamp(dot(mvNormal, lightVector), 0.0, 1.0), shininess);\n"
                "gd_TotalSpecularLight += specular * lights[i].specular * materials[face].specular.rgb * spotIntensity * attenuation;\n"
            "}\n"

            "gd_TotalDiffuseLight += totalAmbientLight;\n"
        "#endif\n"

        "#if GD_FOG\n"
            "gd_FogFragCoord = -position.z;\n"
        "#endif\n"

        "#if GD_TRANSFORM_VERTEX\n"
            "gl_Position = gd_ProjectionMatrix * position;\n"
        "#else\n"
            "gl_Position = position;\n"
        "#endif\n"

            "if (gd_Point.size > 0.0) gl_PointSize = clamp(gd_Point.size * getPointSizeAttenuation(gd_Point, position.xyz), gd_Point.sizeMin, gd_Point.sizeMax);\n"
        "}\n"
    ;
}

static const char* getFragmentShaderHead() {
    return
        "#version 320 es\n"
        "precision highp float;\n"
        "precision highp int;\n"

        "#define GD_MAX_LIGHTS 0\n"
        "#define GD_USE_TEXTURE 0\n"
        "#define GD_ALPHA_TEST 0\n"
        "#define GD_FOG 0\n"
        "#define GD_POINT_SPRITE 0\n"

    SHADER_CHUNK_ALPHA_TEST

    "#if GD_MAX_LIGHTS > 0\n"
        "in vec3 gd_TotalDiffuseLight;\n"
        "in vec3 gd_TotalSpecularLight;\n"
    "#endif\n"

        "in vec4 gd_FrontColor;\n"

    "#if GD_USE_TEXTURE\n"
        "struct gd_TexEnvParameters {\n"
            "int mode;\n"
            "vec4 color;\n"
            "ivec2 combineRGBA;\n"
            "vec2 rgbaScale;\n"
            "ivec4 sourceRGBA;\n"
            "ivec4 operandRGBA;\n"
        "};\n"

        "uniform sampler2D gd_Texture;\n"
        "uniform gd_TexEnvParameters gd_TexEnv;\n"
        "in vec4 gd_TexCoord;\n"

        "vec4 getTexEnvCombineSource(ivec2 mode, vec4 textureColor, vec4 fragColor, vec4 texEnvColor) {\n"
            "vec4 result = vec4(1.0);\n"
            "if (mode.x == " TEXENV_COMBINE_PREVIOUS ") result.rgb = fragColor.rgb;\n"
            "else if (mode.x == " TEXENV_COMBINE_TEXTURE ") result.rgb = textureColor.rgb;\n"
            "else if (mode.x == " TEXENV_COMBINE_CONSTANT ") result.rgb = texEnvColor.rgb;\n"
            "else if (mode.x == " TEXENV_COMBINE_PRIMARY_COLOR ") result.rgb = gd_FrontColor.rgb;\n"
            "if (mode.y == " TEXENV_COMBINE_PREVIOUS ") result.a = fragColor.a;\n"
            "else if (mode.y == " TEXENV_COMBINE_TEXTURE ") result.a = textureColor.a;\n"
            "else if (mode.y == " TEXENV_COMBINE_CONSTANT ") result.a = texEnvColor.a;\n"
            "else if (mode.y == " TEXENV_COMBINE_PRIMARY_COLOR ") result.a = gd_FrontColor.a;\n"
            "return result;\n"
        "}\n"

        "vec4 getTexEnvCombineOperand(ivec2 mode, vec4 source) {\n"
            "vec4 result = vec4(1.0);\n"
            "if (mode.x == " TEXENV_COMBINE_SRC_COLOR ") result.rgb = source.rgb;\n"
            "else if (mode.x == " TEXENV_COMBINE_ONE_MINUS_SRC_COLOR ") result.rgb = 1.0 - source.rgb;\n"
            "if (mode.y == " TEXENV_COMBINE_SRC_ALPHA ") result.a = source.a;\n"
            "else if (mode.y == " TEXENV_COMBINE_ONE_MINUS_SRC_ALPHA ") result.a = 1.0 - source.a;\n"
            "return result;\n"
        "}\n"

        "vec4 applyTexEnv(sampler2D sampler, vec4 texCoord, gd_TexEnvParameters texEnv, vec4 fragColor) {\n"
            "vec4 textureColor = texture(sampler, texCoord.xy);\n"
            "int modeRGB = texEnv.mode;\n"
            "int modeAlpha = " TEXENV_MODE_MODULATE ";\n"
            "vec4 operand0 = textureColor;\n"
            "vec4 operand1 = fragColor;\n"

            "if (texEnv.mode == " TEXENV_MODE_COMBINE ") {\n"
                "modeRGB = texEnv.combineRGBA.x;\n"
                "modeAlpha = texEnv.combineRGBA.y;\n"
                "vec4 source0 = getTexEnvCombineSource(texEnv.sourceRGBA.xz, textureColor, fragColor, texEnv.color);\n"
                "vec4 source1 = getTexEnvCombineSource(texEnv.sourceRGBA.yw, textureColor, fragColor, texEnv.color);\n"
                "operand0 = getTexEnvCombineOperand(texEnv.operandRGBA.xz, source0);\n"
                "operand1 = getTexEnvCombineOperand(texEnv.operandRGBA.yw, source1);\n"
            "}\n"

            "if (modeRGB == " TEXENV_MODE_MODULATE ")\n"
                "operand0.rgb *= operand1.rgb;\n"
            "else if (modeRGB == " TEXENV_MODE_DECAL ")\n"
                "operand0.rgb = operand1.rgb * (1.0 - operand0.a) + operand0.rgb * operand0.a;\n"
            "else if (modeRGB == " TEXENV_MODE_INTERPOLATE ")\n"
                "operand0.rgb = operand1.rgb * (1.0 - operand0.rgb) + texEnv.color.rgb * operand0.rgb;\n"
            "else if (modeRGB == " TEXENV_MODE_ADD ")\n"
                "operand0.rgb += operand1.rgb;\n"
            "else if (modeRGB == " TEXENV_MODE_ADD_SIGNED ")\n"
                "operand0.rgb = operand0.rgb + operand1.rgb - 0.5;\n"
            "else if (modeRGB == " TEXENV_MODE_SUBTRACT ")\n"
                "operand0.rgb -= operand1.rgb;\n"
            "else if (modeRGB == " TEXENV_MODE_DOT3_RGB ") {\n"
                "operand0.rgb = vec3(4.0*((operand0.r - 0.5)*(operand1.r - 0.5)+(operand0.g - 0.5)*(operand1.g - 0.5)+(operand0.b - 0.5)*(operand1.b - 0.5)));\n"
                "modeAlpha = " TEXENV_MODE_REPLACE ";\n"
            "}\n"
            "else if (modeRGB == " TEXENV_MODE_DOT3_RGBA ") {\n"
                "operand0.rgba = vec4(4.0*((operand0.r - 0.5)*(operand1.r - 0.5)+(operand0.g - 0.5)*(operand1.g - 0.5)+(operand0.b - 0.5)*(operand1.b - 0.5)));\n"
                "modeAlpha = " TEXENV_MODE_REPLACE ";\n"
            "}\n"

            "if (modeAlpha == " TEXENV_MODE_MODULATE ")\n"
                "operand0.a *= operand1.a;\n"
            "else if (modeAlpha == " TEXENV_MODE_DECAL ")\n"
                "operand0.a = operand1.a * (1.0 - operand0.a) + operand0.a * operand0.a;\n"
            "else if (modeAlpha == " TEXENV_MODE_INTERPOLATE ")\n"
                "operand0.a = operand1.a * (1.0 - operand0.a) + texEnv.color.a * operand0.a;\n"
            "else if (modeAlpha == " TEXENV_MODE_ADD ")\n"
                "operand0.a += operand1.a;\n"
            "else if (modeAlpha == " TEXENV_MODE_ADD_SIGNED ")\n"
                "operand0.a = operand0.a + operand1.a - 0.5;\n"
            "else if (modeAlpha == " TEXENV_MODE_SUBTRACT ")\n"
                "operand0.a -= operand1.a;\n"

            "operand0.rgb *= texEnv.rgbaScale.x;\n"
            "operand0.a *= texEnv.rgbaScale.y;\n"
            "return operand0;\n"
        "}\n"
    "#endif\n"

    "#if GD_FOG\n"
        SHADER_CHUNK_FOG

        "in float gd_FogFragCoord;\n"

        "vec3 applyFog(gd_FogParameters fog, vec3 fragColor, float fogFragCoord) {\n"
            "float fogFactor = 0.0;\n"
            "int fogMode = int(fog.scale);\n"

            "if (fogMode == " FOG_MODE_LINEAR ") {\n"
                "fogFactor = smoothstep(fog.start, fog.end, fogFragCoord);\n"
            "}\n"
            "else if (fogMode == " FOG_MODE_EXP ") {\n"
                "fogFactor = 1.0 - clamp(exp(-fog.density * fogFragCoord), 0.0, 1.0);\n"
            "}\n"
            "else if (fogMode == " FOG_MODE_EXP2 ") {\n"
                "const float LOG2 = 1.442695;\n"
                "fogFactor = 1.0 - clamp(exp2(-fog.density * fog.density * fogFragCoord * fogFragCoord * LOG2), 0.0, 1.0);\n"
            "}\n"

            "return mix(fragColor, fog.color.rgb, fogFactor);\n"
        "}\n"
    "#endif\n"

    "#if GD_POINT_SPRITE\n"
        "uniform sampler2D gd_PointSprite;\n"

        SHADER_CHUNK_POINT
    "#endif\n"

        "out vec4 gd_FragColor;\n"
    ;
}

static const char* getFragmentShaderBody() {
    return
        "void main() {\n"

        "#if GD_MAX_LIGHTS > 0\n"
            "vec4 finalColor = vec4(gd_FrontColor.rgb * gd_TotalDiffuseLight + gd_TotalSpecularLight, gd_FrontColor.a);\n"
        "#else\n"
            "vec4 finalColor = gd_FrontColor;\n"
        "#endif\n"

        "#if GD_POINT_SPRITE\n"
            "if (gd_Point.size > 0.0) {\n"
                "finalColor *= texture(gd_PointSprite, gl_PointCoord);\n"
                "if (finalColor.a <= 0.1) discard;\n"
            "}\n"
        "#endif\n"

        "#if GD_USE_TEXTURE\n"
            "finalColor = applyTexEnv(gd_Texture, gd_TexCoord, gd_TexEnv, finalColor);\n"
        "#endif\n"

        "#if GD_FOG\n"
            "finalColor.rgb = applyFog(gd_Fog, finalColor.rgb, gd_FogFragCoord);\n"
        "#endif\n"

        "#if GD_ALPHA_TEST\n"
            "applyAlphaTest(finalColor.a);\n"
        "#endif\n"

            "gd_FragColor = finalColor;\n"
        "}\n"
    ;
}

static char* defineShaderOptions(GLenum type, char* source, MaterialOptions* options) {
    int maxLights = options->lighting ? MAX_LIGHTS : 0;

    bool useTexture = options->numTextures >= 1;
    if (useTexture) {
        if (type == GL_VERTEX_SHADER) {
            ArrayBuffer replaceBufs[3] = {0};
            for (int i = 0; i < options->numTextures; i++) {
                ArrayBuffer_putString(&replaceBufs[0], "in vec4 gd_MultiTexCoord%d;\n", i);
                ArrayBuffer_putString(&replaceBufs[1], "out vec4 gd_TexCoord%d;\n", i);
                ArrayBuffer_putString(&replaceBufs[2], "gd_TexCoord%d = gd_TextureMatrix * gd_MultiTexCoord%d;\n", i, i);
            }

            char* search[] = {"in vec4 gd_MultiTexCoord;", "out vec4 gd_TexCoord;", "gd_TexCoord = gd_TextureMatrix * gd_MultiTexCoord;"};
            char* oldSource = NULL;
            for (int i = 0; i < ARRAY_SIZE(replaceBufs); i++) {
                ArrayBuffer_put(&replaceBufs[i], '\0');
                source = str_replace(search[i], replaceBufs[i].buffer, oldSource = source);
                free(oldSource);
                ArrayBuffer_free(&replaceBufs[i]);
            }
        }
        else if (type == GL_FRAGMENT_SHADER) {
            ArrayBuffer replaceBufs[4] = {0};
            for (int i = 0; i < options->numTextures; i++) {
                ArrayBuffer_putString(&replaceBufs[0], "uniform sampler2D gd_Texture%d;\n", i);
                ArrayBuffer_putString(&replaceBufs[1], "uniform gd_TexEnvParameters gd_TexEnv%d;\n", i);
                ArrayBuffer_putString(&replaceBufs[2], "in vec4 gd_TexCoord%d;\n", i);
                ArrayBuffer_putString(&replaceBufs[3], "finalColor = applyTexEnv(gd_Texture%d, gd_TexCoord%d, gd_TexEnv%d, finalColor);\n", i, i, i);
            }

            char* search[] = {"uniform sampler2D gd_Texture;", "uniform gd_TexEnvParameters gd_TexEnv;", "in vec4 gd_TexCoord;", "finalColor = applyTexEnv(gd_Texture, gd_TexCoord, gd_TexEnv, finalColor);"};
            char* oldSource = NULL;
            for (int i = 0; i < ARRAY_SIZE(replaceBufs); i++) {
                ArrayBuffer_put(&replaceBufs[i], '\0');
                source = str_replace(search[i], replaceBufs[i].buffer, oldSource = source);
                free(oldSource);
                ArrayBuffer_free(&replaceBufs[i]);
            }
        }
    }

    FOREACH_LINE(source, strlen(source)+1,
         if (cstartswith("#define GD_MAX_LIGHTS", line)) {
             line[lineLen-1] = INT2CHR(maxLights);
         }
         else if (cstartswith("#define GD_USE_TEXTURE", line)) {
             line[lineLen-1] = INT2CHR(useTexture);
         }
         else if (cstartswith("#define GD_ALPHA_TEST", line)) {
             line[lineLen-1] = INT2CHR(options->alphaTest);
         }
         else if (cstartswith("#define GD_FOG", line)) {
             line[lineLen-1] = INT2CHR(options->fog);
         }
         else if (cstartswith("#define GD_POINT_SPRITE", line)) {
             line[lineLen-1] = INT2CHR(options->pointSprite);
         }
         else if (cstartswith("#define GD_TRANSFORM_VERTEX", line)) {
             line[lineLen-1] = INT2CHR(options->transformVertex);
         }
    );

    return source;
}

ShaderMaterial* ShaderMaterial_create(MaterialOptions* options) {
    ShaderMaterial* material = calloc(1, sizeof(ShaderMaterial));

    char* vertexShader = strjoin('\n', 2, getVertexShaderHead(), options->vertexProgram ? options->vertexProgram->shaderCode : getVertexShaderBody());
    char* fragmentShader = strjoin('\n', 2, getFragmentShaderHead(), strdup(options->fragmentProgram ? options->fragmentProgram->shaderCode : getFragmentShaderBody()));

    vertexShader = defineShaderOptions(GL_VERTEX_SHADER, vertexShader, options);
    fragmentShader = defineShaderOptions(GL_FRAGMENT_SHADER, fragmentShader, options);

    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, (const GLchar* const*) &vertexShader, NULL);
    glCompileShader(vertexShaderId);
    GLint success;

#if IS_DEBUG_ENABLED(DEBUG_MODE_SHADER_INFO)
    printShaderLines(GL_VERTEX_SHADER, vertexShaderId, generateMaterialHash(options), vertexShader, strlen(vertexShader));
#endif

    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        println("gladio: could not compile vertex shader\n%s\n", infoLog);
        exit(1);
    }

    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderId, 1, (const GLchar* const*)&fragmentShader, NULL);
    glCompileShader(fragmentShaderId);

#if IS_DEBUG_ENABLED(DEBUG_MODE_SHADER_INFO)
    printShaderLines(GL_FRAGMENT_SHADER, fragmentShaderId, generateMaterialHash(options), fragmentShader, strlen(fragmentShader));
#endif

    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(fragmentShaderId, 512, NULL, infoLog);
        println("gladio: could not compile fragment shader\n%s\n", infoLog);
        exit(1);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShaderId);
    glAttachShader(program, fragmentShaderId);
    glLinkProgram(program);

    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    free(vertexShader);
    free(fragmentShader);

    material->program = program;
    setupMaterial(material, options);

    return material;
}

void ShaderMaterial_destroy(ShaderMaterial* material) {
    if (!material) return;

    SparseArray_free(&material->location.programEnv, false);
    SparseArray_free(&material->location.programLocal, false);
    glDeleteProgram(material->program);
}

static void setLightUniforms(GLRenderer* renderer, ShaderMaterial* material) {
    int numLights = 0;
    for (int i = 0, j; i < MAX_LIGHTS; i++) {
        GLLight* light = &renderer->lights[i];
        if (light->enabled) {
            j = numLights;
            glUniform3fv(material->location.lights[j][0], 1, (const GLfloat*)light->ambient);
            glUniform3fv(material->location.lights[j][1], 1, (const GLfloat*)light->diffuse);
            glUniform3fv(material->location.lights[j][2], 1, (const GLfloat*)light->specular);
            glUniform4fv(material->location.lights[j][3], 1, (const GLfloat*)light->position);
            glUniform3fv(material->location.lights[j][4], 1, (const GLfloat*)light->attenuation);
            glUniform2fv(material->location.lights[j][5], 1, (const GLfloat*)light->spotCutoffExponent);
            glUniform3fv(material->location.lights[j][6], 1, (const GLfloat*)light->spotDirection);
            numLights++;
        }
    }
    glUniform1i(material->location.numLights, numLights);
}

static void setMaterialUniforms(GLRenderer* renderer, ShaderMaterial* material) {
    if (!renderer->materials) return;
    GLMaterial* materials = renderer->materials;

    for (int i = 0; i < 2; i++) {
        glUniform3fv(material->location.materials[i][0], 1, (const GLfloat*)materials[i].ambient);
        glUniform3fv(material->location.materials[i][1], 1, (const GLfloat*)materials[i].diffuse);
        glUniform4fv(material->location.materials[i][2], 1, (const GLfloat*)materials[i].specular);
        glUniform3fv(material->location.materials[i][3], 1, (const GLfloat*)materials[i].emission);
    }
}

static void setARBProgramUniforms(ShaderMaterial* material, ARBProgram* program) {
    for (int i = 0; i < program->envParams.size; i++) {
        SparseArray_Entry* entry = &program->envParams.entries[i];
        void* location = SparseArray_get(&material->location.programEnv, entry->key);
        if (!location) {
            char uniformName[32];
            sprintf(uniformName, "gd_ProgramEnv[%d]", entry->key);
            location = (void*)(int64_t)glGetUniformLocation(material->program, uniformName);
            SparseArray_put(&material->location.programEnv, entry->key, location);
        }
        glUniform4fv((int64_t)location, 1, entry->value);
    }

    for (int i = 0; i < program->localParams.size; i++) {
        SparseArray_Entry* entry = &program->localParams.entries[i];
        void* location = SparseArray_get(&material->location.programLocal, entry->key);
        if (!location) {
            char uniformName[32];
            sprintf(uniformName, "gd_ProgramLocal[%d]", entry->key);
            location = (void*)(int64_t)glGetUniformLocation(material->program, uniformName);
            SparseArray_put(&material->location.programLocal, entry->key, location);
        }
        glUniform4fv((int64_t)location, 1, entry->value);
    }
}

void ShaderMaterial_updatePointUniforms(ShaderMaterial* material, GLRenderer* renderer) {
    glUniform1f(material->location.point[0], renderer->state.point.size);
    glUniform1f(material->location.point[1], renderer->state.point.sizeMin);
    glUniform1f(material->location.point[2], renderer->state.point.sizeMax);
    glUniform1f(material->location.point[4], renderer->state.point.distanceAttenuation[0]);
    glUniform1f(material->location.point[5], renderer->state.point.distanceAttenuation[1]);
    glUniform1f(material->location.point[6], renderer->state.point.distanceAttenuation[2]);
}

void ShaderMaterial_updateUniforms(ShaderMaterial* material, GLRenderer* renderer, MaterialOptions* options) {
    if (options->transformVertex) {
        glUniformMatrix4fv(material->location.modelViewMatrix, 1, GL_FALSE, GLRenderer_getMatrixFromStack(renderer, MODEL_VIEW_MATRIX_INDEX));
        glUniformMatrix4fv(material->location.projectionMatrix, 1, GL_FALSE, GLRenderer_getMatrixFromStack(renderer, PROJECTION_MATRIX_INDEX));
    }

    if (options->numTextures >= 1) {
        glUniformMatrix4fv(material->location.textureMatrix, 1, GL_FALSE, GLRenderer_getMatrixFromStack(renderer, TEXTURE_MATRIX_INDEX));

        for (int i = 0; i < MAX_TEXCOORDS; i++) {
            if (material->location.texture[i] != -1) {
                glUniform1i(material->location.texture[i], i);

                glUniform1i(material->location.texEnv[i][0], renderer->state.texEnv[i].mode);
                glUniform4fv(material->location.texEnv[i][1], 1, renderer->state.texEnv[i].color);
                glUniform2iv(material->location.texEnv[i][2], 1, renderer->state.texEnv[i].combineRGBA);
                glUniform2fv(material->location.texEnv[i][3], 1, renderer->state.texEnv[i].rgbaScale);
                glUniform4iv(material->location.texEnv[i][4], 1, renderer->state.texEnv[i].sourceRGBA);
                glUniform4iv(material->location.texEnv[i][5], 1, renderer->state.texEnv[i].operandRGBA);
            }
        }
    }

    if (options->alphaTest) {
        glUniform2f(material->location.alphaTest, renderer->state.alphaTest.func, renderer->state.alphaTest.ref);
    }

    if (options->fog) {
        glUniform4fv(material->location.fog[0], 1, renderer->state.fog.color);
        glUniform1f(material->location.fog[1], renderer->state.fog.density);
        glUniform1f(material->location.fog[2], renderer->state.fog.start);
        glUniform1f(material->location.fog[3], renderer->state.fog.end);
        glUniform1f(material->location.fog[4], renderer->state.fog.mode);
    }

    if (options->lighting) {
        setLightUniforms(renderer, material);
        setMaterialUniforms(renderer, material);
    }

    if (options->vertexProgram) setARBProgramUniforms(material, options->vertexProgram);
    if (options->fragmentProgram) setARBProgramUniforms(material, options->fragmentProgram);
}