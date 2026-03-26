#ifndef GLADIO_SHADER_CHUNKS_H
#define GLADIO_SHADER_CHUNKS_H

#define SHADER_CHUNK_FOG \
    "struct gd_FogParameters {\n" \
        "vec4 color;\n" \
        "float density;\n" \
        "float start;\n" \
        "float end;\n" \
        "float scale;\n" \
    "};\n" \
    "uniform gd_FogParameters gd_Fog;\n"

#define SHADER_CHUNK_POINT \
    "struct gd_PointParameters {\n" \
        "float size;\n" \
        "float sizeMin;\n" \
        "float sizeMax;\n" \
        "float fadeThresholdSize;\n" \
        "float distanceConstantAttenuation;\n" \
        "float distanceLinearAttenuation;\n" \
        "float distanceQuadraticAttenuation;\n" \
    "};\n" \
    "uniform gd_PointParameters gd_Point;\n"

#define SHADER_CHUNK_ALPHA_TEST \
    "#if GD_ALPHA_TEST\n" \
    "uniform vec2 gd_AlphaTest;\n" \
\
    "void applyAlphaTest(float alpha) {\n" \
        "int alphaTestFunc = int(gd_AlphaTest.x);\n" \
        "float alphaTestRef = gd_AlphaTest.y;\n" \
        "if (alphaTestFunc == " ALPHA_TEST_FUNC_ALWAYS " ||\n" \
        "(alphaTestFunc == " ALPHA_TEST_FUNC_LESS " && alpha < alphaTestRef) ||\n" \
        "(alphaTestFunc == " ALPHA_TEST_FUNC_EQUAL " && alpha == alphaTestRef) ||\n" \
        "(alphaTestFunc == " ALPHA_TEST_FUNC_LEQUAL " && alpha <= alphaTestRef) ||\n" \
        "(alphaTestFunc == " ALPHA_TEST_FUNC_GREATER " && alpha > alphaTestRef) ||\n" \
        "(alphaTestFunc == " ALPHA_TEST_FUNC_NOTEQUAL " && alpha != alphaTestRef) ||\n" \
        "(alphaTestFunc == " ALPHA_TEST_FUNC_GEQUAL " && alpha >= alphaTestRef)) return;\n" \
        "discard;\n" \
    "}\n" \
    "#endif\n"

#endif