// Camera.h
// Version 2.0
// started by Deyou Kong, 2017-07-02
// Checked by Deyou Kong, 2017-07-02
// Updated by Deyou Kong, 2017-07-20

#pragma once

#include <opencv2/opencv.hpp>

//#define CAMERA_ENABLE_THREAD // Use muti-thread to get camera data

class CCamera // Exist one instance only
{
public:

	CCamera() : mCamRaw(480, 640, CV_8UC3), mCamDec(240, 320, CV_8UC3) { };
	
	bool Grab();
	
	inline cv::Mat GetRaw() { return mCamRaw; };
	inline cv::Mat GetDec() { return mCamDec; };
	
	bool SetExposure(bool bAuto, int iExposure);

private:

	cv::Mat mCamRaw; // 640 * 480
	cv::Mat mCamDec; // 320 * 240
};
