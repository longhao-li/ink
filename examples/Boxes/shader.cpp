#include "shader.hpp"

// Generated headers.
#include "boxes.directional_light.ps.hlsl.hpp"
#include "boxes.point_light.ps.hlsl.hpp"
#include "boxes.rootsig.hlsl.hpp"
#include "boxes.spot_light.ps.hlsl.hpp"
#include "boxes.vs.hlsl.hpp"

const Shader RootSig{g_rootsig, sizeof(g_rootsig)};
const Shader VertexShader{g_vertex_main, sizeof(g_vertex_main)};
const Shader PointLightPixelShader{g_point_light_pixel_main, sizeof(g_point_light_pixel_main)};
const Shader SpotLightPixelShader{g_spot_light_pixel_main, sizeof(g_spot_light_pixel_main)};
const Shader DirectionalLightPixelShader{g_directional_light_pixel_main,
                                         sizeof(g_directional_light_pixel_main)};
