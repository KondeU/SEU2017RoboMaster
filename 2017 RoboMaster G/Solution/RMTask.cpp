// RMTask.cpp
// Version 2.1
// Started by Deyou Kong, 2017-07-10
// Checked by Deyou Kong, 2017-07-10
// Updated by Deyou Kong, 2017-07-30

#include "RMTask.h"

int iAttack = 0;

void GearController(CFlight & cFlight)
{
	static int iLastGear = GEAR_UP;
	int iGear = cFlight.GetGear();

	#if !DEBUG_DIALCTR
	// Normal gear control

	static double dManualAttackTimer = cLog.Timer();
	static bool bManualAttack = false;

	switch (iGear)
	{
	case GEAR_ERR: // If occur gear error, use default gear state
		if (CTRL_MANUAL != iCrntCtrl)
		{
			cFlight.CtrlRelease();
			iCrntCtrl = CTRL_MANUAL;

			iState = STATE_DETECT;

			cSerial.SetLight(FRMDAT_LIGHT_LEFT, LGCLR_RED);
			cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_RED);
		}
		break;
	
	case GEAR_UP: // Manual control
		if (iLastGear == GEAR_DN)
		{
			dManualAttackTimer = cLog.Timer();
			bManualAttack = true;
			break;
		}
		else
		{
			if (cLog.Timer() - dManualAttackTimer > 500)
			{
				bManualAttack = false;
				if (CTRL_MANUAL != iCrntCtrl)
				{
					dManualAttackTimer = cLog.Timer();

					cFlight.CtrlRelease();
					iCrntCtrl = CTRL_MANUAL;

					iState = STATE_DETECT;

					cSerial.SetLight(FRMDAT_LIGHT_LEFT, LGCLR_RED);
					cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_RED);
				}
			}
		}
		break;

	case GEAR_DN: // Execute control or manual attack
		if (bManualAttack)
		{
			bManualAttack = false;
			
			if (iAttack >= 10)
			{
				sprintf(szOpTxt, "(Manual control) Attack count over 10!");
				cLog << szOpTxt << endl;
				cLog.pVideolog->AddText(szOpTxt);
			}
			else
			{
				cSerial.SetMotor(cLog.dGolf[iAttack]);

				sprintf(szOpTxt, "(Manual control) Attack now, num: %d", iAttack);
				cLog << szOpTxt << endl;
				cLog.pVideolog->AddText(szOpTxt);
				
				iAttack++;
			}
		}
		else
		{
			if (CTRL_EXECUTE != iCrntCtrl)
			{
				cFlight.CtrlObtain();
				iCrntCtrl = CTRL_EXECUTE;

				switch (iDetect)
				{
				case STATE_ERROR:
					{
						cSerial.SetLight(FRMDAT_LIGHT_LEFT, LGCLR_RED);
						cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_NONE);
					}
					break;

				case STATE_ATTACK:
					{
						iState = STATE_ATTACK;

						cSerial.SetLight(FRMDAT_LIGHT_LEFT, LGCLR_GREEN);
						cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_GREEN);
					}
					break;

				case STATE_LAND:
					{
						iState = STATE_LAND;

						cSerial.SetLight(FRMDAT_LIGHT_LEFT, LGCLR_BLUE);
						cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_BLUE);
					}
					break;
				}
			}
		}
		break;
	}

	#else
	// Dial control debug turn on

	static int iGolf = 0;
	static int iLastGear = GEAR_UP;
	switch (iGear)
	{
	case GEAR_UP:
	case GEAR_DN:
		if (iLastGear != iGear)
		{
			if (iGolf < 10)
			{
				cSerial.SetMotor(cLog.dAttackMotorAngle[iGolf]);
				iGolf++;
			}
			else
			{
				cSerial.SetLight(FRMDAT_LIGHT_LEFT, LGCLR_YELLOW);
			}
		}
		break;
	}

	#endif
	
	iLastGear = iGear;
}

int RMTask(ConboardSDKScript & sdkScript)
{
	CFlightIndoor cAircraft(&sdkScript);
	//cAircraft.CtrlObtain();
	//cAircraft.Takeoff();

	cAircraft.SetCameraAngle(90, 0, -90);

	//extern double dPX;
	//extern double dIX;
	//extern double dDX;
	//extern double dPY;
	//extern double dIY;
	//extern double dDY;

	bool Processing(CFlightIndoor&);
	while (Processing(cAircraft))
	{
		GearController(cAircraft);

		char cKey = waitKey(1);

		if ('q' == cKey || 'Q' == cKey) // Force quit
		{
			cLog << "@ Force quit!" << endl;
			break;
		}

		switch (cKey) // Key event
		{
		case ',':
			iExposureValue -= 10;
			break;
		
		case '.':
			iExposureValue += 10;
			break;
		
		case '<':
			iExposureValue -= 100;
			break;
		
		case '>':
			iExposureValue += 100;
			break;
		
		case '/':
			bExposureAuto = true;
			cCamera.SetExposure(bExposureAuto, iExposureValue);
			break;
		
		case '?':
			bExposureAuto = false;
			cCamera.SetExposure(bExposureAuto, iExposureValue);
			break;
			
		/*case 'a':
			dPX += 0.01;
			break;
			
		case 's':
			dIX += 0.01;
			break;
		
		case 'd':
			dDX += 0.01;
			break;

		case 'z':
			dPX -= 0.01;
			break;
		
		case 'x':
			dIX -= 0.01;
			break;

		case 'c':
			dDX -= 0.01;
			break;
			
		case 'f':
			dPY += 0.01;
			break;
			
		case 'g':
			dIY += 0.01;
			break;
		
		case 'h':
			dDY += 0.01;
			break;

		case 'v':
			dPY -= 0.01;
			break;
		
		case 'b':
			dIY -= 0.01;
			break;

		case 'n':
			dDY -= 0.01;
			break;*/
		}
	}

	cAircraft.Land();
	cAircraft.CtrlRelease();

	return 0;
}

char * FlashStateString(int iState)
{
	static char szState[][16] =
	{
		"STATE_ERROR",
		"STATE_DETECT",
		"STATE_LAND",
		"STATE_ATTACK"
	};

	return szState[iState];
}

void AdjustExposure(int iState)
{
	static int iLastState = STATE_DETECT;

	if (iState != iLastState)
	{
		switch(iState)
		{
		case STATE_DETECT:
		case STATE_ERROR:
			bExposureAuto = true;
			cCamera.SetExposure(bExposureAuto, iExposureValue);
			break;

		case STATE_LAND:
			bExposureAuto = true;
			cCamera.SetExposure(bExposureAuto, iExposureValue);
			break;

		case STATE_ATTACK:
			//bExposureAuto = false;
			bExposureAuto = true;
			cCamera.SetExposure(bExposureAuto, iExposureValue);
			break;
		}
	}

	iLastState = iState;
}

bool Processing(CFlightIndoor & cFlgt)
{
	#if DEBUG_DIALCTR // Dial control debug, do not fly or detect
	usleep(10000); // 10ms
	return true;
	#endif

	AdjustExposure(iState); // Adjust exposure to different environment

	static long long llProcCounter = 0;
	llProcCounter++;

	sprintf(szOpTxt,
		"@ Current: %lld... with State: %s <Exposure: %s %d>",
		llProcCounter, FlashStateString(iState),
		bExposureAuto ? "A" : "M", iExposureValue);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	if (!cCamera.Grab())
	{
		cSerial.SetLight(FRMDAT_LIGHT_RIGHT, LGCLR_YELLOW);

		sprintf(szOpTxt, "Grab camera data error!");
		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);
	}
	
	mFrmRaw = cCamera.GetRaw();              // 640 * 480, color
	mFrmDec = cCamera.GetDec();              // 320 * 240, color
	cvtColor(mFrmDec, mGryDec, CV_BGR2GRAY); // 320 * 240, gray
	pyrDown(mGryDec, mGryPyr);               // 160 * 120, gray

	cLog.pVideolog->AddFrame(0, 0, mGryDec); // Original gray image

	// Save raw
	
	if (cLog.bSaveRaw)
	{
		static bool bInited = false;
		
		static VideoWriter * pVideo = nullptr;
		
		if (!bInited) // Have not init
		{
			char szRaw[32] = "./Runlog/Raw";
			sprintf(szRaw, "%s%d.avi", szRaw, cLog.iLogCounter);
		
			pVideo = new VideoWriter(szRaw, CV_FOURCC('D', 'I', 'V', 'X'), 20,
				Size(640, 480), true);
				
			bInited = true;
		}
		
		if (pVideo)
		{
			(*pVideo) << mFrmRaw;
		}
	}

	if (CTRL_MANUAL == iCrntCtrl) // Manual control
	{
		sprintf(szOpTxt, "Manual controlling...");
		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);
	}

	int Detect();
	int Land(CFlightIndoor & cFlgt);
	int Attack(CFlightIndoor & cFlgt);

	try
	{
		switch (iState)
		{
		case STATE_DETECT:
		case STATE_ERROR:
			if (Detect())
			{
				return false;
			}
			break;

		case STATE_LAND:
			if (Land(cFlgt))
			{
				return false;
			}
			break;

		case STATE_ATTACK:
			if (Attack(cFlgt))
			{
				return false;
			}
			break;
		}
	}
	catch (cv::Exception e)
	{
		sprintf(szOpTxt, "!!!! Catched cv exception !!!!");
		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);
	}
	catch (...)
	{
		sprintf(szOpTxt, "!!!! Catched unknown exception !!!!");
		cLog << szOpTxt << endl;
		cLog.pVideolog->AddText(szOpTxt);
	}

	static double dLastTimer = cLog.Timer();

	double dCrntTimer = cLog.Timer();
	
	double dUsedTime = dCrntTimer - dLastTimer;
	dLastTimer = dCrntTimer;

	sprintf(szOpTxt, "Used time:%lf, FPS:%lf", dUsedTime, 1000 / dUsedTime);
	cLog << szOpTxt << endl;
	cLog.pVideolog->AddText(szOpTxt);

	cLog.pVideolog->Flash();

	cLog << endl;
	return true;
}	
		
