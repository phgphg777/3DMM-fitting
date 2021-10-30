#include <map>
#include <memory>
#include "ogl_core.h"
#define GLEW_STATIC
#include "GL/glew.h"

#define GLSL(src) #src
static const char* vs_code = GLSL(
#version 430 core\n

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) uniform mat4 MVP;
layout(location = 1) uniform mat4 M;
layout(location = 2) uniform bool shading;

out vec3 vs_normal;
out vec3 lightDir;

void main()
{
	vec3 lightPos = vec3(0, 30, 100);

	gl_Position = MVP * vec4(position, 1.0);

	if (shading) {
		vec3 pos = (M * vec4(position, 1)).xyz;
		vs_normal = (M * vec4(normal, 0)).xyz;
		lightDir = lightPos - pos;
	}
}
);

static const char* fs_code = GLSL(
#version 430 core\n

in vec3 vs_normal;
in vec3 lightDir;

layout(location = 2) uniform bool shading;
layout(location = 3) uniform vec3 color;
layout(location = 4) uniform float alpha;

out vec4 fs_color;

void main()
{
	vec3 light = vec3(1, 1, 1);
	float ambient = 0.5;
	float diffuse = 0.5;

	if (shading) {
		vec3 n = normalize(vs_normal);
		vec3 l = normalize(lightDir);
		diffuse *= max(dot(n, l), 0);
		light *= diffuse + ambient;
	}

	fs_color.xyz = light * color;
	fs_color.a = alpha;
}
); 


static std::map<std::string, uint> attribute = {
	{ "position", 0 },
	{ "normal", 1 },
};
static std::map<std::string, uint> uniform = {
	{ "MVP", 0 },
	{ "M", 1 },
	{ "shading", 2 },
	{ "color", 3 },
	{ "alpha", 4 },
};


class TransparentMesh : public OpenGLObject
{
	VBO position;
	VBO normal;
	ogl::vec3 meterialColor = { 1.0f, 1.0f, 1.0f };
	float alpha = 0.75f;
	float d_alpha = 0.25f;
	uint numTriangle;

	TransparentMesh() : OpenGLObject(*this) {}
	virtual void put(const ogl::mat4& VP) const;

public:
	static std::shared_ptr<OpenGLShader> get_shader();

	template<typename Vertices, typename Normals, typename Indices>
	static std::shared_ptr<TransparentMesh> create(
		const Vertices& V, const Indices& VI, const Normals& N, const Indices& NI);
	
	void inc_alpha() {
		alpha += d_alpha;
		alpha = (alpha < 1.0f) ? alpha : 1.0f;
	}
	void dec_alpha() {
		alpha -= d_alpha;
		alpha = (0.0f < alpha) ? alpha : 0.0f;
	}

	~TransparentMesh() {}
};


std::shared_ptr<OpenGLShader> TransparentMesh::get_shader()
{
	if (!ShaderManager::find("transparent mesh"))
	{
		auto shader = ShaderManager::create_shader(vs_code, fs_code);
		ShaderManager::register_shader("transparent mesh", shader);
	}
	return ShaderManager::get_shader("transparent mesh");
}


template<typename Vertices, typename Normals, typename Indices>
std::shared_ptr<TransparentMesh> TransparentMesh::create(
	const Vertices& V, const Indices& VI, const Normals& N, const Indices& NI)
{
	bind_sharing_context();

	if (VI.size() != NI.size())
		throw std::runtime_error("Indices for normals must be provided in the same size for vertices");

	std::shared_ptr<TransparentMesh> obj(new TransparentMesh);

	///////////////////////////////////////////////////////////////////////////////////////
	int num = (int)(obj->numTriangle = (unsigned int)VI.size());

	float(*pos)[3][3] = new float[num][3][3];
	for (int i = 0; i<num; i++)
		for (int j = 0; j<3; j++)
			for (int k = 0; k<3; k++)
				pos[i][j][k] = (float)V[VI[i][j]][k];

	float(*nor)[3][3] = new float[num][3][3];
	for (int i = 0; i < num; i++)
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 3; k++)
				nor[i][j][k] = (float)N[NI[i][j]][k];
	///////////////////////////////////////////////////////////////////////////////////////

	obj->position.loadData((float*)pos, 3, 3 * obj->numTriangle);
	obj->normal.loadData((float*)nor, 3, 3 * obj->numTriangle);
	delete[] pos;
	delete[] nor;

	return obj;
}


void TransparentMesh::put(const ogl::mat4& VP) const
{
	ogl::mat4 MVP = VP * Model;

	position.connect(attribute["position"]);
	normal.connect(attribute["normal"]);

	glUseProgram(shader->programId());

	glUniformMatrix4fv(uniform["MVP"], 1, GL_FALSE, &MVP[0]);
	glUniformMatrix4fv(uniform["M"], 1, GL_FALSE, &const_cast<ogl::mat4&>(Model)[0]);
	glUniform3fv(uniform["color"], 1, &const_cast<ogl::vec3&>(meterialColor)[0]);
	glUniform1f(uniform["alpha"], alpha);
	glUniform1i(uniform["shading"], GL_TRUE);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_TRIANGLES, 0, 3 * numTriangle);
	glBlendFunc(GL_ONE, GL_ZERO);
}




#include <vector>
#include <array>


std::shared_ptr<OpenGLObject> create_TransparentMesh(
	const std::vector<std::array<float, 3>>& V,
	const std::vector<std::array<int, 3>>& VI,
	const std::vector<std::array<float, 3>>& N,
	const std::vector<std::array<int, 3>>& NI)
{
	return TransparentMesh::create(V, VI, N, NI);
}


void inc_alpha(std::shared_ptr<OpenGLObject> mesh)
{
	std::static_pointer_cast<TransparentMesh>(mesh)->inc_alpha();
}


void dec_alpha(std::shared_ptr<OpenGLObject> mesh)
{
	std::static_pointer_cast<TransparentMesh>(mesh)->dec_alpha();
}