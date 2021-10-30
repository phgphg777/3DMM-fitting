#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include "ogl_linestrips.h"
#include "hmp_hair.h"

void read_from_datafile(const std::string& filename, int* pNum, int* pSize, float** pPositions);
void read_from_hmphair(const hmp::Hair& hair, int* pNum, int* pSize, float** pPositions);
void read_from_hmphair(const hmp::Hair& hair, int* pNum, int* pSize, float** pPositions, float** pColors);


void coloring_by_random(float** pColors, int numStrip, int sizeStrip)
{
	*pColors = new float[numStrip*sizeStrip*3];

	float r, g, b;
	for (int i = 0; i < numStrip; ++i) {	
		r = rand() % 256 / 255.0f;
		g = rand() % 256 / 255.0f;
		b = rand() % 256 / 255.0f;
		for (int j = 0; j < sizeStrip; ++j) {
			(*pColors) [i*sizeStrip*3 + j*3 + 0] = r;
			(*pColors) [i*sizeStrip*3 + j*3 + 1] = g;
			(*pColors) [i*sizeStrip*3 + j*3 + 2] = b;
		}
	}
}

std::shared_ptr<LineStrips> LineStrips::create(const std::string& filename)
{
	bind_sharing_context();

	std::shared_ptr<LineStrips> obj(new LineStrips);

	obj->shader = ShaderManager::get_shader("hair shader");

	float* positions, * colors;
	std::string ext_name( filename );
	ext_name.erase(0, ext_name.rfind('.')+1);
	if (ext_name == "data") {
		read_from_datafile(filename, &obj->numStrip, &obj->sizeStrip, &positions);
	}	
	else 
		return nullptr;
	coloring_by_random(&colors, obj->numStrip, obj->sizeStrip);

	obj->position.loadData(positions, 3, obj->numStrip * obj->sizeStrip);
	obj->color.loadData(colors, 3, obj->numStrip * obj->sizeStrip);

	delete [] positions; delete [] colors;

	obj->setBuffer();

	return obj;
}

std::shared_ptr<LineStrips> LineStrips::create(const hmp::Hair& hair)
{
	bind_sharing_context();

	std::shared_ptr<LineStrips> obj(new LineStrips);

	obj->shader = ShaderManager::get_shader("hair shader");

	float* positions, * colors;
	if(hair.getColors().size()==hair.getFibers().size()){
		read_from_hmphair(hair, &obj->numStrip, &obj->sizeStrip, &positions, &colors);
	}else {
		read_from_hmphair(hair, &obj->numStrip, &obj->sizeStrip, &positions);
		coloring_by_random(&colors, obj->numStrip, obj->sizeStrip);
	}

	obj->position.loadData(positions, 3, obj->numStrip * obj->sizeStrip);
	obj->color.loadData(colors, 3, obj->numStrip * obj->sizeStrip);

	delete [] positions; delete [] colors;

	obj->setBuffer();

	return obj;
}

void LineStrips::setBuffer()
{
	buffer1 = new int[numStrip];
	buffer2 = new int[numStrip];

	for (int i = 0; i < numStrip; ++i) {
		buffer1[i] = i*sizeStrip;
		buffer2[i] = sizeStrip;
	}
}

uint LineStrips::organize_vao() const
{
	const Locations& attributes = shader->attributes();
	uint vao;

	//Genrate a vao and store the states of vertex attributes
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

		position.connect(attributes.at("position"));
		color.connect(attributes.at("color"));

		glDisableVertexAttribArray(attributes.at("normal"));
		glDisableVertexAttribArray(attributes.at("texel"));

	glBindVertexArray(0);
	
	return vao;
}

void LineStrips::put(uint vao, const ogl::mat4& VP) const
{
	const Locations& uniforms = shader->uniforms();
	//ogl::mat4 MVP = VP * Model * ogl::rotate(180, {0,1,0});
	ogl::mat4 MVP = VP * Model;
	
	glUseProgram(shader->programId());
	glUniformMatrix4fv( uniforms.at("MVP"), 1, GL_FALSE, &MVP[0] );
	glUniform1i( uniforms.at("shading"), GL_FALSE );
	glUniform1i( uniforms.at("texturing"), GL_FALSE );
	
	glBindVertexArray(vao);
		if(selected_only)
			for(auto id : selected_strips) 
				glDrawArrays(GL_LINE_STRIP, id * sizeStrip, sizeStrip);
		else 
			glMultiDrawArrays(GL_LINE_STRIP,  buffer1, buffer2, numStrip);	
	glBindVertexArray(0);
}

LineStrips::~LineStrips()
{	
	delete[] buffer1; 
	delete[] buffer2; 
}

void read_from_datafile(const std::string& filename, int* pNum, int* pSize, float** pPositions)
{
	FILE * fp = fopen(filename.c_str(), "rb");
	if(NULL==fp) {
		printf("%s could not be opened...\n", filename.c_str());
		exit(0);
	}

	int num, size, temp;
	fread(&num, sizeof(int), 1, fp);

	while(1) {
		fread(&size, sizeof(int), 1, fp);
		if(size==1)
			fseek(fp, sizeof(float)*3, SEEK_CUR);
		else
			break;
	}

	fseek(fp, sizeof(int), SEEK_SET);
	*pPositions = new float[num*size*3];

	for (int i = 0; i < num; ++i) 
	{
		fread(&temp, sizeof(int), 1, fp);
		
		if(size==temp)
			fread(&(*pPositions)[i*size*3], sizeof(float), 3*size, fp);
		else if (temp == 1) {
			fseek(fp, sizeof(float)*3, SEEK_CUR);
			--i; --num;
		}else{
			printf("%s is incorrect...\n", filename.c_str());
			exit(0);
		}
	}

	*pNum = num;
	*pSize = size;

	fclose(fp);
}

void read_from_hmphair(const hmp::Hair& hair, int* pNum, int* pSize, float** pPositions)
{
	int num, size;

	auto& F = hair.getFibers();
	num = (int)F.size();
	size = (int)F[0].size();

	*pPositions = new float[num*size*3];

	for (int i = 0; i < num; ++i) 
	{
		for (int j = 0; j < size; ++j) 
		{
			(*pPositions)[i*size*3 + j*3 + 0] = (float) F[i][j][0];
			(*pPositions)[i*size*3 + j*3 + 1] = (float) F[i][j][1];
			(*pPositions)[i*size*3 + j*3 + 2] = (float) F[i][j][2];
		}
	}

	*pNum = num;
	*pSize = size;
}

void read_from_hmphair(const hmp::Hair& hair, int* pNum, int* pSize, float** pPositions, float** pColors)
{
	read_from_hmphair(hair, pNum, pSize, pPositions);

	int num = *pNum, size = *pSize;

	*pColors = new float[num*size*3];

	auto& FC = hair.getColors();
	
	for (int i = 0; i < num; ++i)
	{
		float r = (float) FC[i][0];
		float g = (float) FC[i][1];
		float b = (float) FC[i][2];
		for (int j = 0; j < size; ++j)
		{
			(*pColors)[i*size*3 + j*3 + 0] = r;
			(*pColors)[i*size*3 + j*3 + 1] = g;
			(*pColors)[i*size*3 + j*3 + 2] = b;
		}
	}
}
