package com.winlator.renderer.material;

import android.graphics.Color;
import android.opengl.GLES20;

public abstract class ShaderMaterial {
    private static final float INV_255 = 1.0f / 255.0f;
    public int programId;

    public static class Uniform {
        public final String name;
        public int location = -1;

        public Uniform(String name) {
            this.name = name;
        }
    }

    protected static int compileShaders(String vertexShader, String fragmentShader) {
        int beginIndex = vertexShader.indexOf("void main() {");
        vertexShader = vertexShader.substring(0, beginIndex) +
            "vec2 applyXForm(vec2 p, float xform[6]) {\n" +
                "return vec2(xform[0] * p.x + xform[2] * p.y + xform[4], xform[1] * p.x + xform[3] * p.y + xform[5]);\n" +
            "}\n" +
        vertexShader.substring(beginIndex);

        int programId = GLES20.glCreateProgram();
        int[] compiled = new int[1];

        int vertexShaderId = GLES20.glCreateShader(GLES20.GL_VERTEX_SHADER);
        GLES20.glShaderSource(vertexShaderId, vertexShader);
        GLES20.glCompileShader(vertexShaderId);

        GLES20.glGetShaderiv(vertexShaderId, GLES20.GL_COMPILE_STATUS, compiled, 0);
        if (compiled[0] == 0) {
            throw new RuntimeException("Could not compile vertex shader: \n" + GLES20.glGetShaderInfoLog(vertexShaderId));
        }
        GLES20.glAttachShader(programId, vertexShaderId);

        int fragmentShaderId = GLES20.glCreateShader(GLES20.GL_FRAGMENT_SHADER);
        GLES20.glShaderSource(fragmentShaderId, fragmentShader);
        GLES20.glCompileShader(fragmentShaderId);

        GLES20.glGetShaderiv(fragmentShaderId, GLES20.GL_COMPILE_STATUS, compiled, 0);
        if (compiled[0] == 0) {
            throw new RuntimeException("Could not compile fragment shader: \n" + GLES20.glGetShaderInfoLog(fragmentShaderId));
        }
        GLES20.glAttachShader(programId, fragmentShaderId);

        GLES20.glLinkProgram(programId);

        GLES20.glDeleteShader(vertexShaderId);
        GLES20.glDeleteShader(fragmentShaderId);
        return programId;
    }

    protected String getVertexShader() {
        return "";
    }

    protected String getFragmentShader() {
        return "";
    }

    public void use() {
        if (programId == 0) programId = compileShaders(getVertexShader(), getFragmentShader());
        GLES20.glUseProgram(programId);
    }

    private int getUniformLocation(Uniform uniform) {
        if (uniform.location != -1) return uniform.location;
        int location = programId != 0 ? GLES20.glGetUniformLocation(programId, uniform.name) : -1;
        uniform.location = location;
        return location;
    }

    public void destroy() {
        GLES20.glDeleteProgram(programId);
        programId = 0;
    }

    public void setUniformColor(Uniform uniform, int color) {
        int location = getUniformLocation(uniform);
        if (location != -1) GLES20.glUniform3f(location, Color.red(color) * INV_255, Color.green(color) * INV_255, Color.blue(color) * INV_255);
    }

    public void setUniformFloat(Uniform uniform, float value) {
        int location = getUniformLocation(uniform);
        if (location != -1) GLES20.glUniform1f(location, value);
    }

    public void setUniformFloatArray(Uniform uniform, float[] values) {
        int location = getUniformLocation(uniform);
        if (location != -1) GLES20.glUniform1fv(location, values.length, values, 0);
    }

    public void setUniformInt(Uniform uniform, int value) {
        int location = getUniformLocation(uniform);
        if (location != -1) GLES20.glUniform1i(location, value);
    }

    public void setUniformBool(Uniform uniform, boolean value) {
        setUniformInt(uniform, value ? 1 : 0);
    }

    public void setUniformVec2(Uniform uniform, float x, float y) {
        int location = getUniformLocation(uniform);
        if (location != -1) GLES20.glUniform2f(location, x, y);
    }
}
