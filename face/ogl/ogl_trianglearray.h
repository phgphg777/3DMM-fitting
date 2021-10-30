#pragma once
#include <memory>
#include "ogl_core.h"


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