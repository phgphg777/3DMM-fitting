#include "dlib/gui_widgets.h"
#include "dlib/image_io.h"
#include "dlib/image_transforms/draw.h"
#include "dlib/image_processing/frontal_face_detector.h"
#include "dlib/image_processing/shape_predictor.h"
using namespace dlib;


static full_object_detection get_face_landmarks(const dlib::array2d<rgb_pixel>& img)
{
	auto detector = get_frontal_face_detector();
	std::vector<rectangle> detections = detector(img);

	shape_predictor face_predictor;
	deserialize("dlib/data/shape_predictor_68_face_landmarks.dat") >> face_predictor;

	full_object_detection face_landmarks = face_predictor(img, detections[0]);

	return face_landmarks;
}


void test_dilb()
//void main()
{
	std::string imagename = "data/images/jieum1.jpg";

	try
	{
		dlib::array2d<rgb_pixel> img;
		load_image(img, imagename);
		//pyramid_up(img);

		auto face_landmarks = get_face_landmarks(img);
		for (unsigned i = 0; i < face_landmarks.num_parts(); ++i)
			draw_solid_circle(img, face_landmarks.part(i), 1.5, rgb_pixel(0, 255, 0));

		image_window win;
		win.set_image(img);
		//win.wait_until_closed();
		//return std::move(win);
	}
	catch (std::exception& e)
	{
		std::cout << "\nexception thrown!" << std::endl;
		std::cout << e.what() << std::endl;
	}
}
