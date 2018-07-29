// RMTaskAttack.cpp
// Version 2.1
// Started by Deyou Kong, 2017-07-15
// Checked by Deyou Kong, 2017-07-15
// Updated by Deyou Kong, 2017-07-26

#include "RMTask.h"

int TagFilter(vector<TTagInfo> & vTagInfo); // From RMTaskDetect.cpp

void PixelToReal(float fFX, float fFY, float fCU, float fCV, // From RMTaskLand.cpp
	float fDist, int iPixX, int iPixY, double & dRealX, double & dRealY);

/*int Attack(CFlightIndoor & cFlgt)
{
	vector<TTagInfo> vTagInfo;
	cTag.Detect(mGryPyr, vTagInfo);
	int iTagFound = TagFilter(vTagInfo);

	sprintf(szOpTxt, "Tag found count: %d", iTagFound);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	if (iTagFound <= 0)
	{
		iState = STATE_ERROR;

		cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_NONE);

		sprintf(szOpTxt, "Locate tags lost!");
		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);

		return 0;
	}

	for (size_t i = 0; i < vTagInfo.size(); i++)
	{
		// Draw all found and correct tag in mShowTag
		circle(mGryDec, Point(2 * vTagInfo[i].tCenter.fX, 2 * vTagInfo[i].tCenter.fY),
			4, Scalar(255), 2);
	}

	cLog.pVideolog->AddFrame(1, 0, mGryDec);

	// Map coord
	Point2d * pLocOrg  = &(cLog.tLoc[LOC_ORG]);
	Point2d * pLocLim  = &(cLog.tLoc[LOC_LIM]);
	Point2d * pLocCrnt = &(cLog.tLoc[LOC_CRNT]);
	Point2d * pLocDest = &(cLog.tLoc[LOC_DEST]);

	double dTagAccumHeight = 0;
	double dTagAccumYaw    = 0;

	for (size_t i = 0; i < vTagInfo.size(); i++)
	{
		pLocCrnt->x += cLog.tLoc[vTagInfo[i].iTagID].x - vTagInfo[i].dX;
		pLocCrnt->y += cLog.tLoc[vTagInfo[i].iTagID].y - vTagInfo[i].dY;
		dTagAccumHeight += vTagInfo[i].dZ;
		dTagAccumYaw    += vTagInfo[i].dYaw;
	}
	pLocCrnt->x /= vTagInfo.size();
	pLocCrnt->y /= vTagInfo.size();
	dTagAccumHeight /= vTagInfo.size();
	dTagAccumYaw    /= vTagInfo.size();
	
	sprintf(szOpTxt, "TagHeight:%lfm, TagYaw:%lfdeg", dTagAccumHeight, dTagAccumYaw);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	Point tArmor(-1, -1); // Armor position
	cArmor.FindArmor(mFrmRaw, tArmor);

	bool bArmorValid = false;
	double dTargetX = 0, dTargetY = 0;

	if (tArmor == Point(-1, -1)) // Do not find armor
	{
		pLocDest->x = (pLocOrg->x + pLocLim->x) / 2;
		pLocDest->y = (pLocOrg->y + pLocLim->y) / 2;

		dTargetX = pLocDest->x - pLocCrnt->x;
		dTargetY = pLocDest->y - pLocCrnt->y;

		sprintf(szOpTxt, "Do not found armor!");
		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);
	}
	else // Find probable armor
	{
		double dRealX = 0, dRealY = 0;
		PixelToReal(fRawFX, fRawFY, fRawPX, fRawPY,
			dTagAccumHeight, tArmor.x, tArmor.y, dRealX, dRealY);

		// Change to flight coord
		
		dTargetX = -dRealY;
		dTargetY = dRealX;

		pLocDest->x = pLocCrnt->x + dTargetX;
		pLocDest->y = pLocCrnt->y + dTargetY;

		if ((pLocDest->x < pLocOrg->x) || (pLocDest->x > pLocLim->x) ||
			(pLocDest->y < pLocOrg->y) || (pLocDest->y > pLocLim->y))
		{
			// Armor outside

			pLocDest->x = (pLocOrg->x + pLocLim->x) / 2;
			pLocDest->y = (pLocOrg->y + pLocLim->y) / 2;

			dTargetX = pLocDest->x - pLocCrnt->x;
			dTargetY = pLocDest->y - pLocCrnt->y;

			sprintf(szOpTxt, "Armor(x,y)=(%dpix,%dpix), Found armor outside!", tArmor.x, tArmor.y);
			cLog << szOpTxt << endl;
			cLog.pVideolog->AddText(szOpTxt);
		}
		else
		{
			bArmorValid = true;
		}
	}

	if (bArmorValid)
	{
		circle(mGryDec, Point(tArmor.x / 2, tArmor.y / 2), 6, Scalar(255), 2);
		cLog.pVideolog->AddFrame(1, 1, mGryDec);
	}
	else
	{
		circle(mGryDec, Point(tArmor.x / 2, tArmor.y / 2), 6, Scalar(127), 2);
		cLog.pVideolog->AddFrame(1, 1, mGryDec);		
	}

	sprintf(szOpTxt, "ArmorValid:%d, Target(x,y)=(%lf,%lf)", bArmorValid, dTargetX, dTargetY);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "MapLoc->Crnt(x,y)=(%lf,%lf)", pLocCrnt->x, pLocCrnt->y);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "MapLoc->Dest(x,y)=(%lf,%lf)", pLocDest->x, pLocDest->y);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	// Draw map (Note that base on 320*240)

	Mat mShowMap = Mat::zeros(mGryDec.rows, mGryDec.cols, mGryDec.type());
	line(mShowMap, Point(40,  0), Point(40,  240), Scalar(255));
	line(mShowMap, Point(280, 0), Point(280, 240), Scalar(255));
	circle(mShowMap,
		Point(
			40 + 240 * (pLocDest->x / (pLocLim->x - pLocOrg->x)),
			240 * (pLocDest->y / (pLocLim->y - pLocOrg->y))
			),
		5, Scalar(150));
	circle(mShowMap,
		Point(
			40 + 240 * (pLocCrnt->x / (pLocLim->x - pLocOrg->x)),
			240 * (pLocCrnt->y / (pLocLim->y - pLocOrg->y))
			),
		3, Scalar(255));
	cLog.pVideolog->AddFrame(0, 1, mShowMap);

	// Control

	TFlightState tfs;
	cFlgt.GetFlightState(tfs);
	float fCtrlHeight = tfs.fHeight;

	sprintf(szOpTxt, "Flight barometer height: %f", tfs.fHeight);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	if (dTagAccumHeight < 0.8)
	{
		fCtrlHeight += 0.2;
	}
	else if (dTagAccumHeight > 1.2)
	{
		fCtrlHeight -= 0.2;
	}

	float fCtrlYaw = 0;
	if (fabs(dTagAccumYaw) > 3)
	{
		fCtrlYaw = -1 * dTagAccumYaw;
	}

	// 0 is current, 1 is last1, 2 is last2
	static Point2d tDiff[3] = { Point2d(0, 0), Point2d(0, 0), Point2d(0, 0) };
	tDiff[2].x = tDiff[1].x;
	tDiff[2].y = tDiff[1].y;
	tDiff[1].x = tDiff[0].x;
	tDiff[1].y = tDiff[0].y;
	tDiff[0].x = dTargetX;
	tDiff[0].y = dTargetY;

	double dPX = 0.5;
	double dIX = 0.02;
	double dDX = 0.1;
	double dPY = 0.5;
	double dIY = 0.02;
	double dDY = 0.1;

	double dCtrlX = 
		dPX * (tDiff[0].x - tDiff[1].x) +
		dIX * tDiff[0].x +
		dDX * (tDiff[0].x - 2 * tDiff[1].x + tDiff[2].x);

	double dCtrlY =
		dPY * (tDiff[0].y - tDiff[1].y) +
		dIY * tDiff[0].y +
		dDY * (tDiff[0].y - 2 * tDiff[1].y + tDiff[2].y);

	static double dLastCtrlX = 0, dLastCtrlY = 0;
	dCtrlX += dLastCtrlX;
	dCtrlY += dLastCtrlY;
	dLastCtrlX = dCtrlX;
	dLastCtrlY = dCtrlY;

	sprintf(szOpTxt, "PID-X(p,i,d)=(%lf,%lf,%lf)", dPX, dIX, dDX);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "PID-Y(p,i,d)=(%lf,%lf,%lf)", dPY, dIY, dDY);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "CTRL(hgh,yaw)=(%f,%f)", fCtrlHeight, fCtrlYaw);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "CTRL(x,y)=(%lf,%lf)", dCtrlX, dCtrlY);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	cFlgt.IdrFlying(dCtrlX, dCtrlY, fCtrlHeight, fCtrlYaw);

	return 0;
}*/

//double dPX = 0.01;
double dPX = 0.5;
double dIX = 0.0;
double dDX = 0.0;
//double dPY = 0.01;
double dPY = 0.5;
double dIY = 0.0;
double dDY = 0.0;

int Attack(CFlightIndoor & cFlgt)
{
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
		cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_WHITE);

		dTagAccumYaw = 0;

		sprintf(szOpTxt, "Locate tags lost!");
		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);

		//return 0;
	}

	for (size_t i = 0; i < vTagInfo.size(); i++)
	{
		// Draw all found and correct tag in mShowTag
		circle(mGryDec, Point(2 * vTagInfo[i].tCenter.fX, 2 * vTagInfo[i].tCenter.fY),
			4, Scalar(255), 2);
	}

	cLog.pVideolog->AddFrame(1, 0, mGryDec);

	if (vTagInfo.size())
	{
		dTagAccumHeight = 0;
		dTagAccumYaw    = 0;
	
		for (size_t i = 0; i < vTagInfo.size(); i++)
		{
			dTagAccumHeight += vTagInfo[i].dZ;
			dTagAccumYaw    += vTagInfo[i].dYaw;
		}
	
		dTagAccumHeight /= vTagInfo.size();
		dTagAccumYaw    /= vTagInfo.size();
	}
	
	sprintf(szOpTxt, "TagHeight:%lfm, TagYaw:%lfdeg", dTagAccumHeight, dTagAccumYaw);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

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
	
	sprintf(szOpTxt, "ArmorPosition:(%dpix,%dpix)", tArmor.x, tArmor.y);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	double dRealX = 0, dRealY = 0;
	PixelToReal(fRawFX, fRawFY, fRawPX, fRawPY,
		dTagAccumHeight, tArmor.x, tArmor.y, dRealX, dRealY);

	// Change to flight coord
	
	double dTargetX = -dRealY;
	double dTargetY = dRealX;

	circle(mGryDec, Point(tArmor.x / 2, tArmor.y / 2), 6, Scalar(255), 2);
	cLog.pVideolog->AddFrame(1, 1, mGryDec);

	sprintf(szOpTxt, "ArmorRealDist(forward,right)=(%lfm,%lfm)", dTargetX, dTargetY);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	// Attack
	
	static int iAttack = 0;
	static double dAttackTimer = cLog.Timer();
	
	if (iAttack >= 14)
	{
		cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_CYAN);
	
		sprintf(szOpTxt, "Attack count over 14.");
		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);

		return 0;
	}
	else
	{
		if ((fabs(dTargetX) < 0.05) && (fabs(dTargetY) < 0.05))
		{
			if (cLog.Timer() - dAttackTimer > 3000)
			{
				//cSerial.SetMotor(cLog.dGolf[iAttack]);
			
				if (iAttack < 8)
				{
					cSerial.SetMotor(-40); // Here set step motor angle
				}
				else
				{
					cSerial.SetMotor(-35); // Here set step motor angle
				}
				
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
	}

	// Draw map
	
	Mat mShow = Mat::zeros(mGryDec.size().height, mGryDec.size().width, mGryDec.type());
	
	line(mShow, Point(0, fRawPY / 2), Point(mGryDec.size().width, fRawPY / 2),  Scalar(60));
	line(mShow, Point(fRawPX / 2, 0), Point(fRawPX / 2, mGryDec.size().height), Scalar(60));
	line(mShow, Point(0, fRawPY / 2), Point(mGryDec.size().width, fRawPY / 2),  Scalar(60));
	line(mShow, Point(fRawPX / 2, 0), Point(fRawPX / 2, mGryDec.size().height), Scalar(60));

	circle(mShow, Point(tArmor.x / 2, tArmor.y / 2), 4, Scalar(255), 2);
	cLog.pVideolog->AddFrame(0, 1, mShow);

	// Control

	TFlightState tfs;
	cFlgt.GetFlightState(tfs);
	float fCtrlHeight = tfs.fHeight;

	sprintf(szOpTxt, "Flight barometer height: %f", tfs.fHeight);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	if (dTagAccumHeight < 1.65)
	{
		fCtrlHeight += 0.2;
	}
	else if (dTagAccumHeight > 1.75)
	{
		fCtrlHeight -= 0.2;
	}

	float fCtrlYaw = 0;
	if (fabs(dTagAccumYaw) > 3)
	{
		fCtrlYaw = -1 * dTagAccumYaw;
	}

	// 0 is current, 1 is last1, 2 is last2
	static Point2d tDiff[3] = { Point2d(0, 0), Point2d(0, 0), Point2d(0, 0) };
	tDiff[2].x = tDiff[1].x;
	tDiff[2].y = tDiff[1].y;
	tDiff[1].x = tDiff[0].x;
	tDiff[1].y = tDiff[0].y;
	tDiff[0].x = dTargetX;
	tDiff[0].y = dTargetY;

	//double dPX = 0.0;
	//double dIX = 0.0;
	//double dDX = 0.0;
	//double dPY = 0.0;
	//double dIY = 0.0;
	//double dDY = 0.0;

	double dCtrlX = 
		dPX * (tDiff[0].x - tDiff[1].x) +
		dIX * tDiff[0].x +
		dDX * (tDiff[0].x - 2 * tDiff[1].x + tDiff[2].x);

	double dCtrlY =
		dPY * (tDiff[0].y - tDiff[1].y) +
		dIY * tDiff[0].y +
		dDY * (tDiff[0].y - 2 * tDiff[1].y + tDiff[2].y);

	static double dLastCtrlX = 0, dLastCtrlY = 0;
	dCtrlX += dLastCtrlX;
	dCtrlY += dLastCtrlY;
	dLastCtrlX = dCtrlX;
	dLastCtrlY = dCtrlY;

	dCtrlX = max(min(dCtrlX, 0.5), -0.5);
	dCtrlY = max(min(dCtrlY, 0.5), -0.5);

	sprintf(szOpTxt, "PID-X(p,i,d)=(%lf,%lf,%lf)", dPX, dIX, dDX);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "PID-Y(p,i,d)=(%lf,%lf,%lf)", dPY, dIY, dDY);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "CTRL(hight,yaw)=(%fm,%fdeg/s)", fCtrlHeight, fCtrlYaw);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "CTRL(x,y)=(%lfm/s,%lfm/s)", dCtrlX, dCtrlY);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	cFlgt.IdrFlying(dCtrlX, dCtrlY, fCtrlHeight, fCtrlYaw);

	return 0;
}

/*int Attack(CFlightIndoor & cFlgt)
{
	vector<TTagInfo> vTagInfo;
	cTag.Detect(mGryPyr, vTagInfo);
	int iTagFound = TagFilter(vTagInfo);

	sprintf(szOpTxt, "Tag found count: %d", iTagFound);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	for (size_t i = 0; i < vTagInfo.size(); i++)
	{
		// Draw all found and correct tag in mShowTag
		circle(mGryDec, Point(2 * vTagInfo[i].tCenter.fX, 2 * vTagInfo[i].tCenter.fY),
			4, Scalar(255), 2);
	}

	cLog.pVideolog->AddFrame(1, 0, mGryDec);

	static double dTagAccumHeight = 0;
	static double dTagAccumYaw = 0;

	if (iTagFound <= 0)
	{
		cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_WHITE);

		dTagAccumYaw = 0;

		sprintf(szOpTxt, "Locate tags lost!");
		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);
	}
	else
	{
		dTagAccumHeight = 0;
		dTagAccumYaw    = 0;

		for (size_t i = 0; i < vTagInfo.size(); i++)
		{
			dTagAccumHeight += vTagInfo[i].dZ;
			dTagAccumYaw    += vTagInfo[i].dYaw;
		}
	
		dTagAccumHeight /= vTagInfo.size();
		dTagAccumYaw    /= vTagInfo.size();
	}
	
	sprintf(szOpTxt, "TagHeight:%lfm, TagYaw:%lfdeg", dTagAccumHeight, dTagAccumYaw);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

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
	
	sprintf(szOpTxt, "ArmorPosition(x,y)=(%dpix,%dpix)", tArmor.x, tArmor.y);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	Mat mShow = Mat::zeros(mGryDec.size().height, mGryDec.size().width, mGryDec.type());

	line(mShow, Point(0, fRawPY / 2), Point(mGryDec.size().width, fRawPY / 2),  Scalar(60));
	line(mShow, Point(fRawPX / 2, 0), Point(fRawPX / 2, mGryDec.size().height), Scalar(60));
	line(mShow, Point(0, fRawPY / 2), Point(mGryDec.size().width, fRawPY / 2),  Scalar(60));
	line(mShow, Point(fRawPX / 2, 0), Point(fRawPX / 2, mGryDec.size().height), Scalar(60));

	circle(mShow, Point(tArmor.x / 2, tArmor.y / 2), 4, Scalar(255), 2);
	cLog.pVideolog->AddFrame(0, 1, mShow);

	double dRealX = 0, dRealY = 0;
	PixelToReal(fRawFX, fRawFY, fRawPX, fRawPY,
		dTagAccumHeight, tArmor.x, tArmor.y, dRealX, dRealY);

	// Change to flight coord
	
	double dTargetX = -dRealY;
	double dTargetY = dRealX;

	circle(mGryDec, Point(tArmor.x / 2, tArmor.y / 2), 6, Scalar(255), 2);
	cLog.pVideolog->AddFrame(1, 1, mGryDec);

	sprintf(szOpTxt, "ArmorRealDist(forward,right)=(%lfm,%lfm)", dTargetX, dTargetY);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	// Attack
	
	static int iAttack = 0;
	static double dAttackTimer = cLog.Timer();
	
	static double dDist[3];
	
	dDist[2] = dDist[1];
	dDist[1] = dDist[0];
	dDist[0] = sqrt(dTargetX * dTargetX + dTargetY + dTargetY);
	
	//if ((dDist[0] < 0.2) && (dDist[2] < 0.4))
	if (dDist[0] < 0.2)
	{
		//if ((dDist[0] < dDist[1]) && (dDist[1] < dDist[2]))
		//{
			if (iAttack >= 14)
			{
				cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_CYAN);
	
				sprintf(szOpTxt, "Attack count over 14!");
				cLog << szOpTxt << endl;
				cLog.pVideolog->AddText(szOpTxt);

				return 0;
			}
			else
			{
				if (cLog.Timer() - dAttackTimer > 3000)
				{
					//cSerial.SetMotor(cLog.dGolf[iAttack]);
			
					if (iAttack < 8)
					{
						cSerial.SetMotor(-40); // Here set step motor angle
					}
					else
					{
						cSerial.SetMotor(-35); // Here set step motor angle
					}
				
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
		//}
	}

	// Control

	TFlightState tfs;
	cFlgt.GetFlightState(tfs);
	float fCtrlHeight = tfs.fHeight;

	sprintf(szOpTxt, "Flight barometer height: %f", tfs.fHeight);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	if (dTagAccumHeight < 1.8)
	{
		fCtrlHeight += 0.2;
	}
	else if (dTagAccumHeight > 2.0)
	{
		fCtrlHeight -= 0.2;
	}

	float fCtrlYaw = 0;
	if (fabs(dTagAccumYaw) > 5)
	{
		fCtrlYaw = -0.5 * dTagAccumYaw;
	}

	double dCtrlX = 0;
	double dCtrlY = 0;

	if((fabs(dTargetX) < 0.3) && (fabs(dTargetY) < 0.3))
	{
		const double dBasicVelocity = 0.3; // m/s

		dCtrlX = 1.1 * max(min(dBasicVelocity * (1.0 / dTagAccumHeight) * dTargetX, 0.1), -0.1);
		dCtrlY = 1.1 * max(min(dBasicVelocity * (1.0 / dTagAccumHeight) * dTargetY, 0.1), -0.1);
	}

	sprintf(szOpTxt, "CTRL(hight,yaw)=(%fm,%fdeg/s)", fCtrlHeight, fCtrlYaw);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	sprintf(szOpTxt, "CTRL(x,y)=(%lfm/s,%lfm/s)", dCtrlX, dCtrlY);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	cFlgt.IdrFlying(dCtrlX, dCtrlY, fCtrlHeight, fCtrlYaw);

	return 0;
}*/
