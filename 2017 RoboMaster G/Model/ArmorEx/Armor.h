// Armor.h
// Version 2.0
// Started by Deyou Kong, 2017-07-06
// Checked by Deyou Kong, 2017-07-10
// Updated by Deyou Kong, 2017-08-02

#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#define DEBUG_ARMOR true
#define DEBUG_ARMOROUTPUT true

class CArmor
{
public:

	void FindArmor(cv::Mat mImg, cv::Point & tArmor);
	void FindLight(cv::Mat mImg, std::vector<cv::Point> & vLight);
	
	void FindArmorBase(cv::Mat mImg, cv::Point & tArmor);
	
private:

	static int iThresholdR;
	static int iThresholdG;
	static int iThresholdB;

	static int iMorphologyKenelR;
	static int iMorphologyKenelG;
	static int iMorphologyKenelB;

	static size_t uiContoursPixelsLimitLarge;
	static size_t uiContoursPixelsLimitSmall;

	static double dContoursRectAngleLimit;
	static int iContoursRectMaxSideDiffLimit;
	static int iContoursRectMinSideDiffLimit;
	static int iContoursRectAreaSizeDiffLimit;
	static double dContoursRectAspectRatioLimitLarge;
	static double dContoursRectAspectRatioLimitSmall;
	
	static double dArmorRectDiffLimit;
	
	static int iArmorRectMaxSizeLimitLarge;
	static int iArmorRectMaxSizeLimitSmall;
	static int iArmorRectMinSizeLimitLarge;
	static int iArmorRectMinSizeLimitSmall;
};
