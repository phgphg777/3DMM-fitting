#pragma comment(lib,"opengl32.lib")
#ifdef _WIN64
	#pragma comment(lib,"glew32s(win64).lib")
	#pragma comment(lib,"glfw3(win64).lib")
#else
	#pragma comment(lib,"glew32s(win32).lib")
	#pragma comment(lib,"glfw3(win32).lib")
#endif

#define GLEW_STATIC
#include "GL/glew.h"
#include "GL/glfw3.h"
#include <iostream>
#include "ogl_core.h"



void glfw_message_loop() 
{
	WindowManager::message_loop();
}

void poll_glfw_event() 
{
	WindowManager::poll_event();
}

GLFWwindow* getSharingContext() 
{
	return WindowManager::getSharingContext();
}

void bind_sharing_context()
{
	glfwMakeContextCurrent(WindowManager::getSharingContext());
}

//---------------------------------------------------------------------------//
//                             WindowManager                                 // 
//---------------------------------------------------------------------------//
WindowManager::WindowManager()
{
	/*These glfw ftns below need to be called at once throughout the glfw application program. 
	This constructor is called only once because this class implements singleton pattern.*/
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//The virtual window for the goal of objects sharing will not be visible..
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	
	//If either width or height sets to zero, the ftn fails to the creation and returns nullptr..
	sharingContext = glfwCreateWindow(10, 10, "", 0, 0); 
	
	glfwMakeContextCurrent(sharingContext);  
	//glewExperimental = GL_TRUE;
	glewInit();

	//Now, every window to be created will be visible..
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
}

WindowManager::~WindowManager()
{
	glfwDestroyWindow(sharingContext);
	glfwTerminate();
}

void WindowManager::_message_loop()
{
	while(true)
	{
		for(auto& ele : pWindows)
			if(glfwWindowShouldClose(ele.first))
				return;

		glfwPollEvents();
	}
}

void WindowManager::_poll_event()
{
	glfwPollEvents();
}

void WindowManager::generateWindow(OpenGLWindow* window, int width, int height, int xpos, int ypos)
{
	//Creation of a context and a frame buffer as well as a window..
	GLFWwindow* context = glfwCreateWindow(width, height, "OpenGL", 0, sharingContext);  
	if(xpos>=0 && ypos>=0)
		glfwSetWindowPos(context, xpos, ypos);

	//Set all glfwwindow's callbacks to the WindowManager's static callbacks..
	glfwSetMouseButtonCallback(context, WindowManager::mouse_button_callback_dispatch);
	glfwSetCursorPosCallback(context, WindowManager::cursor_pos_callback_dispatch);
	glfwSetScrollCallback(context, WindowManager::scroll_callback_dispatch);
	glfwSetKeyCallback(context, WindowManager::key_callback_dispatch);

	//Register the correspondence between the glfwwindow (or context) and the openglwindow..
	pWindows[context] = window;

	//glfwCreateWindow() doesn't force newly created context to be current..
	glfwMakeContextCurrent(context);  

	//Dynamin link openGL functions using glew library..
	glewExperimental = GL_TRUE;
	glewInit();

	window->context = context;
}

void WindowManager::destroyWindow(OpenGLWindow* window)
{
	if(window->context==nullptr)
		return;

	window->emptyObjects();

	glfwDestroyWindow(window->context);
	glfwMakeContextCurrent(sharingContext);

	pWindows.erase(window->context);

	window->context = nullptr;
}

void WindowManager::mouse_button_callback_dispatch(GLFWwindow* context, int button, int action, int mods) 
{
	singleton().pWindows[context]->mouse_button_callback(button, action, mods);
}
void WindowManager::cursor_pos_callback_dispatch(GLFWwindow* context, double xpos, double ypos) 
{
	singleton().pWindows[context]->cursor_pos_callback(xpos, ypos);
}
void WindowManager::scroll_callback_dispatch(GLFWwindow* context, double xoffset, double yoffset) 
{
	singleton().pWindows[context]->scroll_callback(xoffset, yoffset);
}
void WindowManager::key_callback_dispatch(GLFWwindow *context, int key, int scancode, int action, int mods) 
{
	singleton().pWindows[context]->key_callback(key, scancode, action, mods);
}


//---------------------------------------------------------------------------//
//                              OpenGLWindow                                 // 
//---------------------------------------------------------------------------//
OpenGLWindow::OpenGLWindow(int width, int height, int xpos, int ypos)
{
	WindowManager::singleton().generateWindow(this, width, height, xpos, ypos);
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Default settings..
	glClearColor(0, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(context);

	View = ogl::translate(0, 0, -5.0);
	int w, h;
	glfwGetFramebufferSize(context, &w, &h);
	Projection = ogl::perspective(90.0f, float(w) / float(h), 0.01f, 100.0f);
}

OpenGLWindow::~OpenGLWindow() 
{ 
	glfwMakeContextCurrent(context);
	glDeleteVertexArrays(1, &vao);
	WindowManager::singleton().destroyWindow(this);	
}

void OpenGLWindow::message_loop()
{
	while (!glfwWindowShouldClose(context))
	{
		glfwPollEvents();
	}
}

void OpenGLWindow::displayScene() const
{
	glfwMakeContextCurrent(context);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	ogl::mat4 VP = Projection*View;

	for(uint i=0; i<scene.size(); ++i)
		scene[i]->put(VP);
	
	glfwSwapBuffers(context);
}

int OpenGLWindow::getMouseState(int button)
{
	return glfwGetMouseButton(context, button);
}

void OpenGLWindow::addObjectOnScene(std::shared_ptr<const OpenGLObject> obj)
{
	scene.push_back( obj );
}

void OpenGLWindow::emptyObjects()
{
	scene.clear();
}

void OpenGLWindow::makecurrent()
{
	glfwMakeContextCurrent(context);
}


//---------------------------------------------------------------------------//
//                              ShaderManager                                // 
//---------------------------------------------------------------------------//
std::map<std::string, std::shared_ptr<OpenGLShader>> ShaderManager::pShaders;

std::shared_ptr<OpenGLShader> ShaderManager::create_shader(const char * vs_code, const char * fs_code)
{
	glfwMakeContextCurrent( WindowManager::getSharingContext() );

	std::shared_ptr<OpenGLShader> shader(new OpenGLShader);
	
	if( !shader->loadProgramOnGPU(vs_code, fs_code) )
		throw std::runtime_error("Exception: Loading shader code is failed..");

	return shader;
}

void ShaderManager::register_shader(const std::string& name, std::shared_ptr<OpenGLShader> pShader)
{
	if(pShader->programId() == 0) 
		throw std::runtime_error("Exception: programId is zero..");

	if (pShaders.find(name) != pShaders.end())
		throw std::runtime_error("Exception: The name \"" + name + "\" is already registered..");

	pShaders[name] = pShader;
}

bool ShaderManager::find(const std::string& name)
{
	return pShaders.find(name) != pShaders.end();
}

std::shared_ptr<OpenGLShader> ShaderManager::get_shader(const std::string& name) 
{ 
	auto it = pShaders.find(name);
	if(it == pShaders.end()) 
		throw std::runtime_error("Exception: The name \"" + name + "\" is not registered..");

	return it->second;
}


//---------------------------------------------------------------------------//
//                              OpenGLShader                                 // 
//---------------------------------------------------------------------------//
bool OpenGLShader::loadProgramOnGPU(const char * vs_code, const char * fs_code)
{
	if(_programId) 
		glDeleteProgram(_programId);

	char errorMessage[2048];
	bool ret = true;

	_programId = glCreateProgram();

	auto load_shader = [&](const char* source, GLenum type) {
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, NULL);
		glCompileShader(shader);	
		glGetShaderInfoLog(shader, 1024, NULL, errorMessage);
		if (errorMessage[0] != '\0') {
			printf("%s\n", errorMessage);
			ret = false;
		}
		else
			glAttachShader(_programId, shader);
		glDeleteShader(shader);
	};
	
	load_shader(vs_code, GL_VERTEX_SHADER);
	load_shader(fs_code, GL_FRAGMENT_SHADER);
	glLinkProgram(_programId);
	glGetProgramInfoLog(_programId, 1024, NULL, errorMessage);
	if (errorMessage[0] != '\0') {
		printf("%s\n", errorMessage);
		ret = false;
	}
	
	if(!ret) {
		glDeleteProgram(_programId);
		_programId = 0;
	}

	return ret;
}

OpenGLShader::~OpenGLShader()
{
	if(_programId)
		glDeleteProgram(_programId);
}


//---------------------------------------------------------------------------//
//                              VBO			                                 // 
//---------------------------------------------------------------------------//
bool VBO::loadData(const float* data, int attributeDimension, int numVertex)
{
	if(id) {
		printf("Warning: The VBO is already loaded..\n");
		return false;
	}

	dim = attributeDimension;

	if(data!=nullptr) {
		glGenBuffers(1, &id);
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*dim*numVertex, data, GL_STATIC_DRAW);
		return true;
	}else {
		printf("Warning: There is no data for the VBO so it sets to zero..\n");
		id = 0;
		return false;
	}
}

bool VBO::connect(uint attributeLocation) const
{
	if(id) {
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glVertexAttribPointer(attributeLocation, dim, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(attributeLocation);
		return true;
	}else {
		puts("Warning: Invalid VBO, so it cannot be connected to any attribute.");
		return false;
	}
}

VBO::~VBO()
{
	if(id)
		glDeleteBuffers(1, &id);
}
