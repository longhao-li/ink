cbuffer LightUniform : register(b1)
{
    float4 objectColor;
    float4 lightColor;
}

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
    return objectColor;
}
