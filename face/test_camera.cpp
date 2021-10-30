#include "dlib/image_io.h"
#include "dlib/image_processing/frontal_face_detector.h"
#include "dlib/image_processing/shape_predictor.h"
//#include "ogl/ogl_trianglearray.h"
#include "ogl/ogl_core.h"


void readOBJ(
	const std::string& filename,
	std::vector< std::array<float,3> >& V, 
	std::vector< std::array<int,3> >& I, 
	std::vector< std::array<float,3> >& N, 
	std::vector< std::array<int,3> >& NI
);

std::array<float,12> estimate_camera_by_POSIT(
	const std::vector<std::array<float,2>>& image_points, 
	const std::vector<std::array<float,3>>& model_points, 
	float img_width, float img_height, float fov_y, int iter=5
);

std::shared_ptr<OpenGLObject> create_TransparentMesh(
	const std::vector<std::array<float, 3>>& V,
	const std::vector<std::array<int, 3>>& VI,
	const std::vector<std::array<float, 3>>& N,
	const std::vector<std::array<int, 3>>& NI
);

std::shared_ptr<OpenGLObject> create_FeaturePoints(
	const std::vector<std::array<float, 2>>& img_points,
	const std::vector<std::array<float, 3>>& mesh_points,
	float width, float height
);

std::shared_ptr<OpenGLObject> create_FullScreenImage(
	unsigned char* data, int width, int height
); 

void inc_alpha(std::shared_ptr<OpenGLObject> mesh);
void dec_alpha(std::shared_ptr<OpenGLObject> mesh);

#define GLFW_RELEASE				0
#define GLFW_PRESS					1
#define GLFW_KEY_MINUS              45
#define GLFW_KEY_EQUAL              61

/*
right most of right eye, left most of left eye, tip of nose, right most of lips, left most of lips, chip
*/
static const std::vector<int> feature_indices_on_model
= {4605, 454, 2816, 6808, 2538, 2639};
static const std::vector<int> feature_indices_on_image
= {36, 45, 30, 48, 54, 8};


static dlib::full_object_detection get_face_landmarks(const dlib::array2d<dlib::rgb_pixel>& img)
{
	auto face_detector = dlib::get_frontal_face_detector();
	std::vector<dlib::rectangle> face_areas = face_detector(img);

	dlib::shape_predictor face_predictor;
	dlib::deserialize("dlib/data/shape_predictor_68_face_landmarks.dat") >> face_predictor;

	dlib::full_object_detection face_landmarks = face_predictor(img, face_areas[0]);

	return face_landmarks;
}


static void centering_model(std::vector< std::array<float,3> >& V)
{
	std::vector< std::array<float,3> > feature_points;
	for(auto idx : feature_indices_on_model)
		feature_points.push_back( V[idx] );

	int num = (float)feature_points.size();
	std::array<float,3> feature_avr = { 0, 0, 0 };
	std::array<float,3> feature_min = { feature_points[0][0], feature_points[0][1], feature_points[0][2] };
	std::array<float,3> feature_max = feature_min;
	float mun = 1.0f / (float)num;
	for (int i = 0; i < num; ++i) {
		for (int j = 0; j < 3; ++j) {
			feature_avr[j] += (float)feature_points[i][j] * mun;
			if( feature_min[j] > (float)feature_points[i][j] )
				feature_min[j] = (float)feature_points[i][j];
			if( feature_max[j] < (float)feature_points[i][j] )
				feature_max[j] = (float)feature_points[i][j];
		}
	}

	float x_offset = (feature_min[0] + feature_max[0]) * 0.5f;
	float y_offset = (feature_min[1] + feature_max[1]) * 0.5f;
	float z_offset = feature_avr[2];

	for (auto& v : V)
	{
		v[0] -= x_offset;
		v[1] -= y_offset;
		v[2] -= z_offset;
	}
}


static ogl::mat4 to_GL_view_matrix(const std::array<float,12>& openCV_R_and_t)
{
	ogl::mat4 view(1.0f);
	view(0,0) = openCV_R_and_t[0]; view(0,1) = openCV_R_and_t[1]; view(0,2) = openCV_R_and_t[2];
	view(1,0) = -openCV_R_and_t[3]; view(1,1) = -openCV_R_and_t[4]; view(1,2) = -openCV_R_and_t[5];
	view(2,0) = -openCV_R_and_t[6]; view(2,1) = -openCV_R_and_t[7]; view(2,2) = -openCV_R_and_t[8];
	view(0,3) = openCV_R_and_t[9]; view(1,3) = -openCV_R_and_t[10]; view(2,3) = -openCV_R_and_t[11];
	
	return view;
}


void test_camera()
//void main()
{
	//std::string img_file = "data/images/500x700.bmp";
	std::string img_file = "data/images/jieum1.jpg";
	std::string model_file = "data/meshes/head_model.obj";

	std::vector< std::array<float,3> > V;
	std::vector< std::array<int,3> > I;
	std::vector< std::array<float,3> > N;
	std::vector< std::array<int,3> > NI;
	readOBJ(model_file, V, I, N, NI);
	//centering_model(V);

	dlib::array2d<dlib::rgb_pixel> img;
	load_image(img, img_file);
	auto face_landmarks = get_face_landmarks(img);

	std::vector< std::array<float,3> > model_points;
	for(int idx : feature_indices_on_model)
		model_points.push_back( V[idx] );

	std::vector< std::array<float,2> > image_points;
	for(int idx : feature_indices_on_image) {
		auto p = face_landmarks.part(idx);
		image_points.push_back( { (float)p(0), (float)p(1) } );
	}

	float fov_y = 60.0f;
	std::array<float,12> camera_pos = 
		estimate_camera_by_POSIT(image_points, model_points, (float)img.nc(), (float)img.nr(), fov_y);

	auto mesh = create_TransparentMesh(V, I, N, I);

	OpenGLWindow win(img.nc(), img.nr());
	win.setViewMatrix( to_GL_view_matrix(camera_pos) );
	win.setProjectionMatrix( ogl::perspective(fov_y, (float)img.nc()/(float)img.nr(), 0.001f, 1000.0f) );
	win.addObjectOnScene( mesh );
	win.addObjectOnScene( create_FullScreenImage((unsigned char*)img.begin(), img.nc(), img.nr()) );

	win.setKeyHandler(
		[&win, mesh](int key, int scancode, int action, int mods)
		{
			if (action != GLFW_PRESS) return;
			if (key == GLFW_KEY_EQUAL) inc_alpha(mesh);
			if (key == GLFW_KEY_MINUS) dec_alpha(mesh);
			win.displayScene();
		}
	);

	win.displayScene();
	win.message_loop();
}

