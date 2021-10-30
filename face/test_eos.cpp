//#include "main.h"
//#include "Eigen\Dense"
//
//std::vector<int> full_sfm_landmarks =
//{ 9,18,19,
//20,21,22,23,24,25,26,27,28,29,
//30,31,32,33,34,35,36,37,38,39,
//40,41,42,43,44,45,46,47,48,49,
//50,51,52,53,54,55,56,57,58,59,
//60,   62,63,64,   66,67,68 };
//std::vector<int> key_landmarks =
//{ 37,40,43,46,
//32,36,
//49,55,
//9 };
//
//static const char* img_file =
////"data/images/a.bmp";
////"data/images/greedy_0_0.bmp";
////"data/images/jaw.bmp";
////"data/images/400x600.bmp";
////"data/images/500x700.bmp";
////"data/images/740x740.bmp";
//"data/images/jieum1.jpg";
//
//
//int main2(int argc, char* argv[])
//{
//	dlib::array2d<dlib::rgb_pixel> img;
//	load_image(img, img_file);
//	auto full_landmarks = get_face_landmarks(img);
//
//	//Note that ibug face landmarks start at 1!! 
//	std::vector<int> interested_landmark_id = key_landmarks;
//
//	auto corr_pair = get_corresponding_pair(interested_landmark_id);
//	std::vector<int> actually_assigned_landmark_id = corr_pair.first;
//	std::vector<int> correspoding_indicies_on_model = corr_pair.second;
//
//	std::vector< std::array<float, 2> > image_points;
//	for (int id : actually_assigned_landmark_id)
//		image_points.push_back(full_landmarks[id - 1]);
//
//	std::array<float, 12> camera_pos;
//	std::vector<float> shape_coeff(pca_dimension(), 0.0f);
//
//	auto model_points = get_model_vertices(shape_coeff, correspoding_indicies_on_model);
//
//	camera_pos = estimate_camera_by_POSIT(
//		image_points, model_points, (float)img.nc(), (float)img.nr(), fov_y);
//
//
//	// Estimate the camera (pose) from the 2D - 3D point correspondences
//	fitting::ScaledOrthoProjectionParameters pose =
//		fitting::estimate_orthographic_projection_linear(image_points, model_points, true, image.rows);
//	fitting::RenderingParameters rendering_params(pose, image.cols, image.rows);
//
//	// The 3D head pose can be recovered as follows:
//	const float yaw_angle = glm::degrees(glm::yaw(rendering_params.get_rotation()));
//	// and similarly for pitch and roll.
//
//	// Estimate the shape coefficients by fitting the shape to the landmarks:
//	const Eigen::Matrix<float, 3, 4> affine_from_ortho =
//		fitting::get_3x4_affine_camera_matrix(rendering_params, image.cols, image.rows);
//
//	const vector<float> fitted_coeffs = fitting::fit_shape_to_landmarks_linear(
//		morphable_model.get_shape_model(), affine_from_ortho, image_points, vertex_indices);
//
//}
//
//
//std::vector<float> fit_shape_to_landmarks_linear(
//	const morphablemodel::PcaModel& shape_model, Eigen::Matrix<float, 3, 4> affine_camera_matrix,
//	const std::vector<Eigen::Vector2f>& landmarks, const std::vector<int>& vertex_ids,
//	Eigen::VectorXf base_face = Eigen::VectorXf(), float lambda = 3.0f,
//	cpp17::optional<int> num_coefficients_to_fit = cpp17::optional<int>(),
//	cpp17::optional<float> detector_standard_deviation = cpp17::optional<float>(),
//	cpp17::optional<float> model_standard_deviation = cpp17::optional<float>())
//{
//	assert(landmarks.size() == vertex_ids.size());
//
//	using Eigen::VectorXf;
//	using Eigen::MatrixXf;
//
//	const int num_coeffs_to_fit = pca_dimension();
//	const int num_landmarks = static_cast<int>(landmarks.size());
//
//	if (base_face.size() == 0)
//	{
//		base_face = shape_model.get_mean();
//	}
//
//	// $\hat{V} \in R^{3N\times m-1}$, subselect the rows of the eigenvector matrix $V$ associated with the $N$ feature points
//	// And we insert a row of zeros after every third row, resulting in matrix $\hat{V}_h \in R^{4N\times m-1}$:
//	MatrixXf V_hat_h = MatrixXf::Zero(4 * num_landmarks, num_coeffs_to_fit);
//	int row_index = 0;
//	for (int i = 0; i < num_landmarks; ++i)
//	{
//		// In the paper, I'm not sure whether they use the orthonormal basis. It seems a bit messy/inconsistent even in the paper.
//		// Update PH 26.5.2014: I think the rescaled basis is fine/better!
//		const auto& basis_rows = shape_model.get_rescaled_pca_basis_at_point(vertex_ids[i]);
//		V_hat_h.block(row_index, 0, 3, V_hat_h.cols()) =
//			basis_rows.block(0, 0, basis_rows.rows(), num_coeffs_to_fit);
//		row_index += 4; // replace 3 rows and skip the 4th one, it has all zeros
//	}
//
//	// Form a block diagonal matrix $P \in R^{3N\times 4N}$ in which the camera matrix C (P_Affine, affine_camera_matrix) is placed on the diagonal:
//	Eigen::SparseMatrix<float> P(3 * num_landmarks, 4 * num_landmarks);
//	std::vector<Eigen::Triplet<float>> P_coefficients; // list of non-zeros coefficients
//	for (int i = 0; i < num_landmarks; ++i) { // Note: could make this the inner-most loop.
//		for (int x = 0; x < affine_camera_matrix.rows(); ++x) {
//			for (int y = 0; y < affine_camera_matrix.cols(); ++y) {
//				P_coefficients.push_back(
//					Eigen::Triplet<float>(3 * i + x, 4 * i + y, affine_camera_matrix(x, y)));
//			}
//		}
//	}
//	P.setFromTriplets(P_coefficients.begin(), P_coefficients.end());
//
//	// The variances: Add the 2D and 3D standard deviations.
//	// If the user doesn't provide them, we choose the following:
//	// 2D (detector) standard deviation: In pixel, we follow [1] and choose sqrt(3) as the default value.
//	// 3D (model) variance: 0.0f. It only makes sense to set it to something when we have a different variance
//	// for different vertices.
//	// The 3D variance has to be projected to 2D (for details, see paper [1]) so the units do match up.
//	const float sigma_squared_2D = std::pow(detector_standard_deviation.value_or(std::sqrt(3.0f)), 2) +
//		std::pow(model_standard_deviation.value_or(0.0f), 2);
//	// We use a VectorXf, and later use .asDiagonal():
//	const VectorXf Omega = VectorXf::Constant(3 * num_landmarks, 1.0f / sigma_squared_2D);
//	// Earlier, we set Sigma in a for-loop and then computed Omega, but it was really unnecessary:
//	// Sigma(i, i) = sqrt(sigma_squared_2D), but then Omega is Sigma.t() * Sigma (squares the diagonal) - so
//	// we just assign 1/sigma_squared_2D to Omega here.
//
//	// The landmarks in matrix notation (in homogeneous coordinates), $3N\times 1$
//	VectorXf y = VectorXf::Ones(3 * num_landmarks);
//	for (int i = 0; i < num_landmarks; ++i)
//	{
//		y(3 * i) = landmarks[i][0];
//		y((3 * i) + 1) = landmarks[i][1];
//		// y((3 * i) + 2) = 1; // already 1, stays (homogeneous coordinate)
//	}
//
//	// The mean, with an added homogeneous coordinate (x_1, y_1, z_1, 1, x_2, ...)^t
//	VectorXf v_bar = VectorXf::Ones(4 * num_landmarks);
//	for (int i = 0; i < num_landmarks; ++i)
//	{
//		v_bar(4 * i) = base_face(vertex_ids[i] * 3);
//		v_bar((4 * i) + 1) = base_face(vertex_ids[i] * 3 + 1);
//		v_bar((4 * i) + 2) = base_face(vertex_ids[i] * 3 + 2);
//		// v_bar((4 * i) + 3) = 1; // already 1, stays (homogeneous coordinate)
//	}
//
//	// Bring into standard regularised quadratic form with diagonal distance matrix Omega:
//	const MatrixXf A = P * V_hat_h; // camera matrix times the basis
//	const MatrixXf b = P * v_bar - y; // camera matrix times the mean, minus the landmarks
//	const MatrixXf AtOmegaAReg = A.transpose() * Omega.asDiagonal() * A +
//		lambda * Eigen::MatrixXf::Identity(num_coeffs_to_fit, num_coeffs_to_fit);
//	const MatrixXf rhs = -A.transpose() * Omega.asDiagonal() * b; // It's -A^t*Omega^t*b, but we don't need to
//																  // transpose Omega, since it's a diagonal
//																  // matrix, and Omega^t = Omega.
//
//																  // c_s: The 'x' that we solve for. (The variance-normalised shape parameter vector, $c_s =
//																  // [a_1/sigma_{s,1} , ..., a_m-1/sigma_{s,m-1}]^t$.)
//																  // We get coefficients ~ N(0, 1), because we're fitting with the rescaled basis. The coefficients are not
//																  // multiplied with their eigenvalues.
//	const VectorXf c_s = AtOmegaAReg.colPivHouseholderQr().solve(rhs);
//
//	return std::vector<float>(c_s.data(), c_s.data() + c_s.size());
//};
