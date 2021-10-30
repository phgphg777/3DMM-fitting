#include <vector>
#include <array>
#include <iostream>
#include "Eigen/Dense"
using namespace std;
using namespace Eigen;

void test_eigen()
//void main()
{
	Eigen::MatrixXf v(3,3);

	v<<1,2,3,4,5,6,7,8,9;

	cout << (v.diagonal().array() += 10);

	//cout<< v <<std::endl;
	
}