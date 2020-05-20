/*
 * Copyright (C) 2019 Inan Evin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "common.glh"
#include <uniformBuffers.glh>
#include <utility.glh>

#if defined(VS_BUILD)

Layout(0) attribute vec3 position;
Layout(1) attribute vec2 texCoord;
Layout(2) attribute vec3 normal;
Layout(4) attribute mat4 model;
Layout(8) attribute mat4 inverseTransposeModel;

out VS_OUT
{
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
} vs_out;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
	vs_out.FragPos = vec3(model * vec4(position,1.0));
	vs_out.Normal = mat3(inverseTransposeModel) * normal;
    vs_out.TexCoords = texCoord;
}

#elif defined(GEO_BUILD)

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 TexCoords;
} gs_in[];

out vec2 TexCoords; 

uniform float time;

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
    return position + vec4(direction, 0.0);
}

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

void main() {    
    vec3 normal = GetNormal();

    gl_Position = explode(gl_in[0].gl_Position, normal);
    TexCoords = gs_in[0].TexCoords;
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, normal);
    TexCoords = gs_in[1].TexCoords;
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, normal);
    TexCoords = gs_in[2].TexCoords;
    EmitVertex();
    EndPrimitive();
}

#elif defined(FS_BUILD)

#include "lighting.glh"

struct Material
{
sampler2D diffuse;
sampler2D specular;
vec3 objectColor;
float specularIntensity;
int specularExponent;
int surfaceType;
};

uniform Material material;

in VS_OUT
{
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
} fs_in;

out vec4 fragColor;


vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
	
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
	
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.specularExponent);
	
    // combine results
	vec3 ambient  = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 specular = light.specular * spec * material.specularIntensity * vec3(texture(material.specular, fs_in.TexCoords));
    return (ambient + diffuse + specular);
}


vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
	
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
	
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.specularExponent);
	
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
				 
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
	
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.specularExponent);
	
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));   
	
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));
	
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

void main()
{
	fragColor = texture(material.diffuse, fs_in.TexCoords);
	vec3 viewPos = vec3(cameraPosition.x, cameraPosition.y, cameraPosition.z);
	vec3 norm = normalize(fs_in.Normal);
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	
    vec3 result = CalculateDirectionalLight(directionalLight, norm, viewDir);
	
    // phase 2: point lights
     for(int i = 0; i < pointLightCount; i++)
        result += CalculatePointLight(pointLights[i], norm, fs_in.FragPos, viewDir);    
		
    // phase 3: spot light
	for(int i = 0; i < spotLightCount; i++)
		result += CalculateSpotLight(spotLights[0], norm, fs_in.FragPos, viewDir);    
    
	if(visualizeDepth)
	{
		float depth = LinearizeDepth(gl_FragCoord.z, cameraFar, cameraNear) / cameraFar;		
		fragColor = vec4(vec3(depth), 1);
	}
	else
	{
		float alpha = material.surfaceType == 0 ? 1.0 : texture(material.diffuse, fs_in.TexCoords).a;
		fragColor = vec4(result, alpha);
	}
}
#endif
