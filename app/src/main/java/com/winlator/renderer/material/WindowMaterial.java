package com.winlator.renderer.material;

public class WindowMaterial extends ShaderMaterial {
    public final Uniforms uniforms = new Uniforms();

    public static class Uniforms {
        public final Uniform xform = new Uniform("xform");
        public final Uniform viewSize = new Uniform("viewSize");
        public final Uniform texture = new Uniform("texture");
        public final Uniform noAlpha = new Uniform("noAlpha");
        public final Uniform flipY = new Uniform("flipY");
    }

    @Override
    protected String getVertexShader() {
        return String.join("\n",
            "uniform float xform[6];",
            "uniform vec2 viewSize;",
            "uniform bool flipY;",

            "attribute vec2 position;",
            "varying vec2 vUV;",

            "void main() {",
                "vUV = vec2(position.x, flipY ? (1.0 - position.y) : position.y);",
                "vec2 transformedPos = applyXForm(position, xform);",
                "gl_Position = vec4(2.0 * transformedPos.x / viewSize.x - 1.0, 1.0 - 2.0 * transformedPos.y / viewSize.y, 0.0, 1.0);",
            "}"
        );
    }

    @Override
    protected String getFragmentShader() {
        return String.join("\n",
            "precision mediump float;",

            "uniform sampler2D texture;",
            "uniform float noAlpha;",
            "varying vec2 vUV;",

            "void main() {",
                "vec4 texelColor = texture2D(texture, vUV);",
                "gl_FragColor = vec4(texelColor.rgb, max(texelColor.a, noAlpha));",
            "}"
        );
    }
}
