struct PixelInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

Texture2D boxTexture : register(t0);
Texture2D faceTexture : register(t1);
sampler linearSampler : register(s0);

float4 main(PixelInput input) : SV_TARGET
{
    float4 boxColor = boxTexture.Sample(linearSampler, input.texcoord);
    float4 faceColor = faceTexture.Sample(linearSampler, input.texcoord);
    return lerp(boxColor, faceColor, 0.2f);
}
