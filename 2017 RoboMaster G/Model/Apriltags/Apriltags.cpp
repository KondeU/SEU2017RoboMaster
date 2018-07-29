// Apriltags.cpp
// Version 2.0
// started by Deyou Kong, 2017-07-05
// Checked by Deyou Kong, 2017-07-05

#include <vector>
#include <utility>
#include <cstring>
#include <sstream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include <eigen3/Eigen/Dense>
using namespace Eigen;

#include "TagDetector.h"
#include "Tag16h5.h"
#include "Tag25h7.h"
#include "Tag25h9.h"
#include "Tag36h9.h"
#include "Tag36h11.h"
using namespace AprilTags;

#include "Apriltags.h"


//==================================================================

#include <cmath>

#ifndef PI
const double PI = 3.14159265358979323846;
#endif
const double TWOPI = 2.0*PI;

/**
* Normalize angle to be within the interval [-pi,pi].
*/
inline double standardRad(double t) {
	if (t >= 0.) {
		t = fmod(t + PI, TWOPI) - PI;
	}
	else {
		t = fmod(t - PI, -TWOPI) + PI;
	}
	return t;
}

/**
* Convert rotation matrix to Euler angles
*/
void wRo_to_euler(const Eigen::Matrix3d& wRo, double& yaw, double& pitch, double& roll) {
	yaw = standardRad(atan2(wRo(1, 0), wRo(0, 0)));
	double c = cos(yaw);
	double s = sin(yaw);
	pitch = standardRad(atan2(-wRo(2, 0), wRo(0, 0)*c + wRo(1, 0)*s));
	roll = standardRad(atan2(wRo(0, 2)*s - wRo(1, 2)*c, -wRo(0, 1)*s + wRo(1, 1)*c));
}

//==================================================================


CApriltags::CApriltags(string szTagFamily, int iWidth, int iHeight,
	double dFX, double dFY, double dPX, double dPY, double dTagSize)
	: tTagCodes(tagCodes16h5)
{
	pTagDetector = nullptr;

	this->iWidth  = iWidth;
	this->iHeight = iHeight;

	this->dFX = dFX;
	this->dFY = dFY;
	this->dPX = dPX;
	this->dPY = dPY;

	this->dTagSize = dTagSize;
	
	SetTagFamily(szTagFamily);
}

CApriltags::~CApriltags()
{
	if (pTagDetector)
	{
		delete pTagDetector;
	}
}

void CApriltags::SetTagFamily(string s)
{
	if (s == "tag16h5")
	{
		tTagCodes = tagCodes16h5;
	}
	else if (s == "tag25h7")
	{
		tTagCodes = tagCodes25h7;
	}
	else if (s == "tag25h9")
	{
		tTagCodes = tagCodes25h9;
	}
	else if (s == "tag36h9")
	{
		tTagCodes = tagCodes36h9;
	}
	else if (s == "tag36h11")
	{
		tTagCodes = tagCodes36h11;
	}
	else
	{
		tTagCodes = tagCodes16h5;
	}
	
	if (pTagDetector)
	{
		delete pTagDetector;
	}
	pTagDetector = new TagDetector(tTagCodes);
}

void CApriltags::Detect(cv::Mat mImg, vector<TTagInfo> & vTagInfo)
{
	vTagInfo.clear();

	vector<TagDetection> vDetections = pTagDetector->extractTags(mImg);

	for (int i = 0; i < vDetections.size(); i++)
	{
		Vector3d eTranslation;
		Matrix3d eRotation;
		vDetections[i].getRelativeTranslationRotation(dTagSize,
			dFX, dFY, dPX, dPY, eTranslation, eRotation);

		Matrix3d eF;
		eF <<
			1,  0, 0,
			0, -1, 0,
			0,  0, 1;
		Matrix3d eFixedRot = eF * eRotation;

		double dYaw, dPitch, dRoll;
		wRo_to_euler(eFixedRot, dYaw, dPitch, dRoll);

		TTagInfo tTagInfo;

		tTagInfo.iTagID = vDetections[i].id;

		tTagInfo.dDistance = eTranslation.norm();

		tTagInfo.dX = eTranslation(0);
		tTagInfo.dY = eTranslation(1);
		tTagInfo.dZ = eTranslation(2);

		tTagInfo.dYaw   = dYaw;
		tTagInfo.dPitch = dPitch;
		tTagInfo.dRoll  = dRoll;
		
		tTagInfo.tCenter.fX = vDetections[i].cxy.first;
		tTagInfo.tCenter.fY = vDetections[i].cxy.second;

		for (int iCornerIndex = 0; iCornerIndex < 4; iCornerIndex++)
		{
			tTagInfo.tCorner[iCornerIndex].fX = vDetections[i].p[iCornerIndex].first;
			tTagInfo.tCorner[iCornerIndex].fY = vDetections[i].p[iCornerIndex].second;
		}

		vTagInfo.push_back(tTagInfo);
	}
}

void CApriltags::SetExternalTag(string szTagProp, int iTagCount, const unsigned long long ullTag[])
{
	if (szTagProp == "tag16h5")
	{
		tTagCodes = TagCodes(16, 5, ullTag, iTagCount);
	}
	else if (szTagProp == "tag25h7")
	{
		tTagCodes = TagCodes(25, 7, ullTag, iTagCount);
	}
	else if (szTagProp == "tag25h9")
	{
		tTagCodes = TagCodes(25, 9, ullTag, iTagCount);
	}
	else if (szTagProp == "tag36h9")
	{
		tTagCodes = TagCodes(36, 9, ullTag, iTagCount);
	}
	else if (szTagProp == "tag36h11")
	{
		tTagCodes = TagCodes(36, 11, ullTag, iTagCount);
	}
	else
	{
		int iGrid = 16, iHammingDist = 5;
		stringstream ss;
		ss << szTagProp;
		ss >> iGrid >> iHammingDist;
		tTagCodes = TagCodes(iGrid, iHammingDist, ullTag, iTagCount);
	}
	
	if (pTagDetector)
	{
		delete pTagDetector;
	}
	pTagDetector = new TagDetector(tTagCodes);
}
