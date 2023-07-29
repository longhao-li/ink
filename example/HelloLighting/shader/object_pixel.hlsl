struct Light
{
    float4 position;
    float4 ambient;
    float4 diffuse;
    float4 specular;
};

cbuffer LightUniform : register(b1)
{
    float4 cameraPos;
    Light light;
    float shininess;
}

Texture2D diffuseMap : register(t0);
Texture2D specularMap : register(t1);
sampler lightSampler : register(s0);

struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
};

float4 main(float4 position : SV_POSITION,
            float4 vertPos : POSITION,
            float3 normal : NORMAL,
            float2 texcoord : TEXCOORD) : SV_TARGET
{
    Material material;
    material.ambient = diffuseMap.Sample(lightSampler, texcoord);
    material.diffuse = material.ambient;
    material.specular = specularMap.Sample(lightSampler, texcoord);
    
    float4 ambient = material.ambient * light.ambient;
    
    // diffuse
    float4 norm = float4(normalize(normal), 0.0f);
    float4 lightDir = normalize(light.position - vertPos);
    float cosTheta = max(dot(norm, lightDir), 0.0f);
    float4 diffuse = cosTheta * light.diffuse * material.diffuse;
    
    // specular
    float4 viewDir = normalize(cameraPos - vertPos);
    float4 reflectDir = reflect(-lightDir, norm);
    
    float4 spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    float4 specular = material.specular * spec * light.specular;
    
    return ambient + diffuse + specular;
}
