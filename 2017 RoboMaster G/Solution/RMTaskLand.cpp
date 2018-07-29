// RMTaskDetect.cpp
// Version 2.1
// Started by Deyou Kong, 2017-07-14
// Checked by Deyou Kong, 2017-07-14
// Updated by Deyou Kong, 2017-07-25

#include "RMTask.h"

void AdjustHghLineGap(float fHeight)
{
	double dHeight = 0.001 * fHeight;

	if (dHeight > 0.8)
	{
		tLand.iLmHghMaxLineGap = tLand.iLmHghMaxLineGapInit;
	}
	else
	{
		tLand.iLmHghMaxLineGap = tLand.iLmHghMaxLineGapInit + static_cast<int>(30 * (0.8 - dHeight));

		if (dHeight < 0.5)
		{
			tLand.iLmHghMaxLineGap += 2;
		}
	}
}

double CalcPointDistance(Point pnt1, Point pnt2)
{
	return sqrt((pnt1.x - pnt2.x) * (pnt1.x - pnt2.x) + (pnt1.y - pnt2.y) * (pnt1.y - pnt2.y));
}

void AddLine(Mat & mImg, Point pnt1, Point pnt2, int iAddValue = 1)
{
	LineIterator itLine(mImg, pnt1, pnt2);

	for (int i = 0; i < itLine.count; i++)
	{
		mImg.at<uchar>(itLine.pos()) += iAddValue;

		++itLine;
	}
}

void FindCross(Mat & mImg, vector<Point> & vcPntCross, int iThreshold = 1)
{
	vcPntCross.clear();

	for (int iRow = 0; iRow < mImg.rows; iRow++)
	{
		const unsigned char * pData = mImg.ptr<uchar>(iRow);

		for (int iCol = 0; iCol < mImg.cols; iCol++)
		{
			if (pData[iCol] > iThreshold)
			{
				vcPntCross.push_back(Point(iCol, iRow));
			}
			else if (pData[iCol] == iThreshold)
			{
				if ((iRow > 0) && (iCol > 0))
				{
					if (mImg.at<uchar>(iRow - 1, iCol) == iThreshold &&
						mImg.at<uchar>(iRow, iCol - 1) == iThreshold &&
						mImg.at<uchar>(iRow - 1, iCol - 1) == iThreshold)
					{
						vcPntCross.push_back(Point(iCol, iRow));
					}
				}
			}
		}
	}
}

void DrawCircles(Mat & mImg, vector<Point> & vPntCross)
{
	vector<Point>::iterator itBeg = vPntCross.begin();
	vector<Point>::iterator itEnd = vPntCross.end();

	for (vector<Point>::iterator it = itBeg; it < itEnd; ++it)
	{
		circle(mImg, *it, 3, Scalar(150));
	}
}

void PixelToReal(float fFX, float fFY, float fCU, float fCV,
	float fDist, int iPixX, int iPixY, double & dRealX, double & dRealY)
{
	dRealX = (iPixX - fCU) * fDist / fFX;
	dRealY = (iPixY - fCV) * fDist / fFY;
}

bool LandingControl(double dOffsetX, double dOffsetY, float fHeight, CFlightIndoor & cFlgt)
{
	const double dBasicVelocity = 0.3; // m/s

	double dSpeedX = 1.1 * max(min(dBasicVelocity * fHeight * dOffsetX, 0.1), -0.1);
	double dSpeedY = 1.1 * max(min(dBasicVelocity * fHeight * dOffsetY, 0.1), -0.1);

	TFlightState tfs;
	cFlgt.GetFlightState(tfs);
	float fLogHeight = tfs.fHeight;

	sprintf(szOpTxt, "Landing!! Speed(forward,right)=(%lfm/s,%lfm/s)", dSpeedX, dSpeedY);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "Landing!! Height(flog,ultra)=(%lfm,%lfm)", fLogHeight, fHeight);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	static int iLandingCounter = 0; 

	if (fHeight < 0.6)
	{
		if ((fabs(dOffsetX) < 0.06) && (fabs(dOffsetY) < 0.06))
		//if ((fabs(dOffsetX) < 0.03) && (fabs(dOffsetY) < 0.03))
		{
			iLandingCounter++;
			
			if (iLandingCounter > 2) // Counter >= 3
			{
				cFlgt.IdrFlyDn(4);
				cFlgt.Land();

				sprintf(szOpTxt, "Height lower than threshold, flight land!");
				cLog << szOpTxt << endl;
				cLog.pVideolog->AddText(szOpTxt);
				
				iLandingCounter = 0;
				
				return true;
			}
			else
			{
				sprintf(szOpTxt, "Height lower than threshold, counter: %d", iLandingCounter);
				cLog << szOpTxt << endl;
				cLog.pVideolog->AddText(szOpTxt);
				
				cFlgt.IdrFlying(1.2 * dSpeedX, 1.2 * dSpeedY, fLogHeight);
				
				return false;
			}
		}
		else
		{
			cFlgt.IdrFlying(1.2 * dSpeedX, 1.2 * dSpeedY, fLogHeight + 0.2);
		}
	}
	else if (fHeight < 1.0)
	{
		if (fabs(dOffsetX) < 0.1 && fabs(dOffsetY) < 0.1)
		{
			cFlgt.IdrFlying(dSpeedX, dSpeedY, fLogHeight - 0.2);
		}
		else
		{
			cFlgt.IdrFlying(dSpeedX, dSpeedY, fLogHeight);
		}
	}
	else
	{
		if (fabs(dOffsetX) < 0.2 && fabs(dOffsetY) < 0.2)
		{
			cFlgt.IdrFlying(dSpeedX, dSpeedY, fLogHeight - 0.2);
		}
		else
		{
			cFlgt.IdrFlying(dSpeedX, dSpeedY, fLogHeight);
		}
	}

	iLandingCounter = 0;

	return false;
}

int Land(CFlightIndoor & cFlgt)
{
	int iFrmHeight = mGryDec.rows;
	int iFrmWidth  = mGryDec.cols;

	Mat mShowLine   = Mat::zeros(iFrmHeight, iFrmWidth, mGryDec.type());
	Mat mShowCircle = Mat::zeros(iFrmHeight, iFrmWidth, mGryDec.type());

	//line(mShowLine,   Point(0, iFrmHeight / 2), Point(iFrmWidth, iFrmHeight / 2), Scalar(60));
	//line(mShowLine,   Point(iFrmWidth / 2, 0),  Point(iFrmWidth / 2, iFrmHeight), Scalar(60));
	//line(mShowCircle, Point(0, iFrmHeight / 2), Point(iFrmWidth, iFrmHeight / 2), Scalar(60));
	//line(mShowCircle, Point(iFrmWidth / 2, 0),  Point(iFrmWidth / 2, iFrmHeight), Scalar(60));
	line(mShowLine,   Point(0, tLand.fPY), Point(iFrmWidth, tLand.fPY),  Scalar(60));
	line(mShowLine,   Point(tLand.fPX, 0), Point(tLand.fPX, iFrmHeight), Scalar(60));
	line(mShowCircle, Point(0, tLand.fPY), Point(iFrmWidth, tLand.fPY),  Scalar(60));
	line(mShowCircle, Point(tLand.fPX, 0), Point(tLand.fPX, iFrmHeight), Scalar(60));

	static float fUltraDist = 1800;
	float fUltraDistTemp = cSerial.GetUltrasonicDistance();

	//if ((fUltraDistTemp > 50) && (fUltraDistTemp < 2500))
	if ((fUltraDistTemp > 100) && (fUltraDistTemp < 2500))
	{
		//if (fUltraDistTemp - fUltraDist > 0.2)
		//{
		//	cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_YELLOW);
		//
		//	sprintf(szOpTxt, "Ultrasonic distance change value is too large!");
		//	cLog << szOpTxt << endl;
		//	cLog.pVideolog->AddText(szOpTxt);
		//}
		//else
		//{
			fUltraDist = fUltraDistTemp;
		//}
	}
	else
	{
		//cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_YELLOW);
		
		sprintf(szOpTxt, "Ultrasonic distance out of limit!");
		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);
	}

	AdjustHghLineGap(fUltraDist); // Set dymanic hough line gap value

	// Processing lines

	Canny(mGryDec, mGryDec, tLand.iLmCnySml, tLand.iLmCnyLrg); // Processing circle also use the canny image

	vector<Vec4i> vLines, vLinesAdd; // vLines is probably lines, vLinesAdd is confirmed lines

	HoughLinesP(mGryDec, vLines, 1, CV_PI / 180, tLand.iLmHghThreshold,
		tLand.iLmHghMinLineLength, tLand.iLmHghMaxLineGap);

	for (size_t i = 0; i < vLines.size(); i++)
	{
		bool bAdd = true;

		Vec4i v4iLine = vLines[i];
		Point pntLine1(v4iLine[0], v4iLine[1]), pntLine2(v4iLine[2], v4iLine[3]);
		double dDistLine = CalcPointDistance(pntLine1, pntLine2);

		for (size_t iCheck = 0; iCheck < vLinesAdd.size(); iCheck++)
		{
			Vec4i v4iCheck = vLinesAdd[iCheck];
			Point pntCheck1(v4iCheck[0], v4iCheck[1]), pntCheck2(v4iCheck[2], v4iCheck[3]);
			double dDistCheck = CalcPointDistance(pntCheck1, pntCheck2);

			Vec4i v4iLrg, v4iSml;
			if (dDistLine > dDistCheck)
			{
				v4iLrg = v4iLine;
				v4iSml = v4iCheck;
			}
			else
			{
				v4iLrg = v4iCheck;
				v4iSml = v4iLine;
			}

			Point pntSml((v4iSml[0] + v4iSml[2]) / 2, (v4iSml[1] + v4iSml[3]) / 2); // Center of small line

			double dSmlK = (double)(v4iSml[3] - v4iSml[1]) / (double)(v4iSml[2] - v4iSml[0]);

			double dLrgK = (double)(v4iLrg[3] - v4iLrg[1]) / (double)(v4iLrg[2] - v4iLrg[0]);
			double dLrgB = (((double)v4iLrg[1] - dLrgK * (double)v4iLrg[0]) + ((double)v4iLrg[3] - dLrgK * (double)v4iLrg[2])) / 2;
			
			double dDiffK = fabs(dLrgK - dSmlK);
			double dDistPointLine = fabs((dLrgK * pntSml.x - pntSml.y + dLrgB) / sqrt(dLrgK * dLrgK + 1));

			if ((dDiffK < 0.2) && (dDistPointLine < 3))
			{
				vLinesAdd[iCheck] = v4iLrg;

				bAdd = false;
				break;
			}
		}

		if (bAdd)
		{
			vLinesAdd.push_back(v4iLine);
		}
	}

	Mat mAddLines = Mat::zeros(iFrmHeight, iFrmWidth, mGryDec.type());

	for (size_t i = 0; i < vLinesAdd.size(); i++)
	{
		Vec4i v4iAddLine = vLinesAdd[i];
		Point pntAddLine1(v4iAddLine[0], v4iAddLine[1]), pntAddLine2(v4iAddLine[2], v4iAddLine[3]);

		AddLine(mAddLines, pntAddLine1, pntAddLine2);

		line(mGryDec,   pntAddLine1, pntAddLine2, Scalar(127));
		line(mShowLine, pntAddLine1, pntAddLine2, Scalar(150));
	}

	vector<Point> vPntCross;
	FindCross(mAddLines, vPntCross);
	DrawCircles(mShowLine, vPntCross);

	Point pntLineCrossCenter(-1, -1); // Here is the center of line cross

	if ((vPntCross.size() > 1) && (vPntCross.size() < 5)) // Range:[2,4]
	{
		int iCenterX = 0, iCenterY = 0;
		for (size_t i = 0; i < vPntCross.size(); i++)
		{
			iCenterX += vPntCross[i].x;
			iCenterY += vPntCross[i].y;
		}
		pntLineCrossCenter = Point(iCenterX / vPntCross.size(), iCenterY / vPntCross.size());

		circle(mShowLine, pntLineCrossCenter, 3, Scalar(255));
	}

	// Processing circles

	vector<Vec3f> vCircles;

	HoughCircles(mGryDec, vCircles, CV_HOUGH_GRADIENT, 1, tLand.iLmHCMinDistCircles,
		tLand.iLmHCCnyLrgThreshold, tLand.iLmHCAccumThreshold,
		tLand.iLmHCMinR, tLand.iLmHCMaxR);

	Point pntCircleCenter(-1, -1); // Here is the center of circle

	int iCircleCenterR = INT_MAX;
	if ((vCircles.size() > 0) && (vCircles.size() < 4)) // Range:[1,3]
	{
		int iCenterX = 0, iCenterY = 0;
		for (size_t i = 0; i < vCircles.size(); i++)
		{
			iCenterX += cvRound(vCircles[i][0]);
			iCenterY += cvRound(vCircles[i][1]);
			iCircleCenterR = min(iCircleCenterR, cvRound(vCircles[i][2]));

			circle(mShowCircle, Point(vCircles[i][0], vCircles[i][1]), 3, Scalar(150)); // Draw circles center
			circle(mShowCircle, Point(vCircles[i][0], vCircles[i][1]), cvRound(vCircles[i][2]), Scalar(150));
		}
		pntCircleCenter = Point(iCenterX / vCircles.size(), iCenterY / vCircles.size());

		circle(mGryDec, pntCircleCenter, 3, Scalar(127)); // Draw circle center
		circle(mGryDec, pntCircleCenter, iCircleCenterR, Scalar(127));

		circle(mShowCircle, pntCircleCenter, 3, Scalar(255));
		circle(mShowCircle, pntCircleCenter, iCircleCenterR, Scalar(255));
	}

	// Show detail

	cLog.pVideolog->AddFrame(0, 1, mGryDec);
	cLog.pVideolog->AddFrame(1, 0, mShowLine);
	cLog.pVideolog->AddFrame(1, 1, mShowCircle);

	sprintf(szOpTxt, "Line=> AllLines:%d,ValidLines:%d,Crosses:%d",
		vLines.size(), vLinesAdd.size(), vPntCross.size());
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "Line=> Center(x,y)=(%d,%d)", pntLineCrossCenter.x, pntLineCrossCenter.y);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt,
		"L-Factor=> CnySml:%d || CnyLrg:%d",
		tLand.iLmCnySml, tLand.iLmCnyLrg);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt,
		"L-Factor=> HthThr:%d || MinLineLength:%d || MaxLineGap:%d",
		tLand.iLmHghThreshold, tLand.iLmHghMinLineLength, tLand.iLmHghMaxLineGap);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "Circles=> Circles:%d", vCircles.size());
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "Circles=> Center(x,y)=(%d,%d)", pntCircleCenter.x, pntCircleCenter.y);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt,
		"C-Factor=> MinDist:%d || CnyLrg:%d",
		tLand.iLmHCMinDistCircles, tLand.iLmHCCnyLrgThreshold);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt,
		"C-Factor=> AccumThr:%d || MinR:%d || MaxR:%d",
		tLand.iLmHCAccumThreshold, tLand.iLmHCMinR, tLand.iLmHCMaxR);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	// Control

	if (((pntLineCrossCenter.x < 0) || (pntLineCrossCenter.y < 0)) &&
		((pntCircleCenter.x    < 0) || (pntCircleCenter.y    < 0)))
	{
		iState = STATE_ERROR;

		cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_NONE);

		sprintf(szOpTxt, "Landmark target lost!");
		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);

		return 0;
	}

	// Pixel distance -> Real distance

	int iPixX = 0, iPixY = 0;
	double dRealX = 0, dRealY = 0;

	if ((pntLineCrossCenter.x > 0) && (pntLineCrossCenter.y > 0))
	{
		iPixX = pntLineCrossCenter.x;
		iPixY = pntLineCrossCenter.y;
	}
	else
	{
		iPixX = pntCircleCenter.x;
		iPixY = pntCircleCenter.y;
	}

	PixelToReal(tLand.fFX, tLand.fFY, tLand.fPX, tLand.fPY, fUltraDist, iPixX, iPixY, dRealX, dRealY);

	sprintf(szOpTxt, "Current location in pixel (x,y)=(%dpix,%dpix)", iPixX, iPixY);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "Real dist to center (x,y)=(%lfmm,%lfmm)", dRealX, dRealY);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	// Need convert image coord to flight coord and reflect to control

	// Image coord:
	// +------x
	// |
	// |
	// y

	// Flight coord:
	// x
	// |
	// |
	// +------y

	if (LandingControl(0.001 * -dRealY, 0.001 * dRealX, 0.001 * fUltraDist, cFlgt))
	{
		iState = STATE_ERROR;

		cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_RED);
		
		fUltraDist = 1800;
	}

	return 0;
}
