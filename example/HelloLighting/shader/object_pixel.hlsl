struct Light
{
    float4 position;
    float4 ambient;
    float4 diffuse;
    float4 specular;
};

struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float shininess;
};

cbuffer LightUniform : register(b1)
{
    float4 cameraPos;
    Light light;
    Material material;
}

float4 main(float4 position : SV_POSITION,
            float4 vertPos : POSITION,
            float3 normal : NORMAL) : SV_TARGET
{
    float4 ambient = material.ambient * light.ambient;
    
    // diffuse
    float4 norm = float4(normalize(normal), 0.0f);
    float4 lightDir = normalize(light.position - vertPos);
    float cosTheta = max(dot(norm, lightDir), 0.0f);
    float4 diffuse = cosTheta * light.diffuse * material.diffuse;
    
    // specular
    float4 viewDir = normalize(cameraPos - vertPos);
    float4 reflectDir = reflect(-lightDir, norm);
    float4 spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    float4 specular = material.specular * spec * light.specular;
    
    return ambient + diffuse + specular;
}
