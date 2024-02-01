#define rootsig "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
                "DescriptorTable(CBV(b0, numDescriptors = 3))"

struct Transform {
    float4x4 model;
    float4x4 modelInvTranspose;
    float4x4 view;
    float4x4 projection;
    float3   cameraPos;
};

ConstantBuffer<Transform> transform : register(b0);

struct VertexInput {
    float3 position : POSITION;
    float3 normal   : NORMAL;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal   : NORMAL;
};

[RootSignature(rootsig)]
VertexOutput vertex_main(VertexInput input) {
    VertexOutput output;
    float3 worldPos = mul(float4(input.position, 1.0f), transform.model).xyz;
    output.worldPos = worldPos;
    output.normal   = mul(float4(input.normal, 0.0f), transform.modelInvTranspose).xyz;
    output.position = mul(mul(float4(worldPos, 1.0f), transform.view), transform.projection);
    return output;
}

/// 
/// Cook-Torrance BRDF implementation for local illumination.
/// 
/// Cook-Torrance BRDF is defined as:
///  f(i, 0) = (D * F * G) / (4 * dot(n, l) * dot(n, v))
///

#define Pi 3.14159265358979323846f

/// @brief
///   Schlick's approximation for Fresnel term.
///
/// @param F0
///   Reflectance at normal incidence. Should be in linear color space.
/// @param cosTheta
///   Cosine of the angle between the normal and the view direction.
float3 SchlicksFresnel(float3 F0, float cosTheta) {
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

/// @brief
///   Beckmann normal distribution function.
///
/// @param roughness
///   Roughness of the surface.
/// @param cosThetaH
///   Cosine of the angle between the normal and the half vector.
float Beckmann(float roughness, float cosThetaH) {
    float a2 = roughness * roughness;
    float c2 = cosThetaH * cosThetaH;
    float t2 = (1.0f - c2) / c2;
    return exp(-t2 / a2) / (Pi * a2 * c2 * c2);
}

/// @brief
///   GGX/Trowbridge-Reitz normal distribution function.
///
/// @param roughness
///   Roughness of the surface.
/// @param cosThetaH
///   Cosine of the angle between the normal and the half vector.
float GGX(float roughness, float cosThetaH) {
    float a2 = roughness * roughness;
    float c2 = cosThetaH * cosThetaH;
    float t = 1.0f + (a2 - 1.0f) * c2;
    return a2 / (Pi * t * t);
}

/// @brief
///   Smith's shadowing-masking function.
///
/// @param roughness
///   Roughness of the surface.
/// @param cosThetaV
///   Cosine of the angle between the normal and the view direction.
/// @param cosThetaL
///   Cosine of the angle between the normal and the light direction.
float SmithGeometry(float roughness, float cosThetaV, float cosThetaL) {
    float g1 = cosThetaV / (cosThetaV * (1.0f - roughness) + roughness);
    float g2 = cosThetaL / (cosThetaL * (1.0f - roughness) + roughness);
    return g1 * g2;
}

/// @brief
///   Cook-Torrance BRDF.
///
/// @param roughness
///   Roughness of the surface.
/// @param F0
///   Reflectance at normal incidence. Should be in linear color space.
/// @param normal
///   Normal vector of the surface.
/// @param view
///   View direction.
/// @param light
///   Light direction.
float3 CookTorrance(float roughness, float3 F0, float3 normal, float3 view, float3 light) {
    view  = normalize(view);
    light = normalize(light);
    float3 halfVector = normalize(view + light);

    float dotNV = saturate(dot(normal, view));
    float dotNH = saturate(dot(normal, halfVector));
    float dotNL = saturate(dot(normal, light));

    if (dotNL > 0) {
        float3 fresnel  = SchlicksFresnel(F0, dotNV);
        float  ndf      = GGX(roughness, dotNH); // Or use Beckmann.
        float  geometry = SmithGeometry(roughness, dotNV, dotNL);

        // Be careful with the division by zero.
        return dotNL * fresnel * ndf * geometry / (4.0f * dotNV * dotNL);
    } else {
        return float3(0.0f, 0.0f, 0.0f);
    }
}

struct Material {
    float4 color;
    float  metallic;
    float  roughness;
};

ConstantBuffer<Material> material : register(b1);

struct PointLight {
    float4 position;
    float4 color;
};

struct SpotLight {
    float4 position;
    float4 direction;
    float4 color;
    float  spot;
};

struct DirectionalLight {
    float4 direction;
    float4 color;
};

#if defined(POINT_LIGHT)
ConstantBuffer<PointLight> light : register(b2);
#elif defined(SPOT_LIGHT)
ConstantBuffer<SpotLight> light : register(b2);
#elif defined(DIRECTIONAL_LIGHT)
ConstantBuffer<DirectionalLight> light : register(b2);
#endif

typedef VertexOutput PixelInput;

#if defined(POINT_LIGHT)
[RootSignature(rootsig)]
float4 point_light_pixel_main(PixelInput input) : SV_TARGET {
    float3 normal   = input.normal;
    float3 viewDir  = normalize(transform.cameraPos - input.worldPos);
    float3 F0       = lerp(float3(0.04f, 0.04f, 0.04f), material.color.rgb, material.metallic);
    float3 lightDir = normalize(light.position.xyz - input.worldPos);
    float3 color    = light.color.rgb *  CookTorrance(material.roughness, F0, normal, viewDir, lightDir);

    // Gamma correction
	color = pow(color, float3(0.4545, 0.4545, 0.4545));
    return float4(color, 1.0f);
    // return float4(normalize(input.normal), 1.0f);
}
#elif defined(SPOT_LIGHT)
[RootSignature(rootsig)]
float4 spot_light_pixel_main(PixelInput input) : SV_TARGET {
    // To be written.
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
#elif defined(DIRECTIONAL_LIGHT)
[RootSignature(rootsig)]
float4 directional_light_pixel_main(PixelInput input) : SV_TARGET {
    float3 normal   = input.normal;
    float3 viewDir  = normalize(transform.cameraPos - input.worldPos);
    float3 F0       = lerp(float3(0.04f, 0.04f, 0.04f), material.color.rgb, material.metallic);
    float3 lightDir = normalize(-light.direction.xyz);
    float3 color    = light.color.rgb *  CookTorrance(material.roughness, F0, normal, viewDir, lightDir);

    // Gamma correction
    color = pow(color, float3(0.4545, 0.4545, 0.4545));
    return float4(color, 1.0f);
}
#endif


