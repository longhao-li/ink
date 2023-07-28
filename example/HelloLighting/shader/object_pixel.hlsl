cbuffer LightUniform : register(b1)
{
    float4 lightPos;
    float4 objectColor;
    float4 lightColor;
}

float4 main(float4 position : SV_POSITION,
            float4 vertPos  : POSITION,
            float3 normal   : NORMAL) : SV_TARGET
{
    float4 ambient = 0.1f * lightColor;
    
    // diffuse
    normal = normalize(normal);
    float4 lightDir = normalize(lightPos - vertPos);
    float cosTheta = max(dot(float4(normal, 0.0f), lightDir), 0.0f);
    float4 diffuse = cosTheta * lightColor;
    
    return (ambient + diffuse) * objectColor;
}
