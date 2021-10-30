#include <vector>
#include <array>
#include <fstream>
#include "Eigen/Dense"
#include "cereal/types/array.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/archives/binary.hpp"
#include "cereal_types_eigen.hpp"
#define DEGREE ((float)(3.141592653589793238463/180.0))

//namespace paf {


static const char* filename = "data/sfm/sfm_shape_3448_singular.bin";


class face_model
{
	Eigen::VectorXf _mean;
	Eigen::VectorXf _sigma;	// singular values
    Eigen::MatrixXf _U;		// orthonormal pca basis
    Eigen::MatrixXf _US;	// U * diag(sigma)

	std::vector<std::array<int, 3>> _topology;
	
	face_model()
	{
		std::ifstream is(filename, std::ios::binary);
		if (!is)
			throw std::runtime_error("Error opening given file: " + std::string(filename));

		cereal::BinaryInputArchive in_archive(is);
		in_archive(_mean, _U, _sigma, _topology);
		_US = _U * _sigma.asDiagonal();
	}

public:
	static face_model& singleton() {
		static face_model unique_instance;
		return unique_instance;
	}

	const Eigen::VectorXf& mean() const
		{ return _mean; }
	const Eigen::VectorXf& sigma() const
		{ return _sigma; }
	const Eigen::MatrixXf& U() const
		{ return _U; }
	const Eigen::MatrixXf& US() const
		{ return _US; }
	const std::vector<std::array<int, 3>>& topology() const
		{ return _topology; }

	int dim_data() const
		{ return (int) _mean.size(); }
	int num_vertices() const 
		{ return dim_data() / 3; }
	int num_basis() const 
		{ return (int) _U.cols(); }

	Eigen::VectorXf shape_from_coeff(const std::vector<float>& coeff) const
	{
		assert((int)coeff.size() == num_basis());
		return _mean + _US * Eigen::Map<Eigen::VectorXf>((float*)coeff.data(), coeff.size());
	}

	std::vector< std::array<float, 3> > get_vertices(
		const std::vector<float>& coeff) const
	{
		Eigen::VectorXf shape = shape_from_coeff(coeff);

		int num = num_vertices(); 
		std::vector< std::array<float, 3> > V(num);
		for (int i = 0; i < num; ++i)
			V[i] = { shape[3*i], shape[3*i+1], shape[3*i+2] };
		
		return V;
	}

	std::vector< std::array<float, 3> > get_vertices(
		const std::vector<float>& coeff,
		const std::vector<int>& sub_indices) const
	{
		Eigen::VectorXf shape = shape_from_coeff(coeff);

		int num = (int) sub_indices.size(); 
		std::vector< std::array<float, 3> > V(num);
		for (int i = 0; i < num; ++i) {
			int idx = sub_indices[i];
			V[i] = { shape[3*idx], shape[3*idx+1], shape[3*idx+2] };
		}
			
		return V;
	}
};



int pca_dimension()
{
	return face_model::singleton().num_basis();
}


std::vector< std::array<float, 3> > get_model_vertices(
	const std::vector<float>& coeff,
	const std::vector<int>& sub_indices = std::vector<int>())
{
	if(sub_indices.empty())
		return face_model::singleton().get_vertices(coeff);
	else
		return face_model::singleton().get_vertices(coeff, sub_indices);
}


const std::vector<std::array<int, 3>>& get_model_topology()
{ 
	return face_model::singleton().topology(); 
}


using std::cout;
using std::endl;
enum FIT_MODE {
	SCALED_ORTHO, DIRECT_LINEAR
};
FIT_MODE fit_mode = DIRECT_LINEAR;

std::vector<float> estimate_shape_coeff(
	const std::array<float, 12>& camera_pose,
	const std::vector<std::array<float, 2>>& image_points,
	const std::vector<int>& vertex_indices,
	float img_width, float img_height, float fov_y,
	float lambda, int active_dim = -1)
{
	auto model = face_model::singleton();

	int numf = (int) vertex_indices.size();
	if( numf != (int) image_points.size() )
		throw std::runtime_error("image_points.size() != feature_indices.size()");

	if(active_dim<0 || active_dim>model.US().cols()) 
		active_dim = model.US().cols();

	Eigen::Matrix3f R;
	Eigen::Vector3f t;
	Eigen::RowVector3f I, J, K;
	I << camera_pose[0], camera_pose[1], camera_pose[2]; 
	J << camera_pose[3], camera_pose[4], camera_pose[5]; 
	K << camera_pose[6], camera_pose[7], camera_pose[8];
	t << camera_pose[9], camera_pose[10], camera_pose[11];
	R << I, J, K;

	float half_w = img_width * 0.5f;
	float half_h = img_height * 0.5f;
	float tan = tanf(fov_y*0.5f*DEGREE);
	
	Eigen::MatrixXf u_v_(numf, 2);
	for(int i = 0; i < numf; ++i) {
		u_v_(i, 0) = (image_points[i][0] - half_w) / half_h * tan;
		u_v_(i, 1) = (image_points[i][1] - half_h) / half_h * tan;
	}

	Eigen::MatrixXf PUS(3*numf, active_dim);	// Projection * US
    for (int i = 0; i < numf; ++i)
		PUS.block(3*i, 0, 3, active_dim) = model.US().block(3*vertex_indices[i], 0, 3, active_dim);

	Eigen::MatrixXf C = Eigen::MatrixXf::Zero(2*numf, 3*numf);
	Eigen::VectorXf b(2*numf);
	
	switch(fit_mode)
	{
	case DIRECT_LINEAR:
		for(int i = 0; i < numf; ++i) {
			C.block<2,3>(2*i,3*i) << (I - u_v_(i,0)*K), (J - u_v_(i,1)*K);
		}
		for(int i = 0; i < numf; ++i) {
			auto mean_i = model.mean().segment<3>(3*vertex_indices[i]);
			b(2*i  ) = u_v_(i,0)*t.z() - t.x() - I*mean_i
					+ u_v_(i,0)*K*mean_i;
			b(2*i+1) = u_v_(i,1)*t.z() - t.y() - J*mean_i
					+ u_v_(i,1)*K*mean_i;
		}
		break;

	case SCALED_ORTHO:
		for(int i = 0; i < numf; ++i) {
			C.block<2,3>(2*i,3*i) << I, J;
		}
		for(int i = 0; i < numf; ++i) {
			auto mean_i = model.mean().segment<3>(3*vertex_indices[i]);
			b(2*i  ) = u_v_(i,0)*t.z() - t.x() - I*mean_i;
			b(2*i+1) = u_v_(i,1)*t.z() - t.y() - J*mean_i;
		}
		break;
	}

	Eigen::MatrixXf A = C*PUS;		// matrix of 2*numf by pca_dim
	
	Eigen::MatrixXf _A_ = A.transpose()*A;
	_A_.diagonal().array() += lambda;
	Eigen::VectorXf x = _A_.colPivHouseholderQr().solve(A.transpose()*b);
	//cout<< "Least Square Error: " << (A*x-b).norm() << endl;

	if(true) // for debug
	{ 
		printf("\n--------------- Shape Esimation Stage ---------------\n");

		Eigen::VectorXf coeff = Eigen::VectorXf::Zero(model.US().cols());
		coeff.head(active_dim) = x;
		Eigen::VectorXf s = model.mean() + model.US() * coeff;

		float error_sum = 0.0f;
		for (int i = 0; i < numf; ++i)
		{
			Eigen::Vector3f p = s.segment<3>(3*vertex_indices[i]);
			p = R*p + t;
			float u = ((p.x()/p.z()) * half_h / tan) + half_w;
			float v = ((p.y()/p.z()) * half_h / tan) + half_h;

			printf("feature(%d) \n", i);
			printf("p.z - t.z  : %f \n", p.z()-t.z());
			printf("target     : %f,   %f \n", image_points[i][0], image_points[i][1]);
			printf("perspective: %f,   %f \n", u, v);
			float error = sqrt((image_points[i][0]-u)*(image_points[i][0]-u) 
				+ (image_points[i][1]-v)*(image_points[i][1]-v));
			printf("error      : %f \n", error);
			error_sum += error;
		}
		printf("Error Sum  : %f \n", error_sum);
	}

	std::vector<float> res(x.data(), x.data() + x.size());
	res.resize(model.US().cols(), 0.0f);
	return res;
}




//}; // namespace paf