#include <memory>
#include "ogl_core.h"
#define GLEW_STATIC
#include "GL/glew.h"

#define GLSL(src) #src
static const char* vs_code = GLSL(
#version 430 core\n

out vec2 vs_texel;

void main()
{
	const vec4 quad[4] = vec4[4](
		vec4(-1.0, -1.0, 0.999999, 1),
		vec4( 1.0, -1.0, 0.999999, 1),
		vec4( 1.0,  1.0, 0.999999, 1),
		vec4(-1.0,  1.0, 0.999999, 1)
	);
	const vec2 uv[4] = vec2[4](
		vec2(0.0, 1.0),
		vec2(1.0, 1.0),
		vec2(1.0, 0.0),
		vec2(0.0, 0.0)
	);

	gl_Position = quad[gl_VertexID];
	vs_texel = uv[gl_VertexID];
}
);

static const char* fs_code = GLSL(
#version 430 core\n

in vec2 vs_texel;

uniform sampler2D sampler;

out vec3 fs_color;

void main()
{
	fs_color = texture(sampler, vs_texel).rgb;
}
);

class FullScreenImage : public OpenGLObject
{
	friend std::shared_ptr<OpenGLObject> create_FullScreenImage(unsigned char* data, int width, int height);
	
	FullScreenImage(unsigned char* data, int width, int height);
	virtual void put(const ogl::mat4&) const;

	GLuint texture;

public:
	static std::shared_ptr<OpenGLShader> get_shader();
	~FullScreenImage() { glDeleteTextures(1, &texture); }
};


FullScreenImage::FullScreenImage(unsigned char* data, int width, int height) 
	: OpenGLObject(*this)
{
	if(width%4==0)
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	else
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	/*The default value of GL_TEXTURE_MIN_FILTER is GL_NEAREST_MIPMAP_LINEAR. So, you need to
	change it or call glGenerateMipmap(GL_TEXTURE_2D) in order to see texture image.*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

std::shared_ptr<OpenGLShader> FullScreenImage::get_shader()
{
	if (!ShaderManager::find("full screen image")) 
	{
		auto shader = ShaderManager::create_shader(vs_code, fs_code);
		ShaderManager::register_shader("full screen image", shader);
	}
	return ShaderManager::get_shader("full screen image");
}


void FullScreenImage::put(const ogl::mat4&) const
{
	glUseProgram(shader->programId());
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


std::shared_ptr<OpenGLObject> create_FullScreenImage(unsigned char* data, int width, int height) 
{
	return std::shared_ptr<OpenGLObject>(new FullScreenImage(data, width, height));
}






//#include "basic_shader.glsl"
//
//std::shared_ptr<OpenGLShader> FullScreenImage::get_shader()
//{
//	if (!ShaderManager::find("basic shader")) 
//	{
//		auto shader = ShaderManager::create_shader(vs_code, fs_code);
//		ShaderManager::register_shader("basic shader", shader);
//	}
//	return ShaderManager::get_shader("basic shader");
//}
//
//static std::map<std::string, uint> attribute = {
//	{"position", 0},
//	{"normal", 1},
//	{"texel", 2},
//	{"color", 3},
//};
//static std::map<std::string, uint> uniform = {
//	{"MVP", 0},
//	{"M", 1},
//	{"shading", 2},
//	{"texturing", 3},
//
//};
//const float quad[12] = {
//	-1.0, -1.0, 0.999999,
//	0.0, -1.0, 0.999999,
//	0.0,  1.0, 0.999999,
//	-1.0,  1.0, 0.999999
//};
//const float uv[8] = {
//	0.0, 1.0,
//	1.0, 1.0,
//	1.0, 0.0,
//	0.0, 0.0
//};
//void FullScreenImage::put(const ogl::mat4&) const
//{
//	uint pos;
//	glGenBuffers(1, &pos);
//	glBindBuffer(GL_ARRAY_BUFFER, pos);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*4, quad, GL_STATIC_DRAW);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
//	glEnableVertexAttribArray(0);
//
//	uint tex;
//	glGenBuffers(1, &tex);
//	glBindBuffer(GL_ARRAY_BUFFER, tex);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*2*4, uv, GL_STATIC_DRAW);
//	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
//	glEnableVertexAttribArray(2);
//
//	glDisableVertexAttribArray(1);
//	glDisableVertexAttribArray(3);
//
//	glUseProgram(shader->programId());
//
//	ogl::mat4 MVP(1.0f);
//	glUniformMatrix4fv( uniform["MVP"], 1, GL_FALSE, &MVP[0] );
//	glUniform1i( uniform["shading"], GL_FALSE );
//	glUniform1i( uniform["texturing"], GL_TRUE );
//
//	glBindTexture(GL_TEXTURE_2D, texture);
//
//	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//}