#include <vector>
#include <array>
#include "dlib/image_processing/frontal_face_detector.h"
#include "dlib/image_processing/shape_predictor.h"


std::vector< std::array<float,2> > get_face_landmarks(
	const dlib::array2d<dlib::rgb_pixel>& img)
{
	auto face_detector = dlib::get_frontal_face_detector();
	std::vector<dlib::rectangle> face_areas = face_detector(img);

	static dlib::shape_predictor face_predictor;
	static bool first = true;
	if (first)
	{
		dlib::deserialize("dlib/data/shape_predictor_68_face_landmarks.dat") >> face_predictor;
		first = false;
	}
	dlib::full_object_detection face_landmarks = face_predictor(img, face_areas[0]);

	std::vector< std::array<float,2> > res(face_landmarks.num_parts());
	for (int i = 0; i < (int)res.size(); ++i) {
		auto p = face_landmarks.part(i);
		res[i] = { (float)p(0), (float)p(1) };
	}

	return res;
}
