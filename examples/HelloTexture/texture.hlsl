struct VertexOutput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

cbuffer uniform0 : register(b0) {
    float4x4 model;
    float4x4 view;
    float4x4 projection;
}

VertexOutput vertex_main(float3 position : POSITION,
                         float2 texcoord : TEXCOORD) {
    VertexOutput output;

    float4x4 transform = mul(mul(model, view), projection);
    output.position = mul(float4(position, 1.0f), transform);
    output.texcoord = texcoord;

    return output;
}

Texture2D boxTexture : register(t0);
Texture2D faceTexture : register(t1);
sampler linearSampler : register(s0);

float4 pixel_main(float4 position : SV_POSITION,
                  float2 texcoord : TEXCOORD) : SV_TARGET {
    float4 boxColor = boxTexture.Sample(linearSampler, texcoord);
    float4 faceColor = faceTexture.Sample(linearSampler, texcoord);
    return lerp(boxColor, faceColor, 0.2f);
}
