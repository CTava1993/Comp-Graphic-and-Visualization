#version 420 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse1; // First texture
    sampler2D diffuse2; // Second texture
    vec3 specular;    
    float shininess;
}; 

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;  
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;

    vec3 lightColor;
}; 


#define NR_POINT_LIGHTS 2

in vec3 FragPos;  
in vec3 Normal; 
in vec4 ourColor;
in vec2 TexCoord;

// Speficies number of textures
uniform int numTextures; 

uniform vec3 viewPos;
uniform Material material;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform DirLight dirLight;

// Functions
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main()
{

    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    // Calculate point lights
    for (int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);


    // Combine texture samples with object color based on numTextures
    vec4 texColor = vec4(1.0); // Default to white if textures are not used

    if (numTextures == 1)
    {
        texColor = texture(material.diffuse1, TexCoord);
    }
    else if (numTextures == 2)
    {
        texColor = texture(material.diffuse1, TexCoord) * texture(material.diffuse2, TexCoord);
    }

    vec4 objectColor = ourColor;

    // Result
    FragColor = objectColor * texColor * vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient;
    vec3 diffuse  = light.diffuse  * diff;
    vec3 specular = light.specular * (spec * material.specular);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return ((ambient + diffuse + specular) * light.lightColor);
}