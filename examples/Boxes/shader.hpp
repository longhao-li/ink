#pragma once

#include <cstddef>

struct Shader {
    const void       *data;
    const std::size_t size;
};

extern const Shader RootSig;
extern const Shader VertexShader;
extern const Shader PointLightPixelShader;
extern const Shader SpotLightPixelShader;
extern const Shader DirectionalLightPixelShader;
