struct VertexInput
{
    float3 position : POSITION;
    float3 color : COLOR;
    float2 texcoord : TEXCOORD;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float2 texcoord : TEXCOORD;
};

VertexOutput main(VertexInput vert)
{
    VertexOutput output;
    
    output.position = float4(vert.position, 1.0f);
    output.color = vert.color;
    output.texcoord = vert.texcoord;
    
    return output;
}
