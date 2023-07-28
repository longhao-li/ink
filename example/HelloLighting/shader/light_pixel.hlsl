cbuffer LightUniform : register(b1)
{
    float4 cameraPos;
    float4 lightPos;
    float4 objectColor;
    float4 lightColor;
}

float4 main(float4 position : SV_POSITION) : SV_TARGET
{
    return lightColor;
}
