#include <vector>
#include <array>
#include <map>
#include <memory>
#include "ogl_core.h"
#define GLEW_STATIC
#include "GL/glew.h"

#define GLSL(src) #src
static const char* vs_code = GLSL(
#version 430 core\n

layout(location = 0) in vec3 position;

layout(location = 0) uniform mat4 MVP;
layout(location = 1) uniform bool screen_coord;

void main()
{	
	if(screen_coord)
		gl_Position = vec4(position, 1.0);
	else
		gl_Position = MVP * vec4(position, 1.0);
}
);

static const char* fs_code = GLSL(
#version 430 core\n

layout(location = 2) uniform vec3 color;

out vec3 fs_color;

void main()
{
	fs_color = color;
}
);


static std::map<std::string, uint> attribute = {
	{ "position", 0 },
};
static std::map<std::string, uint> uniform = {
	{ "MVP", 0 },
	{ "screen_coord", 1 },
	{ "color", 2 },
};


class FeaturePoints : public OpenGLObject
{
	VBO vbo_pos[2];
	ogl::vec3 color[2] = { 
		{ 0.0f, 1.0f, 0.0f }, 
		{ 1.0f, 0.0f, 0.0f } 
	};
	float pointSize[2] = { 5.0f, 5.0f };
	int numFeature;

	enum {nothing=0, image_only, mesh_only, both} mode = both;

	friend std::shared_ptr<OpenGLObject> create_FeaturePoints(
		const std::vector<std::array<float, 2>>&,
		const std::vector<std::array<float, 3>>&,
		float, float);

	FeaturePoints(
		const std::vector<std::array<float, 2>>& img_points,
		const std::vector<std::array<float, 3>>& mesh_points,
		float width, float height);
	
	virtual void put(const ogl::mat4&) const;


public:
	static std::shared_ptr<OpenGLShader> get_shader();
	~FeaturePoints() {}
};


std::shared_ptr<OpenGLShader> FeaturePoints::get_shader()
{
	if (!ShaderManager::find("feature points"))
	{
		auto shader = ShaderManager::create_shader(vs_code, fs_code);
		ShaderManager::register_shader("feature points", shader);
	}
	return ShaderManager::get_shader("feature points");
}


FeaturePoints::FeaturePoints(
	const std::vector<std::array<float, 2>>& img_points,
	const std::vector<std::array<float, 3>>& mesh_points,
	float width, float height)
	: OpenGLObject(*this)
{
	if (mesh_points.size() != img_points.size())
		throw;
	
	numFeature = (int) img_points.size();
	
	std::vector<std::array<float, 2>> img_p(numFeature);
	for (int i = 0; i < numFeature; ++i)
	{
		img_p[i][0] = img_points[i][0] / width * 2 - 1;
		img_p[i][1] = img_points[i][1] / height * (-2) + 1;
	}

	vbo_pos[0].loadData(&img_p[0][0], 2, numFeature);
	vbo_pos[1].loadData(&mesh_points[0][0], 3, numFeature);
}


void FeaturePoints::put(const ogl::mat4& VP) const
{
	glDisable(GL_DEPTH_TEST);

	glUseProgram(shader->programId());

	glUniform3fv(uniform["color"], 1, &const_cast<ogl::vec3&>(color[0])[0]);
	glPointSize(pointSize[0]);
	glUniform1i(uniform["screen_coord"], GL_TRUE);
	vbo_pos[0].connect(attribute["position"]);

	glDrawArrays(GL_POINTS, 0, numFeature);

	glUniform3fv(uniform["color"], 1, &const_cast<ogl::vec3&>(color[1])[0]);
	glPointSize(pointSize[1]);
	glUniform1i(uniform["screen_coord"], GL_FALSE);
	ogl::mat4 MVP = VP * Model;
	glUniformMatrix4fv(uniform["MVP"], 1, GL_FALSE, &MVP[0]);
	vbo_pos[1].connect(attribute["position"]);

	glDrawArrays(GL_POINTS, 0, numFeature);

	glEnable(GL_DEPTH_TEST);
}


std::shared_ptr<OpenGLObject> create_FeaturePoints(
	const std::vector<std::array<float, 2>>& img_points,
	const std::vector<std::array<float, 3>>& mesh_points,
	float width, float height)
{
	return std::shared_ptr<OpenGLObject>(new FeaturePoints(img_points, mesh_points, width, height));
}