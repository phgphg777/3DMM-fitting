#pragma once
#include <vector>
#include <array>
#include <string>
#include "dlib/image_io.h"
#include "ogl/ogl_core.h"
#include "simple_mesh.h"


static ogl::mat4 to_GL_view_matrix(const std::array<float,12>& openCV_R_and_t) 
{
	ogl::mat4 view(1.0f);
	view(0,0) = openCV_R_and_t[0]; view(0,1) = openCV_R_and_t[1]; view(0,2) = openCV_R_and_t[2];
	view(1,0) = -openCV_R_and_t[3]; view(1,1) = -openCV_R_and_t[4]; view(1,2) = -openCV_R_and_t[5];
	view(2,0) = -openCV_R_and_t[6]; view(2,1) = -openCV_R_and_t[7]; view(2,2) = -openCV_R_and_t[8];
	view(0,3) = openCV_R_and_t[9]; view(1,3) = -openCV_R_and_t[10]; view(2,3) = -openCV_R_and_t[11];
	
	return view;
}


std::vector< std::array<float,2> > get_face_landmarks(
	const dlib::array2d<dlib::rgb_pixel>& img
);


std::pair< std::vector<int>, std::vector<int> > get_corresponding_pair(
	std::vector<int>& interested_landmark_id
);


int pca_dimension();


const std::vector<std::array<int, 3>>& get_model_topology();


std::vector< std::array<float, 3> > get_model_vertices(
	const std::vector<float>& coeff,
	const std::vector<int>& sub_indices = std::vector<int>()
);


std::array<float,12> estimate_camera_by_POSIT(
	const std::vector<std::array<float,2>>& image_points, 
	const std::vector<std::array<float,3>>& model_points, 
	float img_width, float img_height, float fov_y, int iter
);


std::vector<float> estimate_shape_coeff(
	const std::array<float, 12>& camera_pose,
	const std::vector<std::array<float, 2>>& image_points,
	const std::vector<int>& feature_indices,
	float img_width, float img_height, float fov_y,
	float lambda, int active_dim = -1
);


std::shared_ptr<OpenGLObject> create_TransparentMesh(
	const std::vector<std::array<float, 3>>& V,
	const std::vector<std::array<int, 3>>& VI,
	const std::vector<std::array<float, 3>>& N,
	const std::vector<std::array<int, 3>>& NI
);


std::shared_ptr<OpenGLObject> create_FullScreenImage(
	unsigned char* data, int width, int height
); 


std::shared_ptr<OpenGLObject> create_FeaturePoints(
	const std::vector<std::array<float, 2>>& img_points,
	const std::vector<std::array<float, 3>>& mesh_points,
	float width, float height
);


void inc_alpha(std::shared_ptr<OpenGLObject> mesh);
void dec_alpha(std::shared_ptr<OpenGLObject> mesh);


#define GLFW_RELEASE				0
#define GLFW_PRESS					1
#define GLFW_KEY_MINUS              45
#define GLFW_KEY_EQUAL              61

