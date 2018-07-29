// RMTaskDetect.cpp
// Version 2.1
// Started by Deyou Kong, 2017-07-14
// Checked by Deyou Kong, 2017-07-14
// Updated by Deyou Kong, 2017-07-26

#include "RMTask.h"

int TagFilter(vector<TTagInfo> & vTagInfo)
{
	//for (vector<TTagInfo>::iterator itr = vTagInfo.begin(); itr != vTagInfo.end(); itr++)
	vector<TTagInfo>::iterator itr = vTagInfo.begin();
	while (itr != vTagInfo.end())
	{
		// Change tag coord to flight

		(*itr).dY      = -(*itr).dY;
		double dHeight = (*itr).dX;
		(*itr).dX      = (*itr).dZ;
		(*itr).dZ      = dHeight;
		double dPitch  = (*itr).dRoll;
		(*itr).dRoll   = (*itr).dPitch;
		(*itr).dPitch  = dPitch;

		// deg
		(*itr).dYaw   = CL_RADTODEG((*itr).dYaw);
		(*itr).dRoll  = CL_RADTODEG((*itr).dRoll);
		(*itr).dPitch = CL_RADTODEG((*itr).dPitch);

		// mm
		//(*itr).dX *= 1000;
		//(*itr).dY *= 1000;
		//(*itr).dZ *= 1000;

		if (!(((*itr).iTagID < 8) || ((*itr).iTagID == 10))) // ID limit
		{
			itr = vTagInfo.erase(itr);
			//itr--;
			continue;
		}

		//if (((*itr).dZ < 800) || ((*itr).dZ > 2500)) // Height limit
		if (((*itr).dZ < 0.8) || ((*itr).dZ > 3.5)) // Height limit
		{
			itr = vTagInfo.erase(itr);
			//itr--;
			continue;
		}

		if ((fabs((*itr).dRoll) > 20) || (fabs((*itr).dPitch) > 20)) // Roll and Pitch limit
		{
			itr = vTagInfo.erase(itr);
			//itr--;
			continue;
		}
		
		itr++;
	}

	return vTagInfo.size();
}

int Detect()
{
	// Find tag

	Mat mShowTag = mGryDec.clone();

	vector<TTagInfo> vTagInfo;
	cTag.Detect(mGryPyr, vTagInfo);
	int iTagFound = TagFilter(vTagInfo);

	for (size_t i = 0; i < vTagInfo.size(); i++)
	{
		// Draw all found and correct tag in mShowTag
		circle(mShowTag, Point(2 * vTagInfo[i].tCenter.fX, 2 * vTagInfo[i].tCenter.fY),
			4, Scalar(255), 2);
	}

	cLog.pVideolog->AddFrame(0, 1, mShowTag);

	sprintf(szOpTxt, "Tag found count: %d", iTagFound);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	// Find circle

	Mat mShowCir = Mat::zeros(mGryDec.rows, mGryDec.cols, mGryDec.type());

	Canny(mGryDec, mGryDec, tLand.iLmCnySml, tLand.iLmCnyLrg);

	vector<Vec3f> vCircles;
	HoughCircles(mGryDec, vCircles, CV_HOUGH_GRADIENT, 1, tLand.iLmHCMinDistCircles,
		tLand.iLmHCCnyLrgThreshold, tLand.iLmHCAccumThreshold,
		tLand.iLmHCMinR, tLand.iLmHCMaxR);

	for (size_t i = 0; i < vCircles.size(); i++)
	{
		// Draw all circles in mShowCir
		circle(mShowCir, Point(vCircles[i][0], vCircles[i][1]), 3, Scalar(255));
		circle(mShowCir, Point(vCircles[i][0], vCircles[i][1]), cvRound(vCircles[i][2]), Scalar(255));

		// Draw all circles in cannyed image
		circle(mGryDec, Point(vCircles[i][0], vCircles[i][1]), 3, Scalar(127));
		circle(mGryDec, Point(vCircles[i][0], vCircles[i][1]), cvRound(vCircles[i][2]), Scalar(127));
	}

	cLog.pVideolog->AddFrame(1, 0, mGryDec);
	cLog.pVideolog->AddFrame(1, 1, mShowCir);

	sprintf(szOpTxt, "Circle found count: %d", vCircles.size());
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	if (vTagInfo.size() > 0) // Found tags
	{
		iDetect = STATE_ATTACK;

		if (CTRL_EXECUTE == iCrntCtrl)
		{
			iState = STATE_ATTACK;

			cSerial.SetLight(FRMDAT_LIGHT_LEFT, LGCLR_GREEN);
			cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_GREEN);
		}
		else // CTRL_MANUAL == iCrntCtrl
		{
			cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_GREEN);
		}
	}
	else if (vCircles.size() > 0) // Found circles
	{
		iDetect = STATE_LAND;

		if (CTRL_EXECUTE == iCrntCtrl)
		{
			iState = STATE_LAND;

			cSerial.SetLight(FRMDAT_LIGHT_LEFT, LGCLR_BLUE);
			cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_BLUE);
		}
		else // CTRL_MANUAL == iCrntCtrl
		{
			cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_BLUE);
		}
	}
	else // None found
	{
		iDetect = STATE_ERROR;

		if (CTRL_EXECUTE == iCrntCtrl)
		{
			iState = STATE_ERROR;

			cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_NONE);
		}
		else // CTRL_MANUAL == iCrntCtrl
		{
			cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_RED);
		}
	}

	return 0;
}
