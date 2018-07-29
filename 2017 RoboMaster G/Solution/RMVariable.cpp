// RMVariable.cpp
// Version 2.1
// Started by Deyou Kong, 2017-07-20
// Checked by Deyou Kong, 2017-07-20
// Updated by Deyou Kong, 2017-07-20

#include "RMTask.h"

CLogger cLog("RMTask");

CCamera cCamera;
const float fRawFX = 462.2708;
const float fRawFY = 460.3916;
const float fRawPX = 310.8862;
const float fRawPY = 221.2809;

CApriltag cTag("tag16h5", 160, 120,
	fRawFX / 4, fRawFY / 4, fRawPX / 4, fRawPY / 4,
	cLog.dTagSize);

CArmor cArmor;

const float TLandmark::fFX = fRawFX / 2;
const float TLandmark::fFY = fRawFY / 2;
const float TLandmark::fPX = fRawPX / 2;
const float TLandmark::fPY = fRawPY / 2;
const int TLandmark::iLmHghMaxLineGapInit = 20;
TLandmark::TLandmark()
{
	iLmCnySml = 150;
	iLmCnyLrg = 300;

	iLmHghThreshold = 60;
	iLmHghMinLineLength = 35;
	iLmHghMaxLineGap = iLmHghMaxLineGapInit;

	iLmHCMinDistCircles = 35;
	iLmHCCnyLrgThreshold = iLmCnyLrg;
	iLmHCAccumThreshold = 50;
	iLmHCMinR = 20;
	iLmHCMaxR = 120;
}
TLandmark tLand;

CSerial cSerial;

int iCrntCtrl = CTRL_EXECUTE;

bool bExposureAuto = true;
int iExposureValue = 900;

int iState = STATE_DETECT;
//int iState = STATE_ATTACK; // Use for debug attack

int iDetect = STATE_ERROR;

char szOpTxt[64];

Mat mFrmRaw;
Mat mFrmDec;
Mat mGryDec;
Mat mGryPyr;
