#include "main.h"


std::vector<int> test_landmarks = 
{37, 46, 31, 49, 55, 9};
std::vector<int> six_landmarks = 
{37, 46, 31, 49, 55, 9};
std::vector<int> full_sfm_landmarks = 
{9,18,19,
20,21,22,23,24,25,26,27,28,29,
30,31,32,33,34,35,36,37,38,39,
40,41,42,43,44,45,46,47,48,49,
50,51,52,53,54,55,56,57,58,59,
60,   62,63,64,   66,67,68};
std::vector<int> key_landmarks =
{37,40,43,46,
32,36,
49,55,
9};
std::vector<int> key_landmarks2 =
{37,38,39,40,  37,40,
43,44,45,46,  43,46,
29,30,31,32,33,34,35,36,  32,36,
49,51,52,53,55,57,58,59,  49,55,
9,  9
};


static int fitting_iter = 1;
static int posit_iter = 3;
static float fov_y = 60.0f;
static float regular_weight = 1.0f;
static int active_dim = -1;

static const char* img_file = 
//"data/images/b.bmp";
//"data/images/greedy_0_0.bmp";
//"data/images/jaw.bmp";
"data/images/jaw.jpg";
//"data/images/jaw2.jpg";
//"data/images/1024x759.bmp";
//"data/images/1024x946.bmp";
//"data/images/400x600.bmp";
//"data/images/500x700.bmp";
//"data/images/740x740.bmp";
//"data/images/jieum1.jpg";


void main()
{
	//Note that ibug face landmarks start at 1!! 
	std::vector<int> interested_landmark_id = full_sfm_landmarks;

	auto corr_pair = get_corresponding_pair(interested_landmark_id);
	std::vector<int> actually_assigned_landmark_id = corr_pair.first;
	std::vector<int> correspoding_indicies_on_model = corr_pair.second;

	dlib::array2d<dlib::rgb_pixel> img;
	load_image(img, img_file);
	auto full_landmarks = get_face_landmarks(img);

	std::vector< std::array<float,2> > image_points;
	for(int id : actually_assigned_landmark_id)
		image_points.push_back( full_landmarks[id-1] );

	std::array<float,12> camera_pos;
	std::vector<float> shape_coeff(pca_dimension(), 0.0f);

	for(int it=0; it<fitting_iter; ++it)
	{
		auto model_points = get_model_vertices(shape_coeff, correspoding_indicies_on_model);

		camera_pos = estimate_camera_by_POSIT(
			image_points, model_points, (float)img.nc(), (float)img.nr(), fov_y, posit_iter);

		shape_coeff = estimate_shape_coeff(
			camera_pos, image_points, correspoding_indicies_on_model, (float)img.nc(), (float)img.nr(), fov_y, regular_weight, active_dim);
	}

	auto V = get_model_vertices(shape_coeff);
	auto I = get_model_topology();
	auto N = get_average_noraml(V, I);
	auto mesh = create_TransparentMesh(V, I, N, I);
	auto model_points = get_model_vertices(shape_coeff, correspoding_indicies_on_model);

	OpenGLWindow win(img.nc(), img.nr());
	win.setViewMatrix( to_GL_view_matrix(camera_pos) );
	win.setProjectionMatrix( ogl::perspective(fov_y, (float)img.nc()/(float)img.nr()) );
	
	win.addObjectOnScene( create_FullScreenImage((unsigned char*)img.begin(), img.nc(), img.nr()) );
	win.addObjectOnScene( mesh );
	win.addObjectOnScene( create_FeaturePoints(image_points, model_points, (float)img.nc(), (float)img.nr()) );
	
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

