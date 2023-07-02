struct VertexInput
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VertexOutput main(VertexInput vert)
{
    VertexOutput output;
    
    output.position = vert.position;
    output.color = vert.color;
    
    return output;
}
