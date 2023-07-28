struct Output
{
    float4 position : SV_POSITION;
    float4 vertPos : POSITION;
    float3 normal : NORMAL;
};

cbuffer TransformUniform : register(b0)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
}

Output main(float3 position : POSITION, float3 normal : NORMAL)
{
    Output output;
    float4x4 transform = mul(mul(model, view), projection);
    output.position = mul(float4(position, 1.0f), transform);
    output.vertPos = mul(float4(position, 1.0f), model);
    output.normal = normal;
    return output;
}