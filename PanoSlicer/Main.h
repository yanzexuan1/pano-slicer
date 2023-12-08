#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <math.h>
#include <iostream>
#include <ctime>
#include <thread>
#include "ConfigParser.h"


using namespace std;
using namespace cv;
using namespace std;
using namespace PanoSlicer;

float M_PI = 3.14159265358979323846f;
float faceTransform[6][2] =
{
	{ 0, 0 },
	{ M_PI / 2,0 },
	{ M_PI, 0 },
	{ -M_PI / 2,0 },
	{ 0, -M_PI / 2 },
	{ 0, M_PI / 2 }
};

#define THREAD_NUMS 6