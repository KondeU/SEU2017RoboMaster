// Armor.cpp
// Version 2.0
// Started by Deyou Kong, 2017-07-06
// Checked by Deyou Kong, 2017-07-10
// Updated by Deyou Kong, 2017-07-25

#include <vector>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include "Armor.h"

int CArmor::iThresholdR = 220;
int CArmor::iThresholdG = 220;
int CArmor::iThresholdB = 220;
int CArmor::iMorphologyKenelR = 3;
int CArmor::iMorphologyKenelG = 3;
int CArmor::iMorphologyKenelB = 3;

size_t CArmor::uiContoursPixelsLimitLarge = 300;
size_t CArmor::uiContoursPixelsLimitSmall = 30;

double CArmor::dContoursRectAngleLimit = 7;
int CArmor::iContoursRectMaxSideDiffLimit = 15;
int CArmor::iContoursRectMinSideDiffLimit = 15;
int CArmor::iContoursRectAreaSizeDiffLimit = 35;
double CArmor::dContoursRectAspectRatioLimitLarge = 10;
double CArmor::dContoursRectAspectRatioLimitSmall = 3;

double CArmor::dArmorRectDiffLimit = 0.1;

int CArmor::iArmorRectMaxSizeLimitLarge = 20;
int CArmor::iArmorRectMaxSizeLimitSmall = 100;
int CArmor::iArmorRectMinSizeLimitLarge = 15;
int CArmor::iArmorRectMinSizeLimitSmall = 75;
// Standard armor size: 230 * 127

void DrawRotatedRect(Mat & mImg, RotatedRect cRotatedRect, int iDrawStrength)
{
	iDrawStrength = min(255, max(iDrawStrength, 0));

	Scalar tDraw(iDrawStrength);
	
	if (mImg.channels() == 3)
	{
		tDraw = Scalar(0, 0, iDrawStrength);
	}

	Point2f tCorner[4];
	cRotatedRect.points(tCorner);
	
	for (int i = 0; i < 4; i++)
	{
		line(mImg, tCorner[i], tCorner[(i + 1) % 4], tDraw);
	}
	circle(mImg, tCorner[0], 2, tDraw);
}

void CArmor::FindArmor(Mat mImg, Point & tArmor)
{
	tArmor = Point(-1, -1);
	
	vector<Point> vLight;
	FindLight(mImg, vLight);
	
	/*if (vLight.size() < 4)
	{
		return;
	}

	double dDifference = 1;
	for (size_t ui1 = 0;       ui1 < vLight.size(); ui1++)
	for (size_t ui2 = ui1 + 1; ui2 < vLight.size(); ui2++)
	for (size_t ui3 = ui2 + 1; ui3 < vLight.size(); ui3++)
	for (size_t ui4 = ui3 + 1; ui4 < vLight.size(); ui4++)
	{
		vector<Point> vPropLight;
		vPropLight.push_back(vLight[ui1]);
		vPropLight.push_back(vLight[ui2]);
		vPropLight.push_back(vLight[ui3]);
		vPropLight.push_back(vLight[ui4]);
		RotatedRect cPropRect = minAreaRect(vPropLight);
		
		int iMaxRect = max(cPropRect.size.width, cPropRect.size.height);
		int iMinRect = min(cPropRect.size.width, cPropRect.size.height);
		
		double dDiff = fabs(double(iMinRect) / double(iMaxRect) - 127 / 230);
		if (dDiff < dArmorRectDiffLimit)
		{
			if (dDiff < dDifference)
			{
				dDifference = dDiff;
				tArmor = cPropRect.center;
			}
		}
	}*/
	
	if (vLight.size() != 4)
	{
		return;
	}
	
	RotatedRect cRect = minAreaRect(vLight);
	tArmor = cRect.center;
	
	
	/*RotatedRect cRect = minAreaRect(vLight);
	int iMaxRect = max(cRect.size.width, cRect.size.height);
	int iMinRect = min(cRect.size.width, cRect.size.height);
	double dDiff = fabs(double(iMinRect) / double(iMaxRect) - 127 / 230);
	
	if (dDiff < dArmorRectDiffLimit &&
		iMaxRect < iArmorRectMaxSizeLimitLarge && iMaxRect > iArmorRectMaxSizeLimitSmall &&
		iMinRect < iArmorRectMinSizeLimitLarge && iMaxRect > iArmorRectMinSizeLimitSmall)
	{
		tArmor = cRect.center;
	}*/
}

void CArmor::FindLight(Mat mImg, vector<Point> & vLight)
{
	vLight.clear();
	
	vector<Mat> vMatPanel;
	split(mImg, vMatPanel);

	Mat mR, mG, mB;
	threshold(vMatPanel[2], mR, iThresholdR, 255, CV_THRESH_BINARY);
	threshold(vMatPanel[1], mG, iThresholdG, 255, CV_THRESH_BINARY);
	threshold(vMatPanel[0], mB, iThresholdB, 255, CV_THRESH_BINARY);

	Mat mKenelR = getStructuringElement(MORPH_RECT, Size(iMorphologyKenelR, iMorphologyKenelR));
	Mat mKenelG = getStructuringElement(MORPH_RECT, Size(iMorphologyKenelG, iMorphologyKenelG));
	Mat mKenelB = getStructuringElement(MORPH_RECT, Size(iMorphologyKenelB, iMorphologyKenelB));
	morphologyEx(mR, mR, MORPH_OPEN, mKenelR);
	morphologyEx(mG, mG, MORPH_OPEN, mKenelG);
	morphologyEx(mB, mB, MORPH_OPEN, mKenelB);

	Mat mAnd;
	bitwise_and(mR, mB, mAnd);
	bitwise_and(mAnd, mG, mAnd);

	#if DEBUG_ARMOR
	imshow("R Channel", mR);
	imshow("G Channel", mG);
	imshow("B Channel", mB);
	imshow("R-G-B And", mAnd);
	#endif

	vector<vector<Point>> vContours;
	vector<Point> vContoursCentroid;

	findContours(mAnd, vContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	#if DEBUG_ARMOR
	Mat mContoursAll(mAnd.size(), CV_8UC1, Scalar(0));
	drawContours(mContoursAll, vContours, -1, Scalar(255), 1);
	imshow("All Contours", mContoursAll);
	#endif

	for (vector<vector<Point>>::iterator itrContours = vContours.begin();
		itrContours != vContours.end(); itrContours++)
	{
		if (((*itrContours).size() < uiContoursPixelsLimitSmall) ||
			((*itrContours).size() > uiContoursPixelsLimitLarge))
		{
			itrContours = vContours.erase(itrContours);
			itrContours--;
			
			continue;
		}
		
		Moments cMom = moments(Mat(*itrContours));
		vContoursCentroid.push_back(Point(cMom.m10 / cMom.m00, cMom.m01 / cMom.m00));
	}

	#if DEBUG_ARMOR
	Mat mContoursFilted(mAnd.size(), CV_8UC1, Scalar(0));
	drawContours(mContoursFilted, vContours, -1, Scalar(255), 1);
	imshow("Filted Contours", mContoursFilted);
	#endif
	
	vector<RotatedRect> vRectContours;
	for (size_t ui = 0; ui < vContours.size(); ui++)
	{
		RotatedRect cRect = minAreaRect(vContours[ui]);
		vRectContours.push_back(cRect);
	}

	#if DEBUG_ARMOR
	Mat mContoursRect(mAnd.size(), CV_8UC1, Scalar(0));
	for (size_t ui = 0; ui < vRectContours.size(); ui++)
	{
		DrawRotatedRect(mContoursRect, vRectContours[ui], 255);
	}
	imshow("Rect Contours", mContoursRect);
	#endif
	
	for (size_t ui1 = 0; ui1 < vContours.size(); ui1++)
	{
		for (size_t ui2 = ui1 + 1; ui2 < vContours.size(); ui2++)
		{
			if ((fabs(vRectContours[ui1].angle - vRectContours[ui2].angle) * CV_PI / 180) > dContoursRectAngleLimit)
				continue;
			
			int iMaxRect1 = max(vRectContours[ui1].size.width, vRectContours[ui1].size.height);
			int iMinRect1 = min(vRectContours[ui1].size.width, vRectContours[ui1].size.height);
			
			int iMaxRect2 = max(vRectContours[ui2].size.width, vRectContours[ui2].size.height);
			int iMinRect2 = min(vRectContours[ui2].size.width, vRectContours[ui2].size.height);
			
			if ((abs(iMaxRect1 - iMaxRect2)) > iContoursRectMaxSideDiffLimit)
				continue;

			if ((abs(iMinRect1 - iMinRect2)) > iContoursRectMinSideDiffLimit)
				continue;
			
			if ((iMaxRect1 / iMinRect1) < dContoursRectAspectRatioLimitSmall ||
				(iMaxRect1 / iMinRect1) > dContoursRectAspectRatioLimitLarge ||
				(iMaxRect2 / iMinRect2) < dContoursRectAspectRatioLimitSmall ||
				(iMaxRect2 / iMinRect2) > dContoursRectAspectRatioLimitLarge)
				continue;

			bool bHaveAdd1 = false, bHaveAdd2 = false;

			for (size_t ui = 0; ui < vLight.size(); ui++)
			{
				if (vLight[ui] == vContoursCentroid[ui1])
				{
					bHaveAdd1 = true;
				}
				else if (vLight[ui] == vContoursCentroid[ui2])
				{
					bHaveAdd2 = true;
				}

				if (bHaveAdd1 && bHaveAdd2)
				{
					break;
				}
			}

			if (! bHaveAdd1)
			{
				vLight.push_back(vContoursCentroid[ui1]);
			}

			if (!bHaveAdd2)
			{
				vLight.push_back(vContoursCentroid[ui2]);
			}
		}
	}
	
	#if DEBUG_ARMOROUTPUT
	Mat mFoundLight(mAnd.size(), CV_8UC1, Scalar(0));
	drawContours(mFoundLight, vContours, -1, Scalar(100), 1);
	for (size_t ui = 0; ui < vRectContours.size(); ui++)
	{
		DrawRotatedRect(mFoundLight, vRectContours[ui], 100);
	}
	for (size_t ui = 0; ui < vLight.size(); ui++)
	{
		circle(mFoundLight, vLight[ui], 4, Scalar(255));
	}
	imshow("Found Light <OUTPUT>", mFoundLight);
	#endif
}

void CArmor::FindArmorBase(Mat mImg, Point & tArmor)
{
	tArmor = Point(-1, -1);
	
	vector<Mat> vMatPanel;
	split(mImg, vMatPanel);

	Mat mR, mG, mB;
	threshold(vMatPanel[2], mR, iThresholdR, 255, CV_THRESH_BINARY);
	threshold(vMatPanel[1], mG, iThresholdG, 255, CV_THRESH_BINARY);
	threshold(vMatPanel[0], mB, iThresholdB, 255, CV_THRESH_BINARY);

	Mat mKenelR = getStructuringElement(MORPH_RECT, Size(iMorphologyKenelR, iMorphologyKenelR));
	Mat mKenelG = getStructuringElement(MORPH_RECT, Size(iMorphologyKenelG, iMorphologyKenelG));
	Mat mKenelB = getStructuringElement(MORPH_RECT, Size(iMorphologyKenelB, iMorphologyKenelB));
	morphologyEx(mR, mR, MORPH_OPEN, mKenelR);
	morphologyEx(mG, mG, MORPH_OPEN, mKenelG);
	morphologyEx(mB, mB, MORPH_OPEN, mKenelB);

	Mat mAnd;
	bitwise_and(mR, mB, mAnd);
	bitwise_and(mAnd, mG, mAnd);

	#if DEBUG_ARMOR
	imshow("R Channel", mR);
	imshow("G Channel", mG);
	imshow("B Channel", mB);
	imshow("R-G-B And", mAnd);
	#endif

	vector<vector<Point>> vContours;
	vector<Point> vContoursCentroid;

	findContours(mAnd, vContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	#if DEBUG_ARMOR
	Mat mContoursAll(mAnd.size(), CV_8UC1, Scalar(0));
	drawContours(mContoursAll, vContours, -1, Scalar(255), 1);
	imshow("All Contours", mContoursAll);
	#endif

	vector<vector<Point>>::iterator itrContours = vContours.begin();
	while (itrContours != vContours.end())
	{
		if (((*itrContours).size() < uiContoursPixelsLimitSmall) ||
			((*itrContours).size() > uiContoursPixelsLimitLarge))
		{
			itrContours = vContours.erase(itrContours);	// itrContours will be next one
			continue;
		}
		
		Moments cMom = moments(Mat(*itrContours));
		vContoursCentroid.push_back(Point(cMom.m10 / cMom.m00, cMom.m01 / cMom.m00));
		
		itrContours++;
	}

	#if DEBUG_ARMOR
	Mat mContoursFilted(mAnd.size(), CV_8UC1, Scalar(0));
	drawContours(mContoursFilted, vContours, -1, Scalar(255), 1);
	imshow("Filted Contours", mContoursFilted);
	#endif
	
	vector<RotatedRect> vRectContours;
	for (size_t ui = 0; ui < vContours.size(); ui++)
	{
		RotatedRect cRect = minAreaRect(vContours[ui]);
		vRectContours.push_back(cRect);
	}

	#if DEBUG_ARMOR
	Mat mContoursRect(mAnd.size(), CV_8UC1, Scalar(0));
	for (size_t ui = 0; ui < vRectContours.size(); ui++)
	{
		DrawRotatedRect(mContoursRect, vRectContours[ui], 255);
	}
	imshow("Rect Contours", mContoursRect);
	#endif
	
	vector<Point> vLight;
	
	for (size_t ui1 = 0; ui1 < vContours.size(); ui1++)
	{
		for (size_t ui2 = ui1 + 1; ui2 < vContours.size(); ui2++)
		{
			double dMaxContoursRectAngle = max(fabs(vRectContours[ui1].angle), fabs(vRectContours[ui2].angle));
			double dMinContoursRectAngle = min(fabs(vRectContours[ui1].angle), fabs(vRectContours[ui2].angle));
			
			double dContoursRectAngle = dMaxContoursRectAngle - dMinContoursRectAngle;
			if (dContoursRectAngle > 45)
			{
				dContoursRectAngle = (90 - dMaxContoursRectAngle) + dMinContoursRectAngle;
			}

			if (dContoursRectAngle > dContoursRectAngleLimit)
				continue;

			//int iArea1 = vRectContours[ui1].size.width * vRectContours[ui1].size.height;
			//int iArea2 = vRectContours[ui2].size.width * vRectContours[ui2].size.height;
			
			//if (abs(iArea1 - iArea2) > iContoursRectAreaSizeDiffLimit)
			//	continue;

			int iMaxRect1 = max(vRectContours[ui1].size.width, vRectContours[ui1].size.height);
			int iMinRect1 = min(vRectContours[ui1].size.width, vRectContours[ui1].size.height);
			
			int iMaxRect2 = max(vRectContours[ui2].size.width, vRectContours[ui2].size.height);
			int iMinRect2 = min(vRectContours[ui2].size.width, vRectContours[ui2].size.height);
			
			if ((abs(iMaxRect1 - iMaxRect2)) > iContoursRectMaxSideDiffLimit)
				continue;

			if ((abs(iMinRect1 - iMinRect2)) > iContoursRectMinSideDiffLimit)
				continue;
			
			if ((iMaxRect1 / iMinRect1) < dContoursRectAspectRatioLimitSmall ||
				(iMaxRect1 / iMinRect1) > dContoursRectAspectRatioLimitLarge ||
				(iMaxRect2 / iMinRect2) < dContoursRectAspectRatioLimitSmall ||
				(iMaxRect2 / iMinRect2) > dContoursRectAspectRatioLimitLarge)
				continue;

			bool bHaveAdd1 = false, bHaveAdd2 = false;

			for (size_t ui = 0; ui < vLight.size(); ui++)
			{
				if (vLight[ui] == vContoursCentroid[ui1])
				{
					bHaveAdd1 = true;
				}
				else if (vLight[ui] == vContoursCentroid[ui2])
				{
					bHaveAdd2 = true;
				}

				if (bHaveAdd1 && bHaveAdd2)
				{
					break;
				}
			}

			if (! bHaveAdd1)
			{
				vLight.push_back(vContoursCentroid[ui1]);
			}

			if (!bHaveAdd2)
			{
				vLight.push_back(vContoursCentroid[ui2]);
			}
		}
	}
	
	#if DEBUG_ARMOROUTPUT
	Mat mFoundLight(mAnd.size(), CV_8UC1, Scalar(0));
	drawContours(mFoundLight, vContours, -1, Scalar(100), 1);
	for (size_t ui = 0; ui < vRectContours.size(); ui++)
	{
		DrawRotatedRect(mFoundLight, vRectContours[ui], 100);
	}
	for (size_t ui = 0; ui < vLight.size(); ui++)
	{
		circle(mFoundLight, vLight[ui], 4, Scalar(255));
	}
	imshow("Found Light <OUTPUT>", mFoundLight);
	#endif
	
	if (vLight.size() != 4)
	{
		return;
	}
	
	RotatedRect cRect = minAreaRect(vLight);
	tArmor = cRect.center;
}



