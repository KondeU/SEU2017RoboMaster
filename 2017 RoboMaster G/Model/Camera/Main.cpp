#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include "Logger.h"
#include "Camera.h"

int main()
{
	CLogger cLog("Main");
	CCamera cCam;
	
	while (true)
    {
		if ('q' == waitKey(1))
		{
			break;
		}

		double dTimeStart = cLog.Timer();

		cCam.Grab();
		Mat mCamRaw = cCam.GetRaw();
		Mat mCamDec = cCam.GetDec();

		cLog.pVideolog->AddFrame(0, 0, mCamDec);
		cLog.pVideolog->AddFrame(0, 1, mCamDec);
		cLog.pVideolog->AddFrame(1, 0, mCamDec);
		cLog.pVideolog->AddFrame(1, 1, mCamDec);

		imshow("Camera Raw", mCamRaw);

		double dTimeEnd = cLog.Timer();

		double dUsedTime = dTimeEnd - dTimeStart;
		cout << "Use time: " << dUsedTime << endl;

		char sz[32];
		sprintf(sz, "Used time: %lf", dUsedTime);
		cLog.pVideolog->AddText("Camera data");
		cLog.pVideolog->AddText(sz);
		sprintf(sz, "Config: %d %d %d %d %d",
			cLog.iLogCounter, cLog.bTeam, cLog.bFlightMove,
			cLog.bShowVideolog, cLog.bSaveVideolog);
		cLog.pVideolog->AddText(sz);

		cLog.pVideolog->Flash();
    }

    return 0;
}


