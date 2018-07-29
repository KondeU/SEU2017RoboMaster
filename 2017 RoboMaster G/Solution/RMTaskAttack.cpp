// RMTaskAttack.cpp
// Version 2.2 simple version, almost manual control
// Started by Deyou Kong, 2017-07-15
// Checked by Deyou Kong, 2017-07-15
// Updated by Deyou Kong, 2017-07-27

#include "RMTask.h"

int TagFilter(vector<TTagInfo> & vTagInfo); // From RMTaskDetect.cpp

void PixelToReal(float fFX, float fFY, float fCU, float fCV, // From RMTaskLand.cpp
	float fDist, int iPixX, int iPixY, double & dRealX, double & dRealY);

int Attack(CFlightIndoor & cFlgt)
{
	// Detect tags
	
	vector<TTagInfo> vTagInfo;
	cTag.Detect(mGryPyr, vTagInfo);
	int iTagFound = TagFilter(vTagInfo);

	sprintf(szOpTxt, "Tag found count: %d", iTagFound);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	static double dTagAccumHeight = 0;
	static double dTagAccumYaw = 0;

	if (iTagFound <= 0)
	{
		sprintf(szOpTxt, "Locate tags lost!");
		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);
	}
	else
	{
		dTagAccumHeight = 0;
		dTagAccumYaw = 0;

		for (size_t i = 0; i < vTagInfo.size(); i++)
		{
			dTagAccumHeight += vTagInfo[i].dZ;
			dTagAccumYaw    += vTagInfo[i].dYaw;
		}

		dTagAccumHeight /= vTagInfo.size();
		dTagAccumYaw    /= vTagInfo.size();

		Mat mShow = mGryDec.clone();

		for (size_t i = 0; i < vTagInfo.size(); i++)
		{
			// Draw all found and correct tag in mShowTag
			circle(mShow, Point(2 * vTagInfo[i].tCenter.fX, 2 * vTagInfo[i].tCenter.fY),
				4, Scalar(255), 2);
		}

		cLog.pVideolog->AddFrame(0, 1, mShow);
	}
	
	sprintf(szOpTxt, "TagHeight:%lfm, TagYaw:%lfdeg", dTagAccumHeight, dTagAccumYaw);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	// Find armor

	Point tArmor(-1, -1); // Armor position
	cArmor.FindArmorBase(mFrmRaw, tArmor);

	if (tArmor == Point(-1, -1)) // Do not find armor
	{
		cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_NONE);

		sprintf(szOpTxt, "Do not found armor!");

		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);
		
		return 0;
	}
	
	double dRealX = 0, dRealY = 0;
	PixelToReal(fRawFX, fRawFY, fRawPX, fRawPY,
		dTagAccumHeight, tArmor.x, tArmor.y, dRealX, dRealY); // Use tag height, must detect tag first

	double dTargetX = -dRealY; // Change to flight coord, TargetX is forward on flight
	double dTargetY = dRealX;  // Change to flight coord, TargetY is right on flight

	sprintf(szOpTxt, "ArmorPosition(x,y)=(%dpix,%dpix)", tArmor.x, tArmor.y);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "ArmorRealDist(forward,right)=(%lfm,%lfm)", dTargetX, dTargetY);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	circle(mGryDec, Point(tArmor.x / 2, tArmor.y / 2), 6, Scalar(255), 2);
	cLog.pVideolog->AddFrame(1, 0, mGryDec);

	Mat mShow = Mat::zeros(mGryDec.size().height, mGryDec.size().width, mGryDec.type());

	line(mShow, Point(0, fRawPY / 2), Point(mGryDec.size().width, fRawPY / 2),  Scalar(60));
	line(mShow, Point(fRawPX / 2, 0), Point(fRawPX / 2, mGryDec.size().height), Scalar(60));
	line(mShow, Point(0, fRawPY / 2), Point(mGryDec.size().width, fRawPY / 2),  Scalar(60));
	line(mShow, Point(fRawPX / 2, 0), Point(fRawPX / 2, mGryDec.size().height), Scalar(60));

	circle(mShow, Point(tArmor.x / 2, tArmor.y / 2), 4, Scalar(255), 2);
	cLog.pVideolog->AddFrame(1, 1, mShow);

	// Control

	TFlightState tfs;
	cFlgt.GetFlightState(tfs);
	float fCtrlHeight = tfs.fHeight;

	sprintf(szOpTxt, "Flight barometer height: %f", tfs.fHeight);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	if (iTagFound > 0)
	{
		if (dTagAccumHeight < 1.8)
		{
			fCtrlHeight += 0.2;
		}
		else if (dTagAccumHeight > 2.0)
		{
			fCtrlHeight -= 0.2;
		}
	}

	float fCtrlYaw = 0;
	
	if (iTagFound > 0)
	{
		if (fabs(dTagAccumYaw) > 5)
		{
			fCtrlYaw = -0.5 * dTagAccumYaw;
		}
	}

	sprintf(szOpTxt, "CTRL(hight,yaw)=(%fm,%fdeg/s)", fCtrlHeight, fCtrlYaw);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	cFlgt.IdrFlying(0, 0, fCtrlHeight, fCtrlYaw);

	// Show attack light or auto attack
	
	extern int iAttack;

	static double dAttackTimer = cLog.Timer();

	if (iAttack >= 10)
	{
		cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_CYAN);
	
		sprintf(szOpTxt, "Attack count over 10!");
		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);

		return 0;
	}
	
	if ((dTargetX < 0.1) && (dTargetY < 0.1))
	{
		cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_RED);
		
		if (cLog.Timer() - dAttackTimer > 2000)
		{
			cSerial.SetMotor(cLog.dGolf[iAttack]);

			sprintf(szOpTxt, "Attack now, num: %d", iAttack);
			cLog << szOpTxt << endl;
			cLog.pVideolog->AddText(szOpTxt);

			iAttack++;
			dAttackTimer = cLog.Timer();
		}
		else
		{
			sprintf(szOpTxt, "Attack gaping.");
			cLog << szOpTxt << endl;
			cLog.pVideolog->AddText(szOpTxt);
		}
	}
	else if ((dTargetX < 0.3) && (dTargetY < 0.3))
	{
		cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_GREEN);
	}
	else
	{
		cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_WHITE);
	}

	return 0;
}
