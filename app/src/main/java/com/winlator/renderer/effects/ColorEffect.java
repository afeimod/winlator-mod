package com.winlator.renderer.effects;

import com.winlator.renderer.material.ScreenMaterial;
import com.winlator.renderer.material.ShaderMaterial;

public class ColorEffect extends Effect {
    private float brightness = 0.0f;
    private float contrast = 0.0f;
    private float gamma = 1.0f;

    @Override
    public ScreenMaterial createMaterial() {
        final ShaderMaterial.Uniform brightnessUniform = new ShaderMaterial.Uniform("brightness");
        final ShaderMaterial.Uniform contrastUniform = new ShaderMaterial.Uniform("contrast");
        final ShaderMaterial.Uniform gammaUniform = new ShaderMaterial.Uniform("gamma");

        return new ScreenMaterial() {
            @Override
            protected String getFragmentShader() {
                return String.join("\n",
                    "precision highp float;",

                    "uniform sampler2D screenTexture;",
                    "uniform float brightness;",
                    "uniform float contrast;",
                    "uniform float gamma;",

                    "varying vec2 vUV;",

                    "void main() {",
                        "vec4 texelColor = texture2D(screenTexture, vUV);",
                        "vec3 color = texelColor.rgb;",

                        "color = clamp(color + brightness, 0.0, 1.0);",
                        "color = (color - 0.5) * clamp(contrast + 1.0, 0.5, 2.0) + 0.5;",
                        "color = pow(color, vec3(1.0 / gamma));",

                        "gl_FragColor = vec4(color, texelColor.a);",
                    "}"
                );
            }

            @Override
            public void use() {
                super.use();

                setUniformFloat(brightnessUniform, brightness);
                setUniformFloat(contrastUniform, contrast);
                setUniformFloat(gammaUniform, gamma);
            }
        };
    }

    public float getBrightness() {
        return brightness;
    }

    public void setBrightness(float brightness) {
        this.brightness = brightness;
    }

    public float getContrast() {
        return contrast;
    }

    public void setContrast(float contrast) {
        this.contrast = contrast;
    }

    public float getGamma() {
        return gamma;
    }

    public void setGamma(float gamma) {
        this.gamma = gamma;
    }
}
