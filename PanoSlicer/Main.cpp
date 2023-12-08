#pragma once

#include "Main.h"

struct ParamThread {
	CConfigParser* config;
	Mat in;
	Mat face;
	int width;
	int height;
};

inline void createTransation(ParamThread param, int faceId)
{
	// 获取图片的行列数量
	float inWidth = (float)param.in.cols;
	float inHeight = (float)param.in.rows;
	// 分配图的 x, y 轴
	Mat mapx(param.height, param.width, CV_32F);
	Mat mapy(param.height, param.width, CV_32F);
	// 计算相邻 ak 和相反 an 的三角形张成球体中心
	const float an = sin(M_PI / 4);
	const float ak = cos(M_PI / 4);

	const float ftu = faceTransform[faceId][0];
	const float ftv = faceTransform[faceId][1];

	// 对于每个图像计算相应的源坐标
	for (int y = 0; y < param.height; y++)
	{
		for (int x = 0; x < param.width; x++)
		{
			// 将坐标映射在平面上
			float nx = (float)y / (float)param.height - 0.5f;
			float ny = (float)x / (float)param.width - 0.5f;
			nx *= 2;
			ny *= 2;
			// Map [-1, 1] plane coord to [-an, an]
			// thats the coordinates in respect to a unit sphere 
			// that contains our box. 
			nx *= an;
			ny *= an;
			float u, v;
			// Project from plane to sphere surface.
			if (ftv == 0)
			{
				// Center faces
				u = atan2(nx, ak);
				v = atan2(ny * cos(u), ak);
				u += ftu;
			}
			else if (ftv > 0)
			{
				// Bottom face 
				float d = sqrt(nx * nx + ny * ny);
				v = M_PI / 2 - atan2(d, ak);
				u = atan2(ny, nx);
			}
			else
			{
				// Top face
				//cout << "aaa";
				float d = sqrt(nx * nx + ny * ny);
				v = -M_PI / 2 + atan2(d, ak);
				u = atan2(-ny, nx);
			}
			// Map from angular coordinates to [-1, 1], respectively.
			u = u / (M_PI);
			v = v / (M_PI / 2);
			// Warp around, if our coordinates are out of bounds. 
			while (v < -1)
			{
				v += 2;
				u += 1;
			}
			while (v > 1)
			{
				v -= 2;
				u += 1;
			}

			while (u < -1)
			{
				u += 2;
			}
			while (u > 1) {
				u -= 2;
			}

			// Map from [-1, 1] to in texture space
			u = u / 2.0f + 0.5f;
			v = v / 2.0f + 0.5f;

			u = u * (inWidth - 1);
			v = v * (inHeight - 1);


			mapx.at<float>(x, y) = u;
			mapy.at<float>(x, y) = v;
		}
	}

	// Recreate output image if it has wrong size or type. 
	if (param.face.cols != param.width || param.face.rows != param.height || param.face.type() != param.in.type())
	{
		param.face = Mat(param.width, param.height, param.in.type());
	}

	// Do actual  using OpenCV's remap
	remap(param.in, param.face, mapx, mapy, INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0));

	// To define directions, we should assume a character is standing in the scene,
	// and is facing to a certain direction.
	// The source panorama is taken from UE, in which, the character looks to +x direction, which is front
	// Thus, in UE, right-handed coordinate and z-up:
	// x: front, -x: back
	// y: right, -y: left
	// z: up, -z: down
	// Note that, in UE/Three.JS/Revit, right/left isn't consistent with the navigation cube!
	// For a given panorama, it's gonna be sliced to 6 images
	//                    --------
	//                    |  up  |
	//          -----------------------------
	// 1/2 back |  | left |front | right |  | 1/2 back
	//          -----------------------------
	//                    | down |
	//                    |______|
	const std::map<int, std::string> nameMap = {
		{ 0, "front.jpg" },
		{ 1, "right.jpg" },
		{ 2, "back.jpg" },
		{ 3, "left.jpg" },
		{ 4, "top.jpg" }, // up
		{ 5, "bottom.jpg" }, // down
	};
	if (faceId < 0 || faceId > 5)
	{
		cout << "Bad faceId: " << faceId;
		return;
	}
	else if (faceId == 4)
	{
		//transpose(param.face, param.face);
		rotate(param.face, param.face, ROTATE_90_CLOCKWISE);
	}
	else if (faceId == 5)
	{
		transpose(param.face, param.face);
		flip(param.face, param.face, 0);
	}
	imwrite(nameMap.at(faceId), param.face);
}


int main()
{
	// read config file
	CConfigParser* pConParser = new CConfigParser();
	bool config = pConParser->Parser("config.ini");
	if (!config)
	{
		cout << "Failed to parse config.ini!" << endl;
		return 1;
	}

	clock_t startTime, endTime;

	bool hasSection = pConParser->HasSection("pano");
	if (hasSection == true)
	{
		vector<string> vecKeys;
		pConParser->GetKeys("pano", vecKeys);
		// cout << vecKeys.size() << endl; // 节点里的数据数
		if (vecKeys.size() == 0)
		{
			cout << "krpano child element is empty" << endl;
			return 0;
		}

		string width = pConParser->GetConfig("pano", "width");
		string height = pConParser->GetConfig("pano", "height");
		string image_path = pConParser->GetConfig("pano", "image_path");

		if (width == "" || height == "" || image_path == "")
		{
			cout << "config value is empty" << endl;
			return 0;
		}

		// start timer
		startTime = clock();

		cv::Mat srcimage = cv::imread(image_path);
		cv::Mat resultImage;

		struct ParamThread cubeParams;
		cubeParams.config = pConParser;
		cubeParams.in = srcimage;
		cubeParams.face = resultImage;
		cubeParams.width = stoi(width);
		cubeParams.height = stoi(height);

		thread targs[THREAD_NUMS];

		for (int i = 0; i < THREAD_NUMS; i++)
		{
			// creates worker thread
			targs[i] = thread(createTransation, std::ref(cubeParams), i);
		}

		for (int i = 0; i < THREAD_NUMS; i++)
		{
			if (targs[i].joinable())
				targs[i].join();
		}

		// job done
		endTime = clock();
		cout << "Totally cost " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	}
	else
	{
		cout << "krpano section doesn't exist" << endl;
		return 0;
	}

	return 0;
}
