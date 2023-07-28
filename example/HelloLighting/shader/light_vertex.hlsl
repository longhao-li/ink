struct Output
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

cbuffer TransformUniform : register(b0)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
}

Output main(float3 position : POSITION, float2 texcoord : TEXCOORD)
{
    Output output;
    float4x4 transform = mul(mul(model, view), projection);
    output.position = mul(float4(position, 1.0f), transform);
    output.texcoord = texcoord;
    return output;
}
