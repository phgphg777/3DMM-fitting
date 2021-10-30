#pragma once

#define GLSL(src) #src
static const char* vs_code = GLSL(
#version 430 core\n

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texel;
layout(location = 3) in vec3 color;

out vec2 fs_texel;
out vec3 fs_color;
out vec3 fs_normal;
out vec3 lightDir;

layout(location = 0) uniform mat4 MVP;
layout(location = 1) uniform mat4 M;
layout(location = 2) uniform bool shading;

vec3 lightPos = vec3(0, 30, 100);

void main()
{
	gl_Position = MVP * vec4(position, 1.0);
	fs_texel = texel;
	fs_color = color;

	if (shading) {
		vec3 pos = (M * vec4(position, 1)).xyz;
		fs_normal = (M * vec4(normal, 0)).xyz;
		lightDir = lightPos - pos;
	}
}
);

static const char* fs_code = GLSL(
#version 430 core\n

in vec2 fs_texel;
in vec3 fs_color;
in vec3 fs_normal;
in vec3 lightDir;
out vec3 color;

uniform sampler2D sampler;
layout(location = 2) uniform bool shading;
layout(location = 3) uniform bool texturing;

float ambient = 0.5;
float diffuse = 0.5;

void main()
{
	vec3 light = vec3(1, 1, 1);
	if (shading) {
		vec3 n = normalize(fs_normal);
		vec3 l = normalize(lightDir);
		diffuse *= max(dot(n, l), 0);
		light *= diffuse + ambient;
	}

	vec3 base;
	if(texturing)
		base = texture(sampler, fs_texel).rgb;
	else
		base = fs_color;
	
	color = light * base;
}
);
