#include "calib.hpp"
#include <stdlib.h>
#include <iostream>

const std::string conf_file = "parametersOfCamera/selfMakeParam1912071820.xml";
const std::string target_file = "parametersOfCamera/parameters1912081958.xml";
int main() {
	int choice;
	stereo_calibrator::init_mode calib_style;
	while (true) {
		std::cout << "选择标定方式：1.VS代码标定    2.matlab其余参数补足" << std::endl;
		std::cin >> choice;
		if (choice == 1) {
			calib_style = stereo_calibrator::init_mode::CALIBRATION;
			break;
		}
		else if (choice == 2) {
			calib_style = stereo_calibrator::init_mode::MATLAB_CALIB;
			break;
		}
		else continue;
	}

	stereo_calibrator calibrator(conf_file, calib_style);
	calibrator.save_params(target_file);
	std::cout << "finished..." << std::endl;
	system("pause");
	return 0;
}