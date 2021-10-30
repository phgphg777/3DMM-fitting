#include "cereal/types/array.hpp"
#include "cereal/types/vector.hpp"
#include "cereal_types_eigen.hpp"
#include "cereal/archives/binary.hpp"
#include "Eigen/Core"
#include <fstream>
#include <sstream>
using namespace std;


static std::string file1 = "data/sfm/sfm_shape_3448_eigen.bin";
static std::string file2 = "data/sfm/sfm_shape_3448_singular.bin";
void test_cereal1()
//void main()
{
	std::ifstream is1(file1, std::ios::binary);
	std::ifstream is2(file2, std::ios::binary);
	
	if (!is1 || !is2)
		throw std::runtime_error("Error opening!");

	cereal::BinaryInputArchive archive1(is1);
	cereal::BinaryInputArchive archive2(is2);

	Eigen::VectorXf m1, m2;
	Eigen::MatrixXf u1, u2;
	Eigen::VectorXf s1, s2;
	//std::vector<std::array<int, 3>> m4;

	archive1(m1, u1, s1);
	archive2(m2, u2, s2);

	//cout<<s1.transpose()<<endl<<endl;
	//cout<<s2.transpose()<<endl<<endl;

	//cout<<m1.transpose()<<endl<<endl;

	//Eigen::VectorXf v = m3.array().sqrt();


}


static std::string in_filename = "data/sfm/sfm_shape_3448.bin";
static std::string out_filename = "data/sfm/sfm_shape_only_3448.bin";
void test_cereal2()
//void main()
{
	std::ifstream infile(in_filename, std::ios::binary);
	std::ofstream outfile(out_filename, std::ios::binary);
	
	if (!infile)
		throw std::runtime_error("Error opening given file: " + in_filename);

	cereal::BinaryInputArchive in_archive(infile);
	cereal::BinaryOutputArchive out_archive(outfile);

	int num;
	Eigen::VectorXf m1;
	Eigen::MatrixXf m2;
	Eigen::VectorXf m3;
	std::vector<std::array<int, 3>> m4;

	in_archive(num, m1, m2, m3, m4);

	Eigen::VectorXf v = m3.array().sqrt();
	out_archive(m1, m2, v, m4);

	
}

void test_cereal3()
{
	std::stringstream ss;

	{
		cereal::BinaryOutputArchive out_archive(ss);

		Eigen::Matrix3f m;
		//m.resize(3,4);
		m.setRandom();

		Eigen::VectorXf v;
		v.resize(5);
		v.setRandom();

		std::cout << m << std::endl << std::endl;
		std::cout << v << std::endl << std::endl;
		
		out_archive(m,v);
	}

	
	Eigen::MatrixXf m;
	cereal::BinaryInputArchive in_archive(ss);
	cereal::BinaryInputArchive in_archive2(ss);

	in_archive(m);
	std::cout << m << std::endl << std::endl;

	in_archive2(m);
	std::cout << m << std::endl << std::endl;
}