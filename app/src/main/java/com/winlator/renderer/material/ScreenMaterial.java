package com.winlator.renderer.material;

public class ScreenMaterial extends ShaderMaterial {
    public final Uniforms uniforms = new Uniforms();

    public static class Uniforms {
        public final Uniform resolution = new Uniform("resolution");
        public final Uniform screenTexture = new Uniform("screenTexture");
    }

    @Override
    protected String getVertexShader() {
        return String.join("\n",
            "attribute vec2 position;",
            "varying vec2 vUV;",

            "void main() {",
                "vUV = position;",
                "gl_Position = vec4(2.0 * position.x - 1.0, 2.0 * position.y - 1.0, 0.0, 1.0);",
            "}"
        );
    }

    @Override
    protected String getFragmentShader() {
        return String.join("\n",
            "precision mediump float;",

            "uniform sampler2D screenTexture;",

            "varying vec2 vUV;",

            "void main() {",
                "gl_FragColor = texture2D(screenTexture, vUV);",
            "}"
        );
    }
}
