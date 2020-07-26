/*
* test digits OCR
*
* @author  Nan Li
* @since   July 2020
*/
#pragma once

#include <fstream>
#include <iostream>
#include <algorithm>
#include <Windows.h>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

bool start_cognize(Mat src, std::string _str_file_name);