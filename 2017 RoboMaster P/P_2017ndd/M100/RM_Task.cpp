// RM_Task.cpp
// Version 2.0
// Writed by Deyou Kong, 2017-05-13
// Checked by Deyou Kong, 2017-05-18


#include "Headers.h"


#ifdef MODE_DATATRANSPARENT
void RM_DataTransparent(void * pData)
{
	return;
}
#endif


#ifdef MODE_AUTOSTART
ofstream fout("./autorun/2017RobomasterTask/Runlog/RM_Task.log");
#else
ofstream fout("./Runlog/RM_Task.log");
#endif


#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
CVideolog cVideolog(CGuidance::iHeight, CGuidance::iWidth);
#endif


char szOpTxt[64]; // Output text, almost used in function CVideolog::AddText


struct TFactor
{
	// Canny factors

	int iLmCnySml;
	int iLmCnyLrg;

	// Hough Line factors

	int iLmHghThreshold;
	int iLmHghMinLineLength;
	int iLmHghMaxLineGap;
	const int iLmHghMaxLineGapInit = 20;

	// Hough Circle factors

	int iLmHCMinDistCircles;
	int iLmHCCnyLrgThreshold;
	int iLmHCAccumThreshold;
	int iLmHCMinR;
	int iLmHCMaxR;

	// Guidance expected brightness

	int iGuidExpectedBrightness;

	TFactor() // Init with default value
	{
		iLmCnySml = 150; ///200;
		iLmCnyLrg = 300; //365;

		iLmHghThreshold = 60;
		iLmHghMinLineLength = 35; // 40;
		iLmHghMaxLineGap = iLmHghMaxLineGapInit;

		iLmHCMinDistCircles = 35;
		iLmHCCnyLrgThreshold = iLmCnyLrg;
		iLmHCAccumThreshold = 50;
		iLmHCMinR = 20;
		iLmHCMaxR = 120;

		iGuidExpectedBrightness = 100; // Factory:50; // Outdoor:60; // Lab:120;
	}
} tFactor; // Variable factor


#define STATE_ERROR   0
#define STATE_SUCCESS 1
#define STATE_START   2
#define STATE_NORMALV 3
#define STATE_PREGAP  4
#define STATE_GAPING  5
#define STATE_PRECHNG 6
#define STATE_CHNGING 7
#define STATE_NORMALH 8
#define STATE_PRELAND 9
#define STATE_LANDING 10

#define STATE_PROC_BEG 2
#define STATE_PROC_SVH 6
#define STATE_PROC_SEG 8
#define STATE_PROC_END 10

int iState = STATE_START; // Current state, just change it in function Process, use it carefully
int iErrLastState = STATE_START; // If occur an error, it will store last normal state

char * FlashStateString(int iState)
{
	static char szState[][16] =
	{
		"STATE_ERROR",
		"STATE_SUCCESS",

		"STATE_START",
		"STATE_NORMALV",
		"STATE_PREGAP",
		"STATE_GAPING",
		"STATE_PRECHNG",
		"STATE_CHNGING",
		"STATE_NORMALH",
		"STATE_PRELAND",
		"STATE_LAND"
	};

	return szState[iState];
}


#define CTRL_MANUAL  0
#define CTRL_EXECUTE 1

int iCrntCtrl = CTRL_EXECUTE; // Current flight controller

void GearController(CFlight & cFlight, int iGear)
{
	switch (iGear)
	{
	case GEAR_UP: // Manual control
		if (CTRL_MANUAL != iCrntCtrl)
		{
			cFlight.CtrlRelease();
			iCrntCtrl = CTRL_MANUAL;

			if (STATE_ERROR == iState)
			{
				iState = iErrLastState; // Restore the last normal state to iState
			}
		}
		break;

	case GEAR_DN: // Execute control
		if (CTRL_EXECUTE != iCrntCtrl)
		{
			cFlight.CtrlObtain();
			iCrntCtrl = CTRL_EXECUTE;
		}
		break;

	case GEAR_ERR:
		if (CTRL_MANUAL != iCrntCtrl)
		{
			cFlight.CtrlRelease();
			iCrntCtrl = CTRL_MANUAL;
		}
		break;
	}
}


// X3 image

#ifdef MODE_X3TRANS
Mat mX3;
#endif


// Guidance image

Mat mDownGreyL(CGuidance::iHeight, CGuidance::iWidth, CV_8UC1);
//Mat mDownGreyR(CGuidance::iHeight, CGuidance::iWidth, CV_8UC1);


// Guidance data

short sGuidDist;
float fGuidFocal;
float fGuidCU;
float fGuidCV;


// Functions declarration: Task

bool Processing(CFlightIndoor & cFlgt, CGuidance & cGuid);
void LimittingHeight(double & dCtrlHeight, short sUltrasonicDist, CFlight & cFlgt);


// Functions declarration: Naviline

#define FUNC_EL_LEFT   0
#define FUNC_EL_RIGHT  1
#define FUNC_EL_CENTER 2
#define FUNC_EL_VALID  3
#define FUNC_EL_SUB    4

#define FUNC_EL_VALID_FAIL 0
#define FUNC_EL_VALID_TRUE 1
#define FUNC_EL_VALID_LARG 2

#define FUNC_AL_BAD_BADFRAME 0
#define FUNC_AL_BAD_SUCCESS  1

#define FUNC_AL_MOVE_CENTER 0
#define FUNC_AL_MOVE_LEFT   -1
#define FUNC_AL_MOVE_RIGHT  1

int FindThrSeg(Mat & mProc);
void ExtractLinesV(Mat & mProc, int iExtLines[CGuidance::iHeight][5]);
void AdjustLinesV(int iExtLines[CGuidance::iHeight][5], int iMaxOffset = 40);
void AnalyseLinesV(int iExtLines[CGuidance::iHeight][5], int & iBad, int & iMove);
void CalcAlphaOffsetV(int iExtLines[CGuidance::iHeight][5], double & dAlpha, double & dOffset, Mat * pMatShow = nullptr);
void FlyingAdjustV(int iExtLines[CGuidance::iHeight][5], double dCtrlHeight, CFlightIndoor & cFlgt, Mat * pMatShow = nullptr);
void ExtractLinesH(Mat & mProc, int iExtLines[CGuidance::iWidth][5]);
void AdjustLinesH(int iExtLines[CGuidance::iWidth][5], int iMaxOffset = 40);
void AnalyseLinesH(int iExtLines[CGuidance::iWidth][5], int & iBad);
void CalcAlphaOffsetH(int iExtLines[CGuidance::iWidth][5], double & dAlpha, double & dOffset, Mat * pMatShow = nullptr);
void FlyingAdjustH(int iExtLines[CGuidance::iWidth][5], double dCtrlHeight, CFlightIndoor & cFlgt, Mat * pMatShow = nullptr);


// Functions declarration: Landing

double CalcPointDistance(Point pnt1, Point pnt2);
bool Landing(double dOffsetX, double dOffsetY, double dHeight, CFlightIndoor & cFlgt);
void AddLine(Mat & mImg, Point pnt1, Point pnt2, int iAddValue = 1);
void FindCross(Mat & mImg, vector<Point> & vcPntCross, int iThreshold = 1);
void DrawCircles(Mat & mImg, vector<Point> & vcPntCross);
void PixelToReal(float fFocal, float fCU, float fCV, short sDist, int iPixX, int iPixY, double & dRealX, double & dRealY);
void AdjustHghLineGap();


// Flight task

int RM_Task(ConboardSDKScript & sdkScript)
{
	char cKey = -1; // Keyboard message

	#ifdef MODE_DELAYSTART
	static const int iDelayUnit = 20000;
	static const int iFrequency = 1000000 / iDelayUnit;
	static const int iDelayTime = 20;
	for (int i = 0; i < iDelayTime * iFrequency; i++)
	{
		cKey = waitKey(1);

		if ('q' == cKey || 'Q' == cKey) // Force quit
		{
			cout << "@ Force quit in start delay!" << endl;
			fout << "@ Force quit in start delay!" << endl;
			return 1;
		}

		if (!(i % iFrequency))
		{
			cout << "@ Start delay enable with: " << (i / iFrequency) << endl;
			fout << "@ Start delay enable with: " << (i / iFrequency) << endl;
		}

		usleep(iDelayUnit);
	}
	#endif

	
	CGuidance cGuidance;
	CFlightIndoor cAircraft(&sdkScript);

	#ifdef MODE_X3TRANS
	CX3 cX3;
	cX3.Init(mX3);
	#endif

	cGuidance.Init();

	cAircraft.CtrlObtain();
	//cAircraft.Takeoff();

	cAircraft.SetCameraAngle(0, 0, -40); // X3 camera view angle
	sleep(3);

	// Processing begin

	while (Processing(cAircraft, cGuidance))
	{
		GearController(cAircraft, cAircraft.GetGear());

		cKey = waitKey(1);

		if ('q' == cKey || 'Q' == cKey) // Force quit
		{
			cout << "@ Force quit!" << endl;
			fout << "@ Force quit!" << endl;
			break;
		}

		switch (cKey) // Key event
		{
		case 'A':
			tFactor.iLmCnySml++;
			break;

		case 'a':
			tFactor.iLmCnySml--;
			break;

		case 'Z':
			tFactor.iLmCnyLrg++;
			break;

		case 'z':
			tFactor.iLmCnyLrg--;
			break;

		case 'S':
			tFactor.iLmHghThreshold++;
			break;

		case 's':
			tFactor.iLmHghThreshold--;
			break;

		case 'X':
			tFactor.iLmHghMinLineLength++;
			break;

		case 'x':
			tFactor.iLmHghMinLineLength--;
			break;

		case 'D':
			tFactor.iLmHghMaxLineGap++;
			break;

		case 'd':
			tFactor.iLmHghMaxLineGap--;
			break;

		case 'C':
			tFactor.iLmHCMinDistCircles++;
			break;

		case 'c':
			tFactor.iLmHCMinDistCircles--;
			break;

		case 'F':
			tFactor.iLmHCCnyLrgThreshold++;
			break;

		case 'f':
			tFactor.iLmHCCnyLrgThreshold--;
			break;

		case 'V':
			tFactor.iLmHCAccumThreshold++;
			break;

		case 'v':
			tFactor.iLmHCAccumThreshold--;
			break;

		case 'G':
			tFactor.iLmHCMinR++;
			break;

		case 'g':
			tFactor.iLmHCMinR--;
			break;

		case 'B':
			tFactor.iLmHCMaxR++;
			break;

		case 'b':
			tFactor.iLmHCMaxR--;
			break;

		case 'K':
		case 'k':
			tFactor.iGuidExpectedBrightness++;
			cGuidance.SetExposure(tFactor.iGuidExpectedBrightness);
			break;

		case 'L':
		case 'l':
			tFactor.iGuidExpectedBrightness--;
			cGuidance.SetExposure(tFactor.iGuidExpectedBrightness);
			break;

		default:
		case -1:
			break;
		}
	}

	// Processing end

	cAircraft.Land();
	cAircraft.CtrlRelease();

	cGuidance.Deinit();

	#ifdef MODE_X3TRANS
	cX3.Deinit();
	#endif

	return 0;
}


// Task processing

bool Processing(CFlightIndoor & cFlgt, CGuidance & cGuid)
{
	static long long llProcCounter = 0;
	llProcCounter++;

	sprintf(szOpTxt, "@ Current: %lld... with State: %s <Brightness: %d>",
		llProcCounter, FlashStateString(iState), tFactor.iGuidExpectedBrightness);
	cout << szOpTxt << endl;
	fout << szOpTxt << endl;
	#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
	cVideolog.AddText(szOpTxt);
	#endif

	// Height should be limited between 1.8m and 2.2m except state is STATE_LANDING

	double dCtrlHeight = 2.0;
	LimittingHeight(dCtrlHeight, sGuidDist, cFlgt);

	// Obtain guidance image, only use the down left camera
	// Max is 20Hz, so donot need call usleep(..) function

	cGuid.GetData();

	#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
	cVideolog.AddFrame(0, 0, mDownGreyL);
	#endif

	if (CTRL_MANUAL == iCrntCtrl) // Manual control
	{
		sprintf(szOpTxt, "Manual controlling...");
		cout << szOpTxt << endl;
		fout << szOpTxt << endl;
		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddText(szOpTxt);
		#endif

		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		if ((iState > STATE_PROC_SEG) && (iState <= STATE_PROC_END))
		{
			Mat mProc = mDownGreyL.clone();
			Canny(mProc, mProc, tFactor.iLmCnySml, tFactor.iLmCnyLrg);

			cVideolog.AddFrame(0, 1, mProc);
		}
		#endif

		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.Flash();
		#endif

		return true;
	}

	static int iTraverse = FUNC_AL_MOVE_CENTER; // H. Move direction

	if (iState <= STATE_PROC_BEG)
	{
		switch (iState)
		{
			case STATE_ERROR: // Occur an error, flight will stay still
			{
				sprintf(szOpTxt, "Last normal state is: %s", FlashStateString(iErrLastState));
				cout << szOpTxt << endl;
				fout << szOpTxt << endl;
				#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
				cVideolog.AddText(szOpTxt);
				#endif
			}
			break;

			case STATE_SUCCESS:
			{
				cFlgt.Land();

				// In order to make sure remote is able to control flight
				iErrLastState = STATE_LANDING;
				iState = STATE_ERROR;
			}
			break;

			case STATE_START:
			{
				cFlgt.SetCameraAngle(0, 0, 0); // X3 camera view angle
				
				#ifdef MODE_AUTOSTART
				sprintf(szOpTxt, "Used auto start!");
				cout << szOpTxt << endl;
				fout << szOpTxt << endl;
				#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
				cVideolog.AddText(szOpTxt);
				#endif
				#endif

				#ifndef MODE_FLIGHTMOVE
				sprintf(szOpTxt, "Used flight no move!");
				cout << szOpTxt << endl;
				fout << szOpTxt << endl;
				#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
				cVideolog.AddText(szOpTxt);
				#endif
				#endif
				
				#if defined MODE_DEBUG_MODEL_NAVILINE
				iState = STATE_NORMALV;
				sprintf(szOpTxt, "Enable DEBUG of Naviline");
				cout << szOpTxt << endl;
				fout << szOpTxt << endl;
				#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
				cVideolog.AddText(szOpTxt);
				#endif
				#elif defined MODE_DEBUG_MODEL_LANDMARK
				iState = STATE_LANDING;
				sprintf(szOpTxt, "Enable DEBUG of Landmark");
				cout << szOpTxt << endl;
				fout << szOpTxt << endl;
				#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
				cVideolog.AddText(szOpTxt);
				#endif
				#else
				iState = STATE_NORMALV;
				sprintf(szOpTxt, "Normal task start");
				cout << szOpTxt << endl;
				fout << szOpTxt << endl;
				#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
				cVideolog.AddText(szOpTxt);
				#endif
				#endif
			}
			break;
		}
	}
	else if (iState <= STATE_PROC_SEG)
	{
		// Image processing

		Mat mProc = mDownGreyL.clone();

		Mat mShow = Mat::zeros(mProc.rows, mProc.cols, mProc.type());
		line(mShow, Point(0, CGuidance::iHeight / 2), Point(CGuidance::iWidth, CGuidance::iHeight / 2), Scalar(60));
		line(mShow, Point(CGuidance::iWidth / 2, 0), Point(CGuidance::iWidth / 2, CGuidance::iHeight), Scalar(60));

		medianBlur(mProc, mProc, 5); // Median blur to make image smooth

		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddFrame(0, 1, mProc);
		#endif
		
		int iThrSeg = FindThrSeg(mProc); // Find threshold segmention
		if (iThrSeg < 0) // Loop over high limit
		{
			sprintf(szOpTxt, "FindThrSeg loop over high limit");
			cout << szOpTxt << endl;
			fout << szOpTxt << endl;
			#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
			cVideolog.AddText(szOpTxt);
			#endif
		
			return true;
		}
		iThrSeg += (255 - iThrSeg) / 10; // Add a check factor

		sprintf(szOpTxt, "Threshold segmention value: %d", iThrSeg);
		cout << szOpTxt << endl;
		fout << szOpTxt << endl;
		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddText(szOpTxt);
		#endif

		threshold(mProc, mProc, iThrSeg, 255, THRESH_BINARY); // Threshold
		morphologyEx(mProc, mProc, MORPH_OPEN, Mat(5, 5, CV_8U, Scalar(1))); // Open operate

		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddFrame(1, 0, mProc);
		#endif

		static double dTimerChange = 0;
		static double dTimerGap    = 0;

		static int iBad  = FUNC_AL_BAD_BADFRAME;
		static int iMove = FUNC_AL_MOVE_CENTER;

		if (iState <= STATE_PROC_SVH) // Vertical
		{
			int iExtLines[CGuidance::iHeight][5] = { 0 }; // Lines edge stored array

			ExtractLinesV(mProc, iExtLines); // Extract lines edge
			AdjustLinesV(iExtLines, 10); // Adjust lines to ignore the large-offset rows
			AnalyseLinesV(iExtLines, iBad, iMove); // Analyse if a move, a bad frame or normal

			sprintf(szOpTxt, "Bad: %s, Move: %d", (FUNC_AL_BAD_SUCCESS == iBad ? "SUC" : "BAD"), iMove);
			cout << szOpTxt << endl;
			fout << szOpTxt << endl;
			#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
			cVideolog.AddText(szOpTxt);
			#endif

			for (int i = 0; i < CGuidance::iHeight; i++)
			{
				if (iExtLines[i][FUNC_EL_VALID] == FUNC_EL_VALID_TRUE)
				{
					mShow.at<uchar>(i, iExtLines[i][FUNC_EL_CENTER]) = 120;
				}
			}

			switch (iState)
			{
				case STATE_NORMALV:
				{
					if (FUNC_AL_MOVE_CENTER != iMove)
					{
						iTraverse = iMove;
						iState = STATE_PRECHNG;
					}
					else
					{
						if (FUNC_AL_BAD_BADFRAME == iBad)
						{
							iState = STATE_PREGAP;
						}
						else // FUNC_AL_BAD_SUCCESS == iBad
						{
							FlyingAdjustV(iExtLines, dCtrlHeight, cFlgt, &mShow);
						}
					}
				}
				break;

				case STATE_PREGAP:
				{
					static int iPreGapFrm = 0;

					sprintf(szOpTxt, "PreGap frame count: %d", iPreGapFrm);
					cout << szOpTxt << endl;
					fout << szOpTxt << endl;
					#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
					cVideolog.AddText(szOpTxt);
					#endif

					if (iPreGapFrm < 10)
					{
						if (FUNC_AL_BAD_SUCCESS == iBad)
						{
							iState = STATE_NORMALV;
							iPreGapFrm = 0;
						}
						else
						{
							iPreGapFrm++;
						}
					}
					else
					{
						dTimerGap = cGuid.Timer();

						sprintf(szOpTxt, "Gap time begin: %lf", dTimerGap);
						cout << szOpTxt << endl;
						fout << szOpTxt << endl;
						#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
						cVideolog.AddText(szOpTxt);
						#endif

						iState = STATE_GAPING;

						iPreGapFrm = 0;
					}
				}
				break;

				case STATE_GAPING:
				{
					double dTimeout = cGuid.Timer() - dTimerGap;

					sprintf(szOpTxt, "Gaping times: %lf", dTimeout);
					cout << szOpTxt << endl;
					fout << szOpTxt << endl;
					#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
					cVideolog.AddText(szOpTxt);
					#endif

					if ((dTimeout > 3000) || (FUNC_AL_BAD_SUCCESS == iBad))
					{
						iState = STATE_NORMALV;
					}
					else
					{
						cFlgt.IdrFlying(0.2, 0, dCtrlHeight, 0);
					}
				}
				break;

				case STATE_PRECHNG:
				{
					static int iPreMoveFrm = 0;

					sprintf(szOpTxt, "PreChange frame count: %d", iPreMoveFrm);
					cout << szOpTxt << endl;
					fout << szOpTxt << endl;
					#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
					cVideolog.AddText(szOpTxt);
					#endif

					if (iPreMoveFrm < 3)
					{
						if (iMove != iTraverse)
						{
							iState = STATE_NORMALV;
							iPreMoveFrm = 0;
							iTraverse = FUNC_AL_MOVE_CENTER;
						}
						else
						{
							iPreMoveFrm++;
						}
					}
					else
					{
						dTimerChange = cGuid.Timer();

						sprintf(szOpTxt, "Move time begin: %lf", dTimerChange);
						cout << szOpTxt << endl;
						fout << szOpTxt << endl;
						#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
						cVideolog.AddText(szOpTxt);
						#endif

						iState = STATE_CHNGING;

						iPreMoveFrm = 0;
					}
				}
				break;
			}
		}
		else // Horizontal
		{
			transpose(mProc, mProc);
			flip(mProc, mProc, 0);
			transpose(mShow, mShow);
			flip(mShow, mShow, 0);

			int iExtLines[CGuidance::iWidth][5] = { 0 }; // Lines edge stored array

			ExtractLinesH(mProc, iExtLines); // Extract lines edge
			AdjustLinesH(iExtLines, 10); // Adjust lines to ignore the large-offset rows
			AnalyseLinesH(iExtLines, iBad); // Analyse if a bad frame or normal

			sprintf(szOpTxt, "Bad: %s", (FUNC_AL_BAD_SUCCESS == iBad ? "SUCCESS" : "BAD"));
			cout << szOpTxt << endl;
			fout << szOpTxt << endl;
			#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
			cVideolog.AddText(szOpTxt);
			#endif

			for (int i = 0; i < CGuidance::iWidth; i++)
			{
				if (iExtLines[i][FUNC_EL_VALID] == FUNC_EL_VALID_TRUE)
				{
					mShow.at<uchar>(i, iExtLines[i][FUNC_EL_CENTER]) = 120;
				}
			}

			switch (iState)
			{
				case STATE_CHNGING:
				{
					double dTimeout = cGuid.Timer() - dTimerChange;

					sprintf(szOpTxt, "Changing times: %lf", dTimeout);
					cout << szOpTxt << endl;
					fout << szOpTxt << endl;
					#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
					cVideolog.AddText(szOpTxt);
					#endif

					if ((dTimeout > 3000) || (FUNC_AL_BAD_SUCCESS == iBad))
					{
						iState = STATE_NORMALH;
					}
					else
					{
						cFlgt.IdrFlying(0.05, iTraverse * 0.2, dCtrlHeight, 0);//
					}
				}
				break;

				case STATE_NORMALH:
				{
					if (FUNC_AL_BAD_BADFRAME == iBad)
					{
						iState = STATE_PRELAND;
					}
					else // FUNC_AL_BAD_SUCCESS == iBad
					{
						FlyingAdjustH(iExtLines, dCtrlHeight, cFlgt, &mShow);
					}				
				}
				break;
			}

			transpose(mProc, mProc);
			flip(mProc, mProc, 1);
			transpose(mShow, mShow);
			flip(mShow, mShow, 1);
		}

		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddFrame(1, 1, mShow);
		#endif
	}
	else if (iState <= STATE_PROC_END)
	{
		// Image processing
		
		Mat mProc;
		
		Mat mShowLine;
		Mat mShowCircle;
		
		try
		{
		mProc = mDownGreyL.clone();
		}
		catch(cv::Exception)
		{
		fout << "CV EXCEPTION - CLONE"<< endl;
		cout << "CV EXCEPTION - CLONE"<< endl;
		}
		
		try
		{
		mShowLine   = Mat::zeros(mProc.rows, mProc.cols, mProc.type());
		mShowCircle = Mat::zeros(mProc.rows, mProc.cols, mProc.type());
		}
		catch(cv::Exception)
		{
		fout << "CV EXCEPTION - ZERO"<< endl;
		cout << "CV EXCEPTION - ZERO"<< endl;
		}

		try
		{
		line(mShowLine, Point(0, CGuidance::iHeight / 2), Point(CGuidance::iWidth, CGuidance::iHeight / 2), Scalar(60));
		line(mShowLine, Point(CGuidance::iWidth / 2, 0), Point(CGuidance::iWidth / 2, CGuidance::iHeight), Scalar(60));
		line(mShowCircle, Point(0, CGuidance::iHeight / 2), Point(CGuidance::iWidth, CGuidance::iHeight / 2), Scalar(60));
		line(mShowCircle, Point(CGuidance::iWidth / 2, 0), Point(CGuidance::iWidth / 2, CGuidance::iHeight), Scalar(60));
		}
		catch(cv::Exception)
		{
		fout << "CV EXCEPTION - DRAWLINE"<< endl;
		cout << "CV EXCEPTION - DRAWLINE"<< endl;		
		}

		// Processing lines
		
		try
		{
		Canny(mProc, mProc, tFactor.iLmCnySml, tFactor.iLmCnyLrg); // Processing circle also use the canny image
		}
		catch(cv::Exception)
		{
		fout << "CV EXCEPTION - CANNY"<< endl;
		cout << "CV EXCEPTION - CANNY"<< endl;			
		}

		vector<Vec4i> vLines, vLinesAdd; // vLines is probably lines, vLinesAdd is confirmed lines
		
		try
		{
		HoughLinesP(mProc, vLines, 1, CV_PI / 180, tFactor.iLmHghThreshold,
			tFactor.iLmHghMinLineLength, tFactor.iLmHghMaxLineGap);
			
		fout << int(mProc.empty()) << endl;//
		}
		catch(cv::Exception)
		{
		fout << "CV EXCEPTION - HGHLINEP"<< endl;
		cout << "CV EXCEPTION - HGHLINEP"<< endl;		
		}

		Mat mAddLines;
		
		try
		{
		mAddLines = Mat::zeros(mProc.rows, mProc.cols, mProc.type());
		}
		catch(cv::Exception)
		{
		fout << "CV EXCEPTION - ADDLINE"<< endl;
		cout << "CV EXCEPTION - ADDLINE"<< endl;
		}


		vector<Point> vPntCross;//
		Point pntLineCrossCenter(-1, -1);//
		try
		{
		for (int i = 0; i < vLines.size(); i++)
		{
			bool bAdd = true;

			Vec4i v4iLine = vLines[i];
			Point pntLine1(v4iLine[0], v4iLine[1]), pntLine2(v4iLine[2], v4iLine[3]);
			double dDistLine = CalcPointDistance(pntLine1, pntLine2);

			for (int iCheck = 0; iCheck < vLinesAdd.size(); iCheck++)
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

				Point pntSml((v4iSml[0] + v4iSml[2]) / 2, (v4iSml[1] + v4iSml[3]) / 2);
				double dLrgK = (double)(v4iLrg[3] - v4iLrg[1]) / (double)(v4iLrg[2] - v4iLrg[0]);
				double dLrgB = (((double)v4iLrg[1] - dLrgK * (double)v4iLrg[0]) + ((double)v4iLrg[3] - dLrgK * (double)v4iLrg[2])) / 2;
				double dDistPointLine = fabs((dLrgK * pntSml.x - pntSml.y + dLrgB) / sqrt(dLrgK * dLrgK + 1));

				if (dDistPointLine < 3)
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

		for (int i = 0; i < vLinesAdd.size(); i++)
		{
			Vec4i v4iAddLine = vLinesAdd[i];
			Point pntAddLine1(v4iAddLine[0], v4iAddLine[1]), pntAddLine2(v4iAddLine[2], v4iAddLine[3]);

			AddLine(mAddLines, pntAddLine1, pntAddLine2);

			line(mProc,     pntAddLine1, pntAddLine2, Scalar(128));
			line(mShowLine, pntAddLine1, pntAddLine2, Scalar(150));
		}

		//vector<Point> vPntCross;
		FindCross(mAddLines, vPntCross);

		DrawCircles(mShowLine, vPntCross);

		//Point pntLineCrossCenter(-1, -1); // Here is the center of line cross
		if ((vPntCross.size() > 1) && (vPntCross.size() < 5)) // Range:[2,4]
		{
			int iCenterX = 0, iCenterY = 0;
			for (int i = 0; i < vPntCross.size(); i++)
			{
				iCenterX += vPntCross[i].x;
				iCenterY += vPntCross[i].y;
			}
			pntLineCrossCenter = Point(iCenterX / vPntCross.size(), iCenterY / vPntCross.size());

			circle(mShowLine, pntLineCrossCenter, 3, Scalar(255));
		}
		}
		catch(cv::Exception)
		{
		fout << "CV EXCEPTION - LINEPROCESS"<< endl;
		cout << "CV EXCEPTION - LINEPROCESS"<< endl;
		}

		// Circles

		vector<Vec3f> vCircles;
		
		try
		{
		HoughCircles(mProc, vCircles, CV_HOUGH_GRADIENT, 1, tFactor.iLmHCMinDistCircles,
			tFactor.iLmHCCnyLrgThreshold, tFactor.iLmHCAccumThreshold,
			tFactor.iLmHCMinR, tFactor.iLmHCMaxR);
		}
		catch(cv::Exception)
		{
		fout << "CV EXCEPTION - HGHCIRCLE"<< endl;
		cout << "CV EXCEPTION - HGHCIRCLE"<< endl;
		}

		Point pntCircleCenter(-1, -1); // Here is the center of circle
		int iCircleCenterR = INT_MAX;
		if ((vCircles.size() > 0) && (vCircles.size() < 4)) // Range:[1,3]
		{
			int iCenterX = 0, iCenterY = 0;
			for (int i = 0; i < vCircles.size(); i++)
			{
				iCenterX += cvRound(vCircles[i][0]);
				iCenterY += cvRound(vCircles[i][1]);
				iCircleCenterR = min(iCircleCenterR, cvRound(vCircles[i][2]));

				circle(mShowCircle, Point(iCenterX, iCenterY), 3, Scalar(150));
				circle(mShowCircle, Point(iCenterX, iCenterY), cvRound(vCircles[i][2]), Scalar(150));
			}
			pntCircleCenter = Point(iCenterX / vCircles.size(), iCenterY / vCircles.size());

			circle(mProc, pntCircleCenter, 3, Scalar(128));
			circle(mProc, pntCircleCenter, iCircleCenterR, Scalar(128));

			circle(mShowCircle, pntCircleCenter, 3, Scalar(255));
			circle(mShowCircle, pntCircleCenter, iCircleCenterR, Scalar(255));
		}

		// Show

		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddFrame(0, 1, mProc);
		cVideolog.AddFrame(1, 0, mShowLine);
		cVideolog.AddFrame(1, 1, mShowCircle);
		#endif

		sprintf(szOpTxt, "Line=> AllLines:%d,ValidLines:%d,Crosses:%d",
			vLines.size(), vLinesAdd.size(), vPntCross.size());
		cout << szOpTxt << endl;
		fout << szOpTxt << endl;
		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddText(szOpTxt);
		#endif

		sprintf(szOpTxt, "Line=> Center(x,y)=(%d,%d)", pntLineCrossCenter.x, pntLineCrossCenter.y);
		cout << szOpTxt << endl;
		fout << szOpTxt << endl;
		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddText(szOpTxt);
		#endif

		sprintf(szOpTxt,
			"L-Factor=> CnySml:%d || CnyLrg:%d",
			tFactor.iLmCnySml, tFactor.iLmCnyLrg);
		cout << szOpTxt << endl;
		fout << szOpTxt << endl;
		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddText(szOpTxt);
		#endif

		sprintf(szOpTxt,
			"L-Factor=> HthThr:%d || MinLineLength:%d || MaxLineGap:%d",
			tFactor.iLmHghThreshold, tFactor.iLmHghMinLineLength, tFactor.iLmHghMaxLineGap);
		cout << szOpTxt << endl;
		fout << szOpTxt << endl;
		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddText(szOpTxt);
		#endif

		sprintf(szOpTxt, "Circles=> Circles:%d", vCircles.size());
		cout << szOpTxt << endl;
		fout << szOpTxt << endl;
		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddText(szOpTxt);
		#endif

		sprintf(szOpTxt, "Circles=> Center(x,y)=(%d,%d)", pntCircleCenter.x, pntCircleCenter.y);
		cout << szOpTxt << endl;
		fout << szOpTxt << endl;
		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddText(szOpTxt);
		#endif

		sprintf(szOpTxt,
			"C-Factor=> MinDist:%d || CnyLrg:%d",
			tFactor.iLmHCMinDistCircles, tFactor.iLmHCCnyLrgThreshold);
		cout << szOpTxt << endl;
		fout << szOpTxt << endl;
		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddText(szOpTxt);
		#endif

		sprintf(szOpTxt,
			"C-Factor=> AccumThr:%d || MinR:%d || MaxR:%d",
			tFactor.iLmHCAccumThreshold, tFactor.iLmHCMinR, tFactor.iLmHCMaxR);
		cout << szOpTxt << endl;
		fout << szOpTxt << endl;
		#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
		cVideolog.AddText(szOpTxt);
		#endif

		switch (iState)
		{
		case STATE_PRELAND:
			{
				tFactor.iGuidExpectedBrightness = 100;
				cGuid.SetExposure(tFactor.iGuidExpectedBrightness);
			
				static int iAccumSeen = 0;

				if (vCircles.size() > 0)// Guidance find a circle, flight will land
				{
					if (iAccumSeen > 3)
					{
						iState = STATE_LANDING;
					}
					else
					{
						iAccumSeen++;
					}
				}
				else
				{
					iAccumSeen = 0;

					cFlgt.IdrFlying(0, iTraverse * 0.1, dCtrlHeight);

					iState = STATE_NORMALH;
				}
			}
			break;

			case STATE_LANDING:
			{	
				static int iLoseTarget = 0;

				bool bValid = true;
				
				// Dymanic hough line gap value
				
				AdjustHghLineGap();

				// Pixel distance -> Real distance
				
				int iPixX = 0, iPixY = 0;
				double dRealX = 0, dRealY = 0;
				
				if (pntLineCrossCenter.x < 0 || pntLineCrossCenter.y < 0)
				{
					if (pntCircleCenter.x < 0 || pntCircleCenter.y < 0)
					{
						bValid = false;
					}
					else
					{
						iPixX = pntCircleCenter.x;
						iPixY = pntCircleCenter.y;
					}
				}
				else
				{
					iPixX = pntLineCrossCenter.x;
					iPixY = pntLineCrossCenter.y; 
				}
				
				// !!! here! just use circle, and < 0.3m will land
				/*if (pntCircleCenter.x < 0 || pntCircleCenter.y < 0)
				{
					bValid = false;
				}
				else
				{
					iPixX = pntCircleCenter.x;
					iPixY = pntCircleCenter.y;
				}*/
				
				if (bValid)
				{
					iLoseTarget = 0;
					
					PixelToReal(fGuidFocal, fGuidCU, fGuidCV, sGuidDist, iPixX, iPixY, dRealX, dRealY);

					sprintf(szOpTxt, "Pixel dist to center (x,y)=(%d,%d)", iPixX, iPixY);
					cout << szOpTxt << endl;
					fout << szOpTxt << endl;
					#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
					cVideolog.AddText(szOpTxt);
					#endif

					sprintf(szOpTxt, "Real dist to center (x,y)=(%lf,%lf)", dRealX, dRealY);
					cout << szOpTxt << endl;
					fout << szOpTxt << endl;
					#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
					cVideolog.AddText(szOpTxt);
					#endif

					// Need convert image coord to flight coord and reflect to control
					if (Landing(0.001 * -dRealY, 0.001 * dRealX, 0.001 * sGuidDist, cFlgt))
					{
						iState = STATE_SUCCESS;
					}
				}
				else
				{
					if ((iLoseTarget > 200) || (sGuidDist > 3000)) // &&=>||
					{
						/*#ifdef MODE_DEBUG_MODEL_LANDMARK
						sprintf(szOpTxt, "!!!!!!!!!! STATE_ERROR !!!!!!!!!!");
						cout << szOpTxt << endl;
						fout << szOpTxt << endl;
						#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
						cVideolog.AddText(szOpTxt);
						#endif*/
						
						iLoseTarget = 0;
						
						iErrLastState = iState;
						iState = STATE_ERROR;
					}
					else
					{
						iLoseTarget++;
						cFlgt.IdrFlyUp(0.1);
					}
					
					sprintf(szOpTxt, "Lost target with the frame %d", iLoseTarget);
					cout << szOpTxt << endl;
					fout << szOpTxt << endl;
					#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
					cVideolog.AddText(szOpTxt);
					#endif
				}
			}
			break;
		}
	}
	else
	{
		iErrLastState = iState;
		iState = STATE_ERROR;
		return true;
	}

	static double dLastGlobalTimer = cGuid.Timer();
	double dCrntGlobalTimer = cGuid.Timer();
	double dUsedTime = dCrntGlobalTimer - dLastGlobalTimer;
	dLastGlobalTimer = dCrntGlobalTimer;

	sprintf(szOpTxt, "Guid height(mm):%d, Used time:%lf", sGuidDist, dUsedTime);
	cout << szOpTxt << endl;
	fout << szOpTxt << endl;
	#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
	cVideolog.AddText(szOpTxt);
	#endif

	#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
	cVideolog.Flash();
	#endif

	cout << endl;
	fout << endl;

	return true;
}


// Use guidance ultrasonic to control flight height

void LimittingHeight(double & dCtrlHeight, short sUltrasonicDist, CFlight & cFlgt)
{
	TFlightState tfs;
	cFlgt.GetFlightState(tfs);

	if (sUltrasonicDist < 1800)
	{
		dCtrlHeight = tfs.fHeight + min(0.001 * (2000 - sUltrasonicDist), 0.5);
	}
	else if (sUltrasonicDist > 2200)
	{
		dCtrlHeight = tfs.fHeight - min(0.001 * (sUltrasonicDist - 2000), 0.5);
	}
	else
	{
		dCtrlHeight = tfs.fHeight;
	}
}


int FindThrSeg(Mat & mProc)
{
	int iResult = 0; // The result optimal threshold

	int iGreyMax = 0; // The max grey value
	int iGreyMin = 0; // The min grey value

	int iOptThr = 0; // Temporary optimal threshold, always changing

	// Assume destination area is white, background area is other color
	float fGreyAveBkg = 0; // Average grey value in background area
	float fGreyAveDes = 0; // Average grey value in destination area

	// Compute the histogram
	Histogram1D hst;
	MatND mHstProc = hst.getHistogram(mProc);

	for (int i = 0; i <= 255; i++)
	{
		if (mHstProc.at<float>(i) > 0)
		{
			iGreyMin = i;
			break;
		}
	}
	for (int i = 255; i >= 0; i--)
	{
		if (mHstProc.at<float>(i) > 0)
		{
			iGreyMax = i;
			break;
		}
	}
	iOptThr = (iGreyMax + iGreyMin) / 2;

	int iSaveCounter = 0;

	while (true)
	{
		int iTmpThr = 0; // Temporarily stored the threshold

		float fOvrCnt = 0; // The number of the pixels that the grey value is over than or equal to iOptThr
		float fOvrSum = 0; // The accumulate sum of the pixels that the grey value is over than or equal to iOptThr

		float fLwrCnt = 0; // The number of the pixels that the grey value is lower to iOptThr
		float fLwrSum = 0; // The accumulate sum of the pixels that the grey value is lower to iOptThr

		// Calc : the grey value is over than or equal to iOptThr
		for (int i = 255; i >= iOptThr; i--)
		{
			fOvrCnt += mHstProc.at<float>(i);
			fOvrSum += (i * mHstProc.at<float>(i));
		}
		if (fOvrCnt)
		{
			fGreyAveDes = fOvrSum / fOvrCnt;
		}

		// Calc : the grey value is lower to iOptThr
		for (int i = 0; i < iOptThr; i++)
		{
			fLwrCnt += mHstProc.at<float>(i);
			fLwrSum += (i * mHstProc.at<float>(i));
		}
		if (fLwrSum)
		{
			fGreyAveBkg = fLwrSum / fLwrCnt;
		}

		iTmpThr = (fGreyAveDes + fGreyAveBkg) / 2;

		if ((abs(iTmpThr - iOptThr)) <= 0.5) // ((abs(iTmpThr - iOptThr)) <= 0.5 => abs(iTmpThr - iOptThr) == 0
		{
			iResult = iTmpThr;
			break;
		}
		else
		{
			iOptThr = iTmpThr;
		}
		
		iSaveCounter++;
		if (iSaveCounter > 10000000)
		{
			return -1;
		}
	}

	return iResult;
}


void ExtractLinesV(Mat & mProc, int iExtLines[CGuidance::iHeight][5])
{
	// The first row compare with iFirstRowCenter (*)
	{
		static int iFirstRowCenter = CGuidance::iWidth / 2;

		unsigned char * pData = mProc.ptr<unsigned char>(0); // Image data in current row

		int iEdgCnt = 0; // Found edge count

		int iOffset = INT_MAX;
		int iTempLines[5] = { 0, 255, (0 + 255) / 2, FUNC_EL_VALID_FAIL, 255 - 0 }; // Temp lines edge

		for (int iCol = 0; iCol < CGuidance::iWidth - 1; iCol++)
		{
			bool bFindEdge = false;

			if ((pData[iCol + 1] - pData[iCol]) == 255)
			{
				iTempLines[FUNC_EL_LEFT] = iCol;
			}
			if ((pData[iCol + 1] - pData[iCol]) == -255)
			{
				iTempLines[FUNC_EL_RIGHT] = iCol;
				bFindEdge = true;
			}

			if (bFindEdge)
			{
				iEdgCnt++;

				iTempLines[FUNC_EL_CENTER] = (iTempLines[FUNC_EL_LEFT] + iTempLines[FUNC_EL_RIGHT]) / 2;

				if (abs(iFirstRowCenter - iTempLines[FUNC_EL_CENTER]) < iOffset)
				{
					iOffset = abs(iFirstRowCenter - iTempLines[FUNC_EL_CENTER]);
					iExtLines[0][FUNC_EL_LEFT]   = iTempLines[FUNC_EL_LEFT];
					iExtLines[0][FUNC_EL_RIGHT]  = iTempLines[FUNC_EL_RIGHT];
					iExtLines[0][FUNC_EL_CENTER] = iTempLines[FUNC_EL_CENTER];
				}
			}
		}

		if (pData[CGuidance::iWidth - 1] == 255) // The last col is white (*)
		{
			iTempLines[FUNC_EL_RIGHT] = CGuidance::iWidth - 1;

			iEdgCnt++;

			iTempLines[FUNC_EL_CENTER] = (iTempLines[FUNC_EL_LEFT] + iTempLines[FUNC_EL_RIGHT]) / 2;

			if (abs(iFirstRowCenter - iTempLines[FUNC_EL_CENTER]) < iOffset)
			{
				iOffset = abs(iFirstRowCenter - iTempLines[FUNC_EL_CENTER]);
				iExtLines[0][FUNC_EL_LEFT]   = iTempLines[FUNC_EL_LEFT];
				iExtLines[0][FUNC_EL_RIGHT]  = iTempLines[FUNC_EL_RIGHT];
				iExtLines[0][FUNC_EL_CENTER] = iTempLines[FUNC_EL_CENTER];
			}
		}

		if (iEdgCnt)
		{
			iExtLines[0][FUNC_EL_SUB] = iExtLines[0][FUNC_EL_RIGHT] - iExtLines[0][FUNC_EL_LEFT];
			if (iExtLines[0][FUNC_EL_SUB] < (CGuidance::iWidth / 4))
			{
				iExtLines[0][FUNC_EL_VALID] = FUNC_EL_VALID_TRUE;
			}
			else
			{
				iExtLines[0][FUNC_EL_VALID] = FUNC_EL_VALID_LARG;
			}
			iFirstRowCenter = iExtLines[0][FUNC_EL_CENTER];
		}
		else
		{
			iExtLines[0][FUNC_EL_VALID] = FUNC_EL_VALID_FAIL;
			iExtLines[0][FUNC_EL_CENTER] = iFirstRowCenter;
		}
	}

	for (int iRow = 1; iRow < CGuidance::iHeight; iRow++)
	{
		unsigned char * pData = mProc.ptr<unsigned char>(iRow); // Image data in current row

		int iEdgCnt = 0; // Found edge count

		int iOffset = INT_MAX;
		int iTempLines[5] = { 0, 255, (0 + 255) / 2, FUNC_EL_VALID_FAIL, 255 - 0 }; // Temp lines edge

		for (int iCol = 0; iCol < CGuidance::iWidth - 1; iCol++)
		{
			bool bFindEdge = false;

			if ((pData[iCol + 1] - pData[iCol]) == 255)
			{
				iTempLines[FUNC_EL_LEFT] = iCol;
			}
			if ((pData[iCol + 1] - pData[iCol]) == -255)
			{
				iTempLines[FUNC_EL_RIGHT] = iCol;
				bFindEdge = true;
			}

			if (bFindEdge)
			{
				iEdgCnt++;

				iTempLines[FUNC_EL_CENTER] = (iTempLines[FUNC_EL_LEFT] + iTempLines[FUNC_EL_RIGHT]) / 2;
				
				if (abs(iExtLines[iRow - 1][FUNC_EL_CENTER] - iTempLines[FUNC_EL_CENTER]) < iOffset)
				{
					iOffset = abs(iExtLines[iRow - 1][FUNC_EL_CENTER] - iTempLines[FUNC_EL_CENTER]);
					iExtLines[iRow][FUNC_EL_LEFT]   = iTempLines[FUNC_EL_LEFT];
					iExtLines[iRow][FUNC_EL_RIGHT]  = iTempLines[FUNC_EL_RIGHT];
					iExtLines[iRow][FUNC_EL_CENTER] = iTempLines[FUNC_EL_CENTER];
				}
			}
		}

		if (pData[CGuidance::iWidth - 1] == 255) // The last col is white
		{
			iTempLines[FUNC_EL_RIGHT] = CGuidance::iWidth - 1;
			
			iEdgCnt++;

			iTempLines[FUNC_EL_CENTER] = (iTempLines[FUNC_EL_LEFT] + iTempLines[FUNC_EL_RIGHT]) / 2;

			if (abs(iExtLines[iRow - 1][FUNC_EL_CENTER] - iTempLines[FUNC_EL_CENTER]) < iOffset)
			{
				iOffset = abs(iExtLines[iRow - 1][FUNC_EL_CENTER] - iTempLines[FUNC_EL_CENTER]);
				iExtLines[iRow][FUNC_EL_LEFT]   = iTempLines[FUNC_EL_LEFT];
				iExtLines[iRow][FUNC_EL_RIGHT]  = iTempLines[FUNC_EL_RIGHT];
				iExtLines[iRow][FUNC_EL_CENTER] = iTempLines[FUNC_EL_CENTER];
			}
		}

		if (iEdgCnt)
		{
			iExtLines[iRow][FUNC_EL_SUB] = iExtLines[iRow][FUNC_EL_RIGHT] - iExtLines[iRow][FUNC_EL_LEFT];
			if (iExtLines[iRow][FUNC_EL_SUB] < (CGuidance::iWidth / 4))
			{
				iExtLines[iRow][FUNC_EL_VALID] = FUNC_EL_VALID_TRUE;
			}
			else
			{
				iExtLines[iRow][FUNC_EL_VALID] = FUNC_EL_VALID_LARG;
			}
		}
		else
		{
			iExtLines[iRow][FUNC_EL_VALID] = FUNC_EL_VALID_FAIL;
			iExtLines[iRow][FUNC_EL_CENTER] = iExtLines[iRow - 1][FUNC_EL_CENTER];
		}
	}
}


void AdjustLinesV(int iExtLines[CGuidance::iHeight][5], int iMaxOffset)
{
	static int iLstFrmCenter = CGuidance::iWidth / 2;
	int iCenter = -1;

	for (int i = 0; i < CGuidance::iHeight; i++)
	{
		if (FUNC_EL_VALID_TRUE == iExtLines[i][FUNC_EL_VALID])
		{
			if (iCenter > 0)
			{
				if (abs(iExtLines[i][FUNC_EL_CENTER] - iCenter) > iMaxOffset) // Default max offset is 40
				{
					iExtLines[i][FUNC_EL_VALID] = FUNC_EL_VALID_FAIL;
				}
				else
				{
					iCenter = iExtLines[i][FUNC_EL_CENTER];
				}
			}
			else
			{
				if (abs(iExtLines[i][FUNC_EL_CENTER] - iLstFrmCenter) < 100)
				{
					iLstFrmCenter = iExtLines[i][FUNC_EL_CENTER];
					iCenter = iExtLines[i][FUNC_EL_CENTER];
				}
				else
				{
					iExtLines[i][FUNC_EL_VALID] = FUNC_EL_VALID_FAIL;
				}
			}
		}
	}
}


void AnalyseLinesV(int iExtLines[CGuidance::iHeight][5], int & iBad, int & iMove)
{
	int iInvalidRows = 0;
	int iLargeRows   = 0;

	iMove = FUNC_AL_MOVE_CENTER;

	for (int i = 0; i < CGuidance::iHeight; i++)
	{
		if (FUNC_EL_VALID_TRUE != iExtLines[i][FUNC_EL_VALID])
		{
			iInvalidRows++;
		}

		if (FUNC_EL_VALID_LARG == iExtLines[i][FUNC_EL_VALID])
		{
			iLargeRows++;
		}
		else
		{
			// Mention! Ignore the last row and first row
			if ((5 < iLargeRows) && (iLargeRows < 35))
			{
				int iDiff = iExtLines[i - iLargeRows][FUNC_EL_CENTER] - iExtLines[i - iLargeRows - 1][FUNC_EL_CENTER];
				if (iDiff < 0)
				{
					iMove = FUNC_AL_MOVE_LEFT;
				}
				else
				{
					iMove = FUNC_AL_MOVE_RIGHT;
				}
				
				iInvalidRows = -1;
				break;
			}
			else
			{
				iLargeRows = 0;
			}
		}
	}

	if (iInvalidRows > 60)
	{
		iBad = FUNC_AL_BAD_BADFRAME;
	}
	else
	{
		iBad = FUNC_AL_BAD_SUCCESS;
	}

	sprintf(szOpTxt, "InvalidRows:%d, LargeRows:%d", iInvalidRows, iLargeRows);
	cout << szOpTxt << endl;
	fout << szOpTxt << endl;
	#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
	cVideolog.AddText(szOpTxt);
	#endif
}


void FlyingAdjustV(int iExtLines[CGuidance::iHeight][5], double dCtrlHeight, CFlightIndoor & cFlgt, Mat * pMatShow)
{
	double dAlpha  = 0;
	double dOffset = 0;

	CalcAlphaOffsetV(iExtLines, dAlpha, dOffset, pMatShow);

	double dSpeedAlpha  = 0;
	double dSpeedOffset = 0;

	if (fabs(dAlpha) > 3)
	{
		if (dAlpha > 30)
		{
			dSpeedAlpha = -15;
		}
		else if (dAlpha < -30)
		{
			dSpeedAlpha = 15;
		}
		else
		{
			dSpeedAlpha = -dAlpha / 2;
		}
	}

	if (fabs(dOffset) > 10)
	{
		if (dOffset > 30)
		{
			dSpeedOffset = -0.15;
		}
		else if (dOffset < -30)
		{
			dSpeedOffset = 0.15;
		}
		else
		{
			dSpeedOffset = -dOffset / 200;
		}
	}

	cFlgt.IdrFlying(0.4, dSpeedOffset, dCtrlHeight, dSpeedAlpha);

	sprintf(szOpTxt, "Value-> Alpha:%lf, Offset:%lf", dAlpha, dOffset);
	cout << szOpTxt << endl;
	fout << szOpTxt << endl;
	#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
	cVideolog.AddText(szOpTxt);
	#endif
	sprintf(szOpTxt, "Speed-> Alpha:%lf, Offset:%lf", dSpeedAlpha, dSpeedOffset);
	cout << szOpTxt << endl;
	fout << szOpTxt << endl;
	#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
	cVideolog.AddText(szOpTxt);
	#endif
}


void CalcAlphaOffsetV(int iExtLines[CGuidance::iHeight][5], double & dAlpha, double & dOffset, Mat * pMatShow)
{
	// Use The Least Squares Method

	const int iLmtUp = 20;
	const int iLmtDn = 220;

	int iValidCountUp = 0;
	int iValidCountDn = 0;

	double dAveXUp = 0, dAveYUp = 0;
	double dAveXDn = 0, dAveYDn = 0;

	double dSumNumeratorUp = 0, dSumDenominatorUp = 0;
	double dSumNumeratorDn = 0, dSumDenominatorDn = 0;

	for (int i = iLmtUp; i < CGuidance::iHeight / 2; i++)
	{
		if (FUNC_EL_VALID_TRUE == iExtLines[i][FUNC_EL_VALID])
		{
			iValidCountUp++;

			dAveXUp += i;
			dAveYUp += iExtLines[i][FUNC_EL_CENTER];

			dSumNumeratorUp   += i * iExtLines[i][FUNC_EL_CENTER];
			dSumDenominatorUp += i * i;
		}
	}
	dAveXUp /= iValidCountUp;
	dAveYUp /= iValidCountUp;

	for (int i = CGuidance::iHeight / 2; i < iLmtDn; i++)
	{
		if (FUNC_EL_VALID_TRUE == iExtLines[i][FUNC_EL_VALID])
		{
			iValidCountDn++;

			dAveXDn += i;
			dAveYDn += iExtLines[i][FUNC_EL_CENTER];

			dSumNumeratorDn   += i * iExtLines[i][FUNC_EL_CENTER];
			dSumDenominatorDn += i * i;
		}
	}
	dAveXDn /= iValidCountDn;
	dAveYDn /= iValidCountDn;

	double dKUp = (dSumNumeratorUp - iValidCountUp * dAveXUp * dAveYUp) / (dSumDenominatorUp - iValidCountUp * dAveXUp * dAveXUp);
	double dBUp = dAveYUp - dKUp * dAveXUp;
	double dKDn = (dSumNumeratorDn - iValidCountDn * dAveXDn * dAveYDn) / (dSumDenominatorDn - iValidCountDn * dAveXDn * dAveXDn);
	double dBDn = dAveYDn - dKDn * dAveXDn;
	
	double dPntUpX = CGuidance::iHeight * 1.0 / 4;
	double dPntUpY = dKUp * dPntUpX + dBUp;
	double dPntDnX = CGuidance::iHeight * 3.0 / 4;
	double dPntDnY = dKDn * dPntDnX + dBDn;

	dOffset = (CGuidance::iWidth / 2) - ((dPntUpY + dPntDnY) / 2);
	dAlpha  = CL_RADTODEG(atan2((dPntDnY - dPntUpY), (dPntDnX - dPntUpX)));

	if (pMatShow)
	{
		line((*pMatShow), Point(dKUp * iLmtUp + dBUp, iLmtUp), Point(dKUp * (CGuidance::iHeight / 2) + dBUp, CGuidance::iHeight / 2), Scalar(180));
		line((*pMatShow), Point(dKDn * iLmtDn + dBDn, iLmtDn), Point(dKDn * (CGuidance::iHeight / 2) + dBDn, CGuidance::iHeight / 2), Scalar(180));
		Point pntUp(dPntUpY, dPntUpX), pntDn(dPntDnY, dPntDnX);
		circle((*pMatShow), pntUp, 3, Scalar(255));
		circle((*pMatShow), pntDn, 3, Scalar(255));
		line((*pMatShow), pntUp, pntDn, Scalar(255));
	}
}


void ExtractLinesH(Mat & mProc, int iExtLines[CGuidance::iWidth][5])
{
	// The first row compare with iFirstRowCenter (*)
	{
		static int iFirstRowCenter = CGuidance::iHeight / 2;

		unsigned char * pData = mProc.ptr<unsigned char>(0); // Image data in current row

		int iEdgCnt = 0; // Found edge count

		int iOffset = INT_MAX;
		int iTempLines[5] = { 0, 255, (0 + 255) / 2, FUNC_EL_VALID_FAIL, 255 - 0 }; // Temp lines edge

		for (int iCol = 0; iCol < CGuidance::iHeight - 1; iCol++)
		{
			bool bFindEdge = false;

			if ((pData[iCol + 1] - pData[iCol]) == 255)
			{
				iTempLines[FUNC_EL_LEFT] = iCol;
			}
			if ((pData[iCol + 1] - pData[iCol]) == -255)
			{
				iTempLines[FUNC_EL_RIGHT] = iCol;
				bFindEdge = true;
			}

			if (bFindEdge)
			{
				iEdgCnt++;

				iTempLines[FUNC_EL_CENTER] = (iTempLines[FUNC_EL_LEFT] + iTempLines[FUNC_EL_RIGHT]) / 2;

				if (abs(iFirstRowCenter - iTempLines[FUNC_EL_CENTER]) < iOffset)
				{
					iOffset = abs(iFirstRowCenter - iTempLines[FUNC_EL_CENTER]);
					iExtLines[0][FUNC_EL_LEFT]   = iTempLines[FUNC_EL_LEFT];
					iExtLines[0][FUNC_EL_RIGHT]  = iTempLines[FUNC_EL_RIGHT];
					iExtLines[0][FUNC_EL_CENTER] = iTempLines[FUNC_EL_CENTER];
				}
			}
		}

		if (pData[CGuidance::iHeight - 1] == 255) // The last col is white (*)
		{
			iTempLines[FUNC_EL_RIGHT] = CGuidance::iHeight - 1;

			iEdgCnt++;

			iTempLines[FUNC_EL_CENTER] = (iTempLines[FUNC_EL_LEFT] + iTempLines[FUNC_EL_RIGHT]) / 2;

			if (abs(iFirstRowCenter - iTempLines[FUNC_EL_CENTER]) < iOffset)
			{
				iOffset = abs(iFirstRowCenter - iTempLines[FUNC_EL_CENTER]);
				iExtLines[0][FUNC_EL_LEFT]   = iTempLines[FUNC_EL_LEFT];
				iExtLines[0][FUNC_EL_RIGHT]  = iTempLines[FUNC_EL_RIGHT];
				iExtLines[0][FUNC_EL_CENTER] = iTempLines[FUNC_EL_CENTER];
			}
		}

		if (iEdgCnt)
		{
			iExtLines[0][FUNC_EL_SUB] = iExtLines[0][FUNC_EL_RIGHT] - iExtLines[0][FUNC_EL_LEFT];
			if (iExtLines[0][FUNC_EL_SUB] < (CGuidance::iHeight / 3))
			{
				iExtLines[0][FUNC_EL_VALID] = FUNC_EL_VALID_TRUE;
			}
			else
			{
				iExtLines[0][FUNC_EL_VALID] = FUNC_EL_VALID_LARG;
			}
			iFirstRowCenter = iExtLines[0][FUNC_EL_CENTER];
		}
		else
		{
			iExtLines[0][FUNC_EL_VALID] = FUNC_EL_VALID_FAIL;
			iExtLines[0][FUNC_EL_CENTER] = iFirstRowCenter;
		}
	}

	for (int iRow = 1; iRow < CGuidance::iWidth; iRow++)
	{
		unsigned char * pData = mProc.ptr<unsigned char>(iRow); // Image data in current row

		int iEdgCnt = 0; // Found edge count

		int iOffset = INT_MAX;
		int iTempLines[5] = { 0, 255, (0 + 255) / 2, FUNC_EL_VALID_FAIL, 255 - 0 }; // Temp lines edge

		for (int iCol = 0; iCol < CGuidance::iHeight - 1; iCol++)
		{
			bool bFindEdge = false;

			if ((pData[iCol + 1] - pData[iCol]) == 255)
			{
				iTempLines[FUNC_EL_LEFT] = iCol;
			}
			if ((pData[iCol + 1] - pData[iCol]) == -255)
			{
				iTempLines[FUNC_EL_RIGHT] = iCol;
				bFindEdge = true;
			}

			if (bFindEdge)
			{
				iEdgCnt++;

				iTempLines[FUNC_EL_CENTER] = (iTempLines[FUNC_EL_LEFT] + iTempLines[FUNC_EL_RIGHT]) / 2;
				
				if (abs(iExtLines[iRow - 1][FUNC_EL_CENTER] - iTempLines[FUNC_EL_CENTER]) < iOffset)
				{
					iOffset = abs(iExtLines[iRow - 1][FUNC_EL_CENTER] - iTempLines[FUNC_EL_CENTER]);
					iExtLines[iRow][FUNC_EL_LEFT]   = iTempLines[FUNC_EL_LEFT];
					iExtLines[iRow][FUNC_EL_RIGHT]  = iTempLines[FUNC_EL_RIGHT];
					iExtLines[iRow][FUNC_EL_CENTER] = iTempLines[FUNC_EL_CENTER];
				}
			}
		}

		if (pData[CGuidance::iWidth - 1] == 255) // The last col is white
		{
			iTempLines[FUNC_EL_RIGHT] = CGuidance::iHeight - 1;
			
			iEdgCnt++;

			iTempLines[FUNC_EL_CENTER] = (iTempLines[FUNC_EL_LEFT] + iTempLines[FUNC_EL_RIGHT]) / 2;

			if (abs(iExtLines[iRow - 1][FUNC_EL_CENTER] - iTempLines[FUNC_EL_CENTER]) < iOffset)
			{
				iOffset = abs(iExtLines[iRow - 1][FUNC_EL_CENTER] - iTempLines[FUNC_EL_CENTER]);
				iExtLines[iRow][FUNC_EL_LEFT]   = iTempLines[FUNC_EL_LEFT];
				iExtLines[iRow][FUNC_EL_RIGHT]  = iTempLines[FUNC_EL_RIGHT];
				iExtLines[iRow][FUNC_EL_CENTER] = iTempLines[FUNC_EL_CENTER];
			}
		}

		if (iEdgCnt)
		{
			iExtLines[iRow][FUNC_EL_SUB] = iExtLines[iRow][FUNC_EL_RIGHT] - iExtLines[iRow][FUNC_EL_LEFT];
			if (iExtLines[iRow][FUNC_EL_SUB] < (CGuidance::iHeight / 4))
			{
				iExtLines[iRow][FUNC_EL_VALID] = FUNC_EL_VALID_TRUE;
			}
			else
			{
				iExtLines[iRow][FUNC_EL_VALID] = FUNC_EL_VALID_LARG;
			}
		}
		else
		{
			iExtLines[iRow][FUNC_EL_VALID] = FUNC_EL_VALID_FAIL;
			iExtLines[iRow][FUNC_EL_CENTER] = iExtLines[iRow - 1][FUNC_EL_CENTER];
		}
	}
}


void AdjustLinesH(int iExtLines[CGuidance::iWidth][5], int iMaxOffset)
{
	static int iLstFrmCenter = CGuidance::iHeight / 2;
	int iCenter = -1;

	for (int i = 0; i < CGuidance::iWidth; i++)
	{
		if (FUNC_EL_VALID_TRUE == iExtLines[i][FUNC_EL_VALID])
		{
			if (iCenter > 0)
			{
				if (abs(iExtLines[i][FUNC_EL_CENTER] - iCenter) > iMaxOffset) // Default max offset is 40
				{
					iExtLines[i][FUNC_EL_VALID] = FUNC_EL_VALID_FAIL;
				}
				else
				{
					iCenter = iExtLines[i][FUNC_EL_CENTER];
				}
			}
			else
			{
				if (abs(iExtLines[i][FUNC_EL_CENTER] - iLstFrmCenter) < 60)
				{
					iLstFrmCenter = iExtLines[i][FUNC_EL_CENTER];
					iCenter = iExtLines[i][FUNC_EL_CENTER];
				}
				else
				{
					iExtLines[i][FUNC_EL_VALID] = FUNC_EL_VALID_FAIL;
				}
			}
		}
	}
}


void AnalyseLinesH(int iExtLines[CGuidance::iWidth][5], int & iBad)
{
	int iInvalidRows = 0;

	for (int i = 0; i < CGuidance::iWidth; i++)
	{
		if (FUNC_EL_VALID_TRUE != iExtLines[i][FUNC_EL_VALID])
		{
			iInvalidRows++;
		}
	}

	if (iInvalidRows > 80)
	{
		iBad = FUNC_AL_BAD_BADFRAME;
	}
	else
	{
		iBad = FUNC_AL_BAD_SUCCESS;
	}

	sprintf(szOpTxt, "InvalidRows:%d", iInvalidRows);
	cout << szOpTxt << endl;
	fout << szOpTxt << endl;
	#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
	cVideolog.AddText(szOpTxt);
	#endif
}


void FlyingAdjustH(int iExtLines[CGuidance::iWidth][5], double dCtrlHeight, CFlightIndoor & cFlgt, Mat * pMatShow)
{
	double dAlpha  = 0;
	double dOffset = 0;

	CalcAlphaOffsetH(iExtLines, dAlpha, dOffset, pMatShow);

	double dSpeedAlpha  = 0;
	double dSpeedOffset = 0;

	if (fabs(dAlpha) > 3)
	{
		if (dAlpha > 30)
		{
			dSpeedAlpha = -15;
		}
		else if (dAlpha < -30)
		{
			dSpeedAlpha = 15;
		}
		else
		{
			dSpeedAlpha = -dAlpha / 2;
		}
	}

	if (fabs(dOffset) > 10)
	{
		if (dOffset > 30)
		{
			dSpeedOffset = 0.15;
		}
		else if (dOffset < -30)
		{
			dSpeedOffset = -0.15;
		}
		else
		{
			dSpeedOffset = dOffset / 200;
		}
	}

	cFlgt.IdrFlying(dSpeedOffset, 0.4, dCtrlHeight, dSpeedAlpha);

	sprintf(szOpTxt, "Value-> Alpha:%lf, Offset:%lf", dAlpha, dOffset);
	cout << szOpTxt << endl;
	fout << szOpTxt << endl;
	#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
	cVideolog.AddText(szOpTxt);
	#endif
	sprintf(szOpTxt, "Speed-> Alpha:%lf, Offset:%lf", dSpeedAlpha, dSpeedOffset);
	cout << szOpTxt << endl;
	fout << szOpTxt << endl;
	#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
	cVideolog.AddText(szOpTxt);
	#endif
}


void CalcAlphaOffsetH(int iExtLines[CGuidance::iWidth][5], double & dAlpha, double & dOffset, Mat * pMatShow)
{
	// Use The Least Squares Method

	const int iLmtUp = 30;
	const int iLmtDn = 290;

	int iValidCountUp = 0;
	int iValidCountDn = 0;

	double dAveXUp = 0, dAveYUp = 0;
	double dAveXDn = 0, dAveYDn = 0;

	double dSumNumeratorUp = 0, dSumDenominatorUp = 0;
	double dSumNumeratorDn = 0, dSumDenominatorDn = 0;

	for (int i = iLmtUp; i < CGuidance::iWidth / 2; i++)
	{
		if (FUNC_EL_VALID_TRUE == iExtLines[i][FUNC_EL_VALID])
		{
			iValidCountUp++;

			dAveXUp += i;
			dAveYUp += iExtLines[i][FUNC_EL_CENTER];

			dSumNumeratorUp   += i * iExtLines[i][FUNC_EL_CENTER];
			dSumDenominatorUp += i * i;
		}
	}
	dAveXUp /= iValidCountUp;
	dAveYUp /= iValidCountUp;

	for (int i = CGuidance::iWidth / 2; i < iLmtDn; i++)
	{
		if (FUNC_EL_VALID_TRUE == iExtLines[i][FUNC_EL_VALID])
		{
			iValidCountDn++;

			dAveXDn += i;
			dAveYDn += iExtLines[i][FUNC_EL_CENTER];

			dSumNumeratorDn   += i * iExtLines[i][FUNC_EL_CENTER];
			dSumDenominatorDn += i * i;
		}
	}
	dAveXDn /= iValidCountDn;
	dAveYDn /= iValidCountDn;

	double dKUp = (dSumNumeratorUp - iValidCountUp * dAveXUp * dAveYUp) / (dSumDenominatorUp - iValidCountUp * dAveXUp * dAveXUp);
	double dBUp = dAveYUp - dKUp * dAveXUp;
	double dKDn = (dSumNumeratorDn - iValidCountDn * dAveXDn * dAveYDn) / (dSumDenominatorDn - iValidCountDn * dAveXDn * dAveXDn);
	double dBDn = dAveYDn - dKDn * dAveXDn;
	
	double dPntUpX = CGuidance::iWidth * 1.0 / 4;
	double dPntUpY = dKUp * dPntUpX + dBUp;
	double dPntDnX = CGuidance::iWidth * 3.0 / 4;
	double dPntDnY = dKDn * dPntDnX + dBDn;

	dOffset = (CGuidance::iHeight / 2) - ((dPntUpY + dPntDnY) / 2);
	dAlpha  = CL_RADTODEG(atan2((dPntDnY - dPntUpY), (dPntDnX - dPntUpX)));

	if (pMatShow)
	{
		line((*pMatShow), Point(dKUp * iLmtUp + dBUp, iLmtUp), Point(dKUp * (CGuidance::iWidth / 2) + dBUp, CGuidance::iWidth / 2), Scalar(180));
		line((*pMatShow), Point(dKDn * iLmtDn + dBDn, iLmtDn), Point(dKDn * (CGuidance::iWidth / 2) + dBDn, CGuidance::iWidth / 2), Scalar(180));
		Point pntUp(dPntUpY, dPntUpX), pntDn(dPntDnY, dPntDnX);
		circle((*pMatShow), pntUp, 3, Scalar(255));
		circle((*pMatShow), pntDn, 3, Scalar(255));
		line((*pMatShow), pntUp, pntDn, Scalar(255));
	}
}


double CalcPointDistance(Point pnt1, Point pnt2)
{
	return sqrt((pnt1.x - pnt2.x) * (pnt1.x - pnt2.x) + (pnt1.y - pnt2.y) * (pnt1.y - pnt2.y));
}


bool Landing(double dOffsetX, double dOffsetY, double dHeight, CFlightIndoor & cFlgt)
{
	const double dBasicVelocity = 0.3; // m/s
	
	double dSpeedX = 1.1 * max(min(dBasicVelocity * dHeight * dOffsetX, 0.1), -0.1);
	double dSpeedY = 1.1 * max(min(dBasicVelocity * dHeight * dOffsetY, 0.1), -0.1);
	
	TFlightState tfs;
	cFlgt.GetFlightState(tfs);
	float fLogHeight = tfs.fHeight;
	
	sprintf(szOpTxt, "Landing!! Speed(forward,right)=(%lf,%lf)", dSpeedX, dSpeedY);
	cout << szOpTxt << endl;
	fout << szOpTxt << endl;
	#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
	cVideolog.AddText(szOpTxt);
	#endif
	
	sprintf(szOpTxt, "Landing!! Height(flog,guid)=(%lf,%lf)", fLogHeight, dHeight);
	cout << szOpTxt << endl;
	fout << szOpTxt << endl;
	#if ((defined MODE_SAVEVIDEOLOG) || (defined MODE_SHOWVIDEOLOG))
	cVideolog.AddText(szOpTxt);
	#endif
	
	// Change to only use circle!!
	if (dHeight < 0.6)
	{
		if (fabs(dOffsetX) < 0.03 && fabs(dOffsetY) < 0.03)
		{
			cFlgt.IdrFlyDn(4);
			return true;
		}
		else
		{
			cFlgt.IdrFlying(1.2 * dSpeedX, 1.2 * dSpeedY, fLogHeight);
		}
	}
	else if (dHeight < 1.0)
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
	
	return false;
}


void AddLine(Mat & mImg, Point pnt1, Point pnt2, int iAddValue)
{
	LineIterator itLine(mImg, pnt1, pnt2);

	for (int i = 0; i < itLine.count; i++)
	{
		mImg.at<uchar>(itLine.pos()) += iAddValue;

		++itLine;
	}
}


void FindCross(Mat & mImg, vector<Point> & vcPntCross, int iThreshold)
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
		//circle(mImg, *it, 3, Scalar(255));
		circle(mImg, *it, 3, Scalar(150));
	}
}


void PixelToReal(float fFocal, float fCU, float fCV, short sDist, int iPixX, int iPixY, double & dRealX, double & dRealY)
{
	dRealX = (iPixX - fCU) * sDist / fFocal;
	dRealY = (iPixY - fCV) * sDist / fFocal;
}


void AdjustHghLineGap()
{
	double dGuidHeight = 0.001 * sGuidDist;
	
	if (dGuidHeight > 0.8)
	{
		tFactor.iLmHghMaxLineGap = tFactor.iLmHghMaxLineGapInit;
	}
	else
	{
		tFactor.iLmHghMaxLineGap = tFactor.iLmHghMaxLineGapInit + static_cast<int>(30 * (0.8 - dGuidHeight));
		
		if (dGuidHeight < 0.5)
		{
			tFactor.iLmHghMaxLineGap += 2;
		}
	}
}
