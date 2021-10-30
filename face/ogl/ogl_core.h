#pragma once
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "ogl_basic.h"
typedef unsigned int uint;


class OpenGLObject;
struct GLFWwindow;

class WindowManager
{
	friend class OpenGLWindow;
	
	//This defines a 1-1 correspondence between GLFWwindow (and context) and OpenGLWindow..
	std::map<GLFWwindow*, OpenGLWindow*> pWindows;

	/*This virtual window exists only for openGL objects sharing such as
	vbo, texture and shader (not including vao)..*/
	GLFWwindow* sharingContext = nullptr;

	void generateWindow(OpenGLWindow* window, int width, int height, int xpos, int ypos);
	void destroyWindow(OpenGLWindow* window);

	WindowManager();
	~WindowManager();

	static WindowManager& singleton() {		//singleton pattern
		static WindowManager unique_instance;
		return unique_instance;
	}

	void _message_loop();
	void _poll_event();

public:
	static void message_loop() { singleton()._message_loop(); }
	static void poll_event() { singleton()._poll_event(); }
	static GLFWwindow* getSharingContext()  { return singleton().sharingContext; }

private:
	static void mouse_button_callback_dispatch(GLFWwindow* context, int button, int action, int mods);
	static void cursor_pos_callback_dispatch(GLFWwindow* context, double xpos, double ypos);
	static void scroll_callback_dispatch(GLFWwindow* context, double xoffset, double yoffset);
	static void key_callback_dispatch(GLFWwindow *context, int key, int scancode, int action, int mods);
};



class OpenGLWindow
{
	friend class WindowManager;
	
	//It contains visible objects constructing a scene..
	std::vector< std::shared_ptr<const OpenGLObject> > scene;
	
	/*It contain object's vaos w.r.t the context. If you want to render same object 
	in two different windows(context), you must have two separate vaos for each window. 
	since opengl doesn't surpport sharing of vao(other than vbo)..*/
	//Finally, I decided to not use vao for each object.
	uint vao = 0;  

	std::function<void(int,int,int)> mouseButtonHandler = [](int,int,int){};
	std::function<void(double,double)> cursorPosHandler = [](double,double){};
	std::function<void(double,double)> scrollHandler = [](double,double){};
	std::function<void(int,int,int,int)> keyHandler = [](int,int,int,int){};

	OpenGLWindow(const OpenGLWindow&) = delete;
	void operator=(const OpenGLWindow&) = delete;

protected:
	GLFWwindow* context = nullptr;
	ogl::mat4 View;			//View matrix for openGL pipeline
	ogl::mat4 Projection;	//Projection matrix for openGL pipeline

protected:
	virtual void mouse_button_callback(int button, int action, int mods) {
		mouseButtonHandler(button, action, mods);
	}
	virtual void cursor_pos_callback(double xpos, double ypos) {
		cursorPosHandler(xpos, ypos);
	}
	virtual void scroll_callback(double xoffset, double yoffset) {
		scrollHandler(xoffset, yoffset);
	}
	virtual void key_callback(int key, int scancode, int action, int mods) {
		keyHandler(key, scancode, action, mods);
	}

public:
	OpenGLWindow(int width = 640, int height = 480, int xpos = -1, int ypos = -1);
	virtual ~OpenGLWindow();

	void setMouseButtonHandler(const std::function<void(int,int,int)>& ftn) 
		{ mouseButtonHandler = ftn; }
	void setCursorPosHandler(const std::function<void(double,double)>& ftn) 
		{ cursorPosHandler = ftn; }
	void setScrollHandler(const std::function<void(double,double)>& ftn) 
		{ scrollHandler = ftn; }
	void setKeyHandler(const std::function<void(int,int,int,int)>& ftn) 
		{ keyHandler = ftn; }

	const ogl::mat4& getViewMatrix() const 
		{ return View; }
	const ogl::mat4& getProjectionMatrix() const 
		{ return Projection; }
	void setViewMatrix(const ogl::mat4& V) 
		{ View = V; }
	void setProjectionMatrix(const ogl::mat4& P) 
		{ Projection = P; }

	int getMouseState(int button);
	void addObjectOnScene(std::shared_ptr<const OpenGLObject> obj);
	void emptyObjects();
	void displayScene() const;
	void message_loop();
	void makecurrent();
};


class OpenGLShader
{
	uint _programId = 0;

	friend class ShaderManager; 
	OpenGLShader(){}
	OpenGLShader(const OpenGLShader&) = delete;
	void operator=(const OpenGLShader&) = delete;
	bool loadProgramOnGPU(const char * vs_code, const char * fs_code);

public:
	~OpenGLShader();
	uint programId() const { return _programId; }
};


class ShaderManager
{
	static std::map< std::string, std::shared_ptr<OpenGLShader> > pShaders;

public:
	static std::shared_ptr<OpenGLShader> create_shader(const char * vs_code, const char * fs_code);
	static void register_shader(const std::string& name, std::shared_ptr<OpenGLShader> pShader);
	static bool find(const std::string& name);
	static std::shared_ptr<OpenGLShader> get_shader(const std::string& name);
};


class OpenGLObject
{
	friend class OpenGLWindow;
	virtual void put(const ogl::mat4& VP) const = 0;

protected:
	ogl::mat4 Model = {1.0f};
	std::shared_ptr<OpenGLShader> shader;

	template<typename derived>
	OpenGLObject(derived&) { shader = derived::get_shader(); }

public:
	const ogl::mat4& getModelMatrix() const { return Model; }
	void setModelMatrix(const ogl::mat4& M) { Model = M; }  
	virtual ~OpenGLObject() {}
};


class VBO
{
	uint id = 0;
	int dim = 0;
	VBO(const VBO&) = delete;
	void operator=(const VBO&) = delete;

public:
	VBO(){}
	~VBO();
	bool loadData(const float* data, int attributeDimension, int numVertex);
	bool connect(uint attributeLocation) const;
};

void bind_sharing_context();

