//#include <vector>
//#include <array>
//#include "ogl/GL/glew.h"
//#include "ogl/GL/glfw3.h"
//#include "ogl/ogl_core.h"
//#include "dlib/image_io.h"
//
//
//static const char* img_file = 
////"data/images/a.bmp";
////"data/images/jaw2.jpg";
////"data/images/400x600.bmp";
////"data/images/740x740.bmp";
//"data/images/jieum1.jpg";
////"data/images/Tom_Cruise_avp_2014_4.jpg";
//
//
//
//class TexturedQuad : public OpenGLObject
//{
//	virtual uint organize_vao() const;
//	virtual void put(uint vao, const ogl::mat4& VP) const;
//public:
//	TexturedQuad();
//	~TexturedQuad(){}
//};
//
//
//TexturedQuad::TexturedQuad()
//{
//	bind_sharing_context();
//	shader = ShaderManager::get_shader("textured quad");
//}
//
//uint TexturedQuad::organize_vao() const
//{
//	return 0;
//}
//
//void TexturedQuad::put(uint vao, const ogl::mat4& VP) const
//{
//	glUseProgram(shader->programId());
//	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//}
//
//
//void main()
//{
//	auto shader = ShaderManager::create_shader(vs_source, fs_source);
//	ShaderManager::register_shader("textured quad", shader);
//
//	dlib::array2d<dlib::rgb_pixel> img;
//	load_image(img, img_file);
//	
//	OpenGLWindow win(img.nc(), img.nr());
//	win.addObjectOnScene(std::shared_ptr<TexturedQuad>(new TexturedQuad));
//	win.displayScene();
//	win.message_loop();
//	
//}




//void readOBJ(
//	const std::string& filename,
//	std::vector< std::array<float,3> >& V, 
//	std::vector< std::array<int,3> >& I, 
//	std::vector< std::array<float,3> >& N, 
//	std::vector< std::array<int,3> >& NI
//);
//
//
//void test_ogl()
////void main()
//{
//	std::vector< std::array<float,3> > V;
//	std::vector< std::array<int,3> > I;
//	std::vector< std::array<float,3> > N;
//	std::vector< std::array<int,3> > NI;
//	readOBJ("data/meshes/head_model.obj", V, I, N, NI);
//
//	auto head = TriangleArray::create(V, I, N, NI);
//
//	ogl::mat4 view(1.0f);
//	view(0,0) = -1; view(0,1) = 0; view(0,2) = 0;
//	view(1,0) = 0; view(1,1) = -1; view(1,2) = 0;
//	view(2,0) = 0; view(2,1) = 0; view(2,2) = 1;
//
//	//view(0,3) = 0; view(1,3) = 0; view(2,3) = 0;
//	//view(0,3) = 0; view(1,3) = -1.76185f; view(2,3) = -0.156346f - 0.05f;
//	view(0,3) = 0; view(1,3) = 1.0f; view(2,3) = -2.0f;
//
//	OpenGLWindow win(640, 480);
//	win.setViewMatrix(view);
//	win.addObjectOnScene(head);
//	win.displayScene();
//	win.message_loop();
//}