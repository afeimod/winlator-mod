package com.winlator.renderer.effects;

import com.winlator.renderer.material.ShaderMaterial;

public abstract class Effect {
    private ShaderMaterial material;

    protected ShaderMaterial createMaterial() {
        return null;
    }

    public ShaderMaterial getMaterial() {
        if (material == null) material = createMaterial();
        return material;
    }
}
