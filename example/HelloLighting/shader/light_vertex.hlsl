cbuffer TransformUniform : register(b0)
{
    float4x4 model;
    float4x4 inverseModel;
    float4x4 view;
    float4x4 projection;
}

float4 main(float3 position : POSITION,
            float3 normal : NORMAL,
            float2 texcoord : TEXCOORD) : SV_POSITION
{
    float4x4 transform = mul(mul(model, view), projection);
    return mul(float4(position, 1.0f), transform);
}
