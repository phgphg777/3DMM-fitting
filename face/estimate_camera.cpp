#include <vector>
#include <array>
#include <iostream>
#include "Eigen/Dense"
#define DEGREE ((float)(3.141592653589793238463/180.0))


std::array<float,12> estimate_camera_by_POSIT(
	const std::vector<std::array<float,2>>& image_points, 
	const std::vector<std::array<float,3>>& model_points, 
	float img_width, float img_height, float fov_y, int iter)
{
	int num = (int) image_points.size();
	if( num != (int) model_points.size() )
		throw std::runtime_error("image_points.size() != model_points.size()");
	if( num < 4 )
		throw std::runtime_error("The number of feateures must be more than 4.");
	
	float half_w = img_width * 0.5f;
	float half_h = img_height * 0.5f;
	float tan = tanf(fov_y*0.5f*DEGREE);

	Eigen::MatrixXf img_points(num, 2);
	for (int i = 0; i < num; ++i) {
		img_points(i, 0) = (image_points[i][0] - half_w) / half_h * tan;
		img_points(i, 1) = (image_points[i][1] - half_h) / half_h * tan;
	}

	Eigen::Vector3f to_center(0,0,0);
	bool centering = false;
	if(centering)
	{
		std::array<float,3> model_avr = { 0, 0, 0 };
		std::array<float,3> model_min = { model_points[0][0], model_points[0][1], model_points[0][2] };
		std::array<float,3> model_max = model_min;
		float mun = 1.0f / (float)num;
		for (int i = 0; i < num; ++i) {
			for (int j = 0; j < 3; ++j) {
				model_avr[j] += (float)model_points[i][j] * mun;
				if( model_min[j] > (float)model_points[i][j] )
					model_min[j] = (float)model_points[i][j];
				if( model_max[j] < (float)model_points[i][j] )
					model_max[j] = (float)model_points[i][j];
			}
		}
		to_center[0] = ((model_min[0] + model_max[0]) * 0.5f);
		to_center[1] = ((model_min[1] + model_max[1]) * 0.5f);
		to_center[2] = model_avr[2];
	}

	Eigen::MatrixXf mdl_points(num, 4);
	for (int i = 0; i < num; ++i) {
		mdl_points(i, 0) = model_points[i][0] - to_center[0];
		mdl_points(i, 1) = model_points[i][1] - to_center[1];
		mdl_points(i, 2) = model_points[i][2] - to_center[2];
		mdl_points(i, 3) = 1.0f;
	}

	auto A = mdl_points.colPivHouseholderQr();
	
	Eigen::Matrix3f R;
	Eigen::Vector3f t;
	
	Eigen::VectorXf correction(num);
	correction.setOnes();

	for(int it=0; it<=iter; ++it)
	{
		Eigen::MatrixXf b = correction.asDiagonal() * img_points;
		Eigen::MatrixXf x = A.solve(b);

		Eigen::Vector3f I = x.block<3,1>(0,0);
		Eigen::Vector3f J = x.block<3,1>(0,1);
		float s1 = I.norm();
		float s2 = J.norm();
		float s = (s1 + s2) / 2.0f;
		//printf("Iter %d: s1=%f, s2=%f \n", it, s1, s2);
		
		Eigen::Vector3f i_ = I/s1;
		Eigen::Vector3f j_ = J/s2;
		Eigen::Vector3f K = i_.cross(j_);

		if(true) {
			Eigen::Matrix3f R_;
			R_.transpose() << i_, j_, K;
			Eigen::JacobiSVD<Eigen::Matrix3f, Eigen::NoQRPreconditioner> svd(R_, Eigen::ComputeFullU | Eigen::ComputeFullV);
			Eigen::Matrix3f U = svd.matrixU();
			Eigen::Matrix3f V = svd.matrixV();
			R = U * V.transpose();
		}else{
			Eigen::Vector3f k_ = K.normalized();
			j_ = k_.cross(i_);
			R.transpose() << i_, j_, k_;
		}
	
		t = (1/s) * Eigen::Vector3f(x(3,0), x(3,1), 1.f);

		for (int i = 0; i < num; ++i) {
			float delta_z = R.row(2) .dot (mdl_points.row(i).head(3));
			correction[i] = (t.z() + delta_z) / t.z();
		}
		
		if(true) // for debug
		{ 
			if(it==0)
				printf("\n--------------- Camera Pose Esimation Stage ---------------\n");
			float error_sum = 0.0f;
			for (int i = 0; i < num; ++i)
			{
				Eigen::Vector3f p = {mdl_points(i,0), mdl_points(i,1), mdl_points(i,2)};
				p = R*p + t;
				float u = ((p.x()/p.z()) * half_h / tan) + half_w;
				float v = ((p.y()/p.z()) * half_h / tan) + half_h;

				if(i==0)
					printf("\n||||||| Iteration %d |||||||\n", it);
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
	}
	
	t -= R * to_center;

	return { 
		R(0,0), R(0,1), R(0,2), 
		R(1,0), R(1,1), R(1,2), 
		R(2,0), R(2,1), R(2,2), 
		t(0), t(1), t(2)
	};
}

