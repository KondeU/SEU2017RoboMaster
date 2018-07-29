// RMTask.h
// Version 2.1
// Started by Deyou Kong, 2017-07-14
// Checked by Deyou Kong, 2017-07-14
// Updated by Deyou Kong, 2017-07-20

#pragma once

#include <opencv2/opencv.hpp>
using namespace cv;

#include "Logger.h"
#include "FlightIndoor.h"
#include "Camera.h"
#include "Apriltag.h"
#include "Armor.h"
#include "Serial.h"

// Golf attack test debug if true
#define DEBUG_GOLFATK true
// Landing test debug if true
#define DEBUG_LANDING true
// Dial control debug if true
// It will use gear to leave golf fall down, note that be careful!
// Do not take off or debug other test if it have been turned on!
#define DEBUG_DIALCTR false

// RMTask logger, save video and output logs
extern CLogger cLog;

// Camera providder, get camera data
extern CCamera cCamera;
extern const float fRawFX;
extern const float fRawFY;
extern const float fRawPX;
extern const float fRawPY;

// The vision algorithm object of detecting apriltags
extern CApriltag cTag;

// The vision algorithm object of detecting armor
extern CArmor cArmor;
// Also, the detecting landmark vision algorithm is embedded in main codes,
// use old version codes
//struct TLandmark;
extern struct TLandmark
{
	// Camera params
	
	static const float fFX;
	static const float fFY;
	static const float fPX;
	static const float fPY;

	// Canny factors

	int iLmCnySml;
	int iLmCnyLrg;

	// Hough Line factors

	int iLmHghThreshold;
	int iLmHghMinLineLength;
	int iLmHghMaxLineGap;
	static const int iLmHghMaxLineGapInit;

	// Hough Circle factors

	int iLmHCMinDistCircles;
	int iLmHCCnyLrgThreshold;
	int iLmHCAccumThreshold;
	int iLmHCMinR;
	int iLmHCMaxR;
	
	TLandmark(); // Init with default value
} tLand;

// Serial communications
extern CSerial cSerial;

#define CTRL_MANUAL  0
#define CTRL_EXECUTE 1

extern int iCrntCtrl; // Current flight controller

extern bool bExposureAuto; // Camera auto exposure
extern int iExposureValue; // Camera exposure value

#define STATE_ERROR  0
#define STATE_DETECT 1
#define STATE_LAND   2
#define STATE_ATTACK 3

// Current state
extern int iState;
// Detect state, it is STATE_ATTACK if found tags, it is STATE_LAND if found circles
// It is STATE_ERROR if none is detected
extern int iDetect;

extern char szOpTxt[64]; // Output text, almost used in CLogger to output logs

extern Mat mFrmRaw; // 640 * 480, color
extern Mat mFrmDec; // 320 * 240, color
extern Mat mGryDec; // 320 * 240, gray
extern Mat mGryPyr; // 160 * 120, gray
