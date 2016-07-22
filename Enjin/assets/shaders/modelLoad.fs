#version 330 core

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
	
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
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
	
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

#define NR_POINT_LIGHTS 4

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;	
uniform Material material;

// Function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    // Properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Phase 1: Directional lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // Phase 2: Point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);  
    // Phase 3: Spot light
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir); 
    
    color = vec4(result, 1.0);  
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);
	
	// Diffuse Shading
	float diffCoeff = max(dot(normal, lightDir), 0.0);
	
	// Specular Shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
	vec3 diffuse = light.diffuse * diffCoeff * vec3(texture(material.texture_diffuse1, TexCoords));
	vec3 specular = light.specular * specCoeff * vec3(texture(material.texture_specular1, TexCoords));
	
	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	
	// Diffuse Shading
	float diffCoeff = max(dot(normal, lightDir), 0.0);
	
	// Specular Shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
	// Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diffCoeff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * specCoeff * vec3(texture(material.texture_specular1, TexCoords));
    
	ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
	
    return (ambient + diffuse + specular);	
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	
	// Diffuse Shading
	float diffCoeff = max(dot(lightDir, normal), 0.0);
	
	// Specular Shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
	// Attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
	
    // Spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);	
	
	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diffCoeff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * specCoeff * vec3(texture(material.texture_specular1, TexCoords));
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
	
    return (ambient + diffuse + specular);		
}