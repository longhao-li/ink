struct VertexInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

cbuffer uniform0 : register(b0)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
}

VertexOutput main(VertexInput vert)
{
    VertexOutput output;
    
    float4x4 transform = mul(mul(model, view), projection);
    output.position = mul(float4(vert.position, 1.0f), transform);
    output.texcoord = vert.texcoord;
    
    return output;
}
