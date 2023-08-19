struct VertexOutput {
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

VertexOutput vertex_main(float4 position : POSITION,
                         float4 color    : COLOR) {
    VertexOutput output;
    output.position = position;
    output.color    = color;
    return output;
}

float4 pixel_main(float4 position : SV_POSITION,
                  float4 color    : COLOR) : SV_TARGET {
    return color;
}
