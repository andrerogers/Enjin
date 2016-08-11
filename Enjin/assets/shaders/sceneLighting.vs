#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out VS_OUT
{
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
} vs_out;

uniform mat4 modelMat;
uniform mat4 normalMat;
uniform mat4 mvpMat;

void main()
{
	gl_Position = mvpMat * vec4(position, 1.0f);
	vs_out.FragPos = vec3(modelMat * vec4(position, 1.0f));
	vs_out.Normal = normalMat * normal;
	vs_out.TexCoords = texCoords;
}