struct Light
{
    float4 position;
    float4 ambient;
    float4 diffuse;
    float4 specular;
};

struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float shininess;
};

cbuffer LightUniform : register(b1)
{
    float4 cameraPos;
    Light light;
    Material material;
}

float4 main(float4 position : SV_POSITION) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
