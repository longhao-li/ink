cbuffer LightUniform : register(b1)
{
    float4 cameraPos;
    float4 lightPos;
    float4 objectColor;
    float4 lightColor;
}

float4 main(float4 position : SV_POSITION,
            float4 vertPos : POSITION,
            float3 normal : NORMAL) : SV_TARGET
{
    float4 ambient = 0.1f * lightColor;
    
    // diffuse
    float4 norm = float4(normalize(normal), 0.0f);
    float4 lightDir = normalize(lightPos - vertPos);
    float cosTheta = max(dot(norm, lightDir), 0.0f);
    float4 diffuse = cosTheta * lightColor;
    
    // specular
    float4 viewDir = normalize(cameraPos - vertPos);
    float4 reflectDir = reflect(-lightDir, norm);
    float4 spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    float4 specular = 0.5f * spec * lightColor;
    
    return (ambient + diffuse + specular) * objectColor;
}
