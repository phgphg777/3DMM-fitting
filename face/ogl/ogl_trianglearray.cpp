#include <vector>
#include <array>
#include <map>
#include <memory>
#include "ogl_core.h"
#define GLEW_STATIC
#include "GL/glew.h"
#include "basic_shader.glsl"


static std::map<std::string, uint> attribute = {
	{"position", 0},
	{"normal", 1},
	{"texel", 2},
	{"color", 3},
};
static std::map<std::string, uint> uniform = {
	{"MVP", 0},
	{"M", 1},
	{"shading", 2},
	{"texturing", 3},
};


class TriangleArray : public OpenGLObject
{
	VBO position;
	VBO normal;
	uint numTriangle;	
	ogl::vec3 meterialColor = {1.0f, 1.0f, 1.0f};

	TriangleArray() : OpenGLObject(*this){}
	virtual void put(const ogl::mat4& VP) const;

public:
	static std::shared_ptr<OpenGLShader> get_shader();

	template<typename Vertices, typename Normals, typename Indices>
	static std::shared_ptr<TriangleArray> create(
		const Vertices& V, const Indices& VI, const Normals& N, const Indices& NI);

	~TriangleArray(){}
};


std::shared_ptr<OpenGLShader> TriangleArray::get_shader()
{
	if (!ShaderManager::find("basic shader")) 
	{
		auto shader = ShaderManager::create_shader(vs_code, fs_code);
		ShaderManager::register_shader("basic shader", shader);
	}
	return ShaderManager::get_shader("basic shader");
}


template<typename Vertices, typename Normals, typename Indices>
std::shared_ptr<TriangleArray> TriangleArray::create(
	const Vertices& V, const Indices& VI, const Normals& N, const Indices& NI)
{
	bind_sharing_context();

	if (VI.size() != NI.size()) 
		throw std::runtime_error("Indices for normals must be provided in the same size for vertices");
	
	std::shared_ptr<TriangleArray> obj(new TriangleArray);
	
///////////////////////////////////////////////////////////////////////////////////////
	int num = (int) (obj->numTriangle = (unsigned int)VI.size());
	
	float (*pos)[3][3] = new float[num][3][3];
	for (int i = 0; i<num; i++)
		for (int j = 0; j<3; j++)
			for (int k = 0; k<3; k++)
				pos[i][j][k] = (float) V[ VI[i][j] ] [k];

	float (*nor)[3][3] = new float[num][3][3];
	for (int i = 0; i < num; i++)
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 3; k++)
				nor[i][j][k] = (float) N[ NI[i][j] ][k];
///////////////////////////////////////////////////////////////////////////////////////

	obj->position.loadData((float*)pos, 3, 3*obj->numTriangle);
	obj->normal.loadData((float*)nor, 3, 3*obj->numTriangle);
	delete [] pos; 
	delete [] nor; 

	return obj;
}


void TriangleArray::put(const ogl::mat4& VP) const
{
	ogl::mat4 MVP = VP * Model;

	position.connect(attribute["position"]);
	normal.connect(attribute["normal"]);
	glDisableVertexAttribArray(attribute["texel"]);
	glDisableVertexAttribArray(attribute["color"]);
	glVertexAttrib3fv(attribute["color"], &const_cast<ogl::vec3&>(meterialColor)[0]);

	glUseProgram(shader->programId());

	glUniformMatrix4fv( uniform["MVP"], 1, GL_FALSE, &MVP[0] );
	glUniformMatrix4fv( uniform["M"], 1, GL_FALSE, &const_cast<ogl::mat4&>(Model)[0] );
	glUniform1i( uniform["shading"], GL_TRUE );
	glUniform1i( uniform["texturing"], GL_FALSE );

	glDrawArrays(GL_TRIANGLES, 0, 3 * numTriangle);
}


std::shared_ptr<OpenGLObject> create_TriangleArray(
	const std::vector<std::array<float, 3>>& V,
	const std::vector<std::array<int, 3>>& VI,
	const std::vector<std::array<float, 3>>& N,
	const std::vector<std::array<int, 3>>& NI)
{
	return TriangleArray::create(V, VI, N, NI);
}