struct PixelInput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float2 texcoord : TEXCOORD;
};

Texture2D texture : register(t0);
sampler linearSampler : register(s0);

float4 main(PixelInput input) : SV_TARGET
{
    float4 color = texture.Sample(linearSampler, input.texcoord);
    return color * float4(input.color, 1.0f);
}
