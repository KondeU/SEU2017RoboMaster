// Apriltags.h
// Version 2.0
// started by Deyou Kong, 2017-07-05
// Checked by Deyou Kong, 2017-07-05

#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#include "TagDetector.h"

struct TTagInfo
{
	int iTagID;

	double dDistance;
	double dX, dY, dZ;
	double dYaw, dPitch, dRoll;

	struct TPoint
	{
		float fX, fY;
	} tCenter, tCorner[4];
	
	// Corner
	// tCorner[3] ---- tCorner[2]
	//     |               |
	//     |     TAGS      |
	//     |               |
	// tCorner[0] ---- tCorner[1]	
};

class CApriltags
{
public:

	CApriltags(string szTagFamily, int iWidth, int iHeight,
		double dFX, double dFY, double dPX, double dPY, double dTagSize);
	~CApriltags();

	void SetTagFamily(string s);
	void Detect(cv::Mat mImg, std::vector<TTagInfo> & vTagInfo); // Make sure mImg is gray image

	void SetExternalTag(string szTagProp, int iTagCount, const unsigned long long ullTag[]);

private:

	AprilTags::TagDetector * pTagDetector;
	AprilTags::TagCodes tTagCodes;

	int iWidth;  // Image width in pixels
	int iHeight; // Image height in pixels

	double dFX;  // Camera focal length x in pixels
	double dFY;  // Camera focal length y in pixels
	double dPX;  // Camera principal point x in pixels
	double dPY;  // Camera principal point y in pixels

	double dTagSize; // Apriltag side length in meters of square black frame
};
