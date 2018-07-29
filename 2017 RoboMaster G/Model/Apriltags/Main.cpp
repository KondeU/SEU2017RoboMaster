#include <iostream>
#include <vector>
#include <sys/time.h>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include "Apriltags.h"

const unsigned long long ullBase[] =
	{
		0x231bLL,
		0x2ea5LL,
		0x346aLL,
		0x45b9LL,
		0x79a6LL,
		0x7f6bLL,
		0xb358LL,
		0x380bLL
	};

double Timer()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return ((double(tv.tv_sec) * 1000) + (double(tv.tv_usec) / 1000));
}

int main()
{
	VideoCapture cCaptrue(0);
	
	if (!cCaptrue.isOpened())
    {
		cout << "Fail to open camera!" << endl;
        exit(1);
    }
    
    cCaptrue.set(CV_CAP_PROP_FRAME_WIDTH,  640);
 	cCaptrue.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
 	
	CApriltags cTags("tag16h5", 320, 240, 462.2708 / 2, 460.3916 / 2, 310.8862 / 2, 221.2809 / 2, 0.167);
	cTags.SetExternalTag("tag16h5", sizeof(ullBase) / sizeof(ullBase[0]), ullBase);
	
	Mat mFrame, mImg, mImgGray;
	
	double dTimerStart = Timer();
	
	while (true)
	{
		if (!(cCaptrue.read(mFrame)))
		{
			cout << "Read camera data failed!" << endl;
			break;
		}
		
		//imshow("Frame", mFrame);
		
		pyrDown(mFrame, mImg);
		//imshow("Image", mImg);
		
		cvtColor(mImg, mImgGray, CV_BGR2GRAY);
		//imshow("Gray", mImgGray);
		
		vector<TTagInfo> vTagInfo;
		cTags.Detect(mImgGray, vTagInfo);
		
		/*
		struct TTagInfo
		{
			int iTagID;

			double dDistance;
			double dX, dY, dZ;
			double dYaw, dPitch, dRoll;

			struct TPoint
			{
				float fX, fY;
			} tCenter, tCorner[4];
		};
		*/

		cout << "Tags found: " << vTagInfo.size() << endl;
		for (int i = 0; i < vTagInfo.size(); i++)
		{
			cout << " " << "ID:" << vTagInfo[i].iTagID << endl;
			
			cout << " " << "Dis:" << vTagInfo[i].dDistance << "m" << endl;
			cout << " " << "X:"  << vTagInfo[i].dX
						<< ",Y:" << vTagInfo[i].dY
						<< ",Z:" << vTagInfo[i].dZ
						<< endl;
			
			cout << " " << "Yaw:"    << vTagInfo[i].dYaw
						<< ",Pitch:" << vTagInfo[i].dPitch
						<< ",Roll:"  << vTagInfo[i].dRoll
						<< endl;
			
			circle(mImg, cv::Point2f(vTagInfo[i].tCenter.fX, vTagInfo[i].tCenter.fY), 4, cv::Scalar(255, 0, 0), 2);
			
			line(mImg, Point2f(vTagInfo[i].tCorner[0].fX, vTagInfo[i].tCorner[0].fY),
				Point2f(vTagInfo[i].tCorner[1].fX, vTagInfo[i].tCorner[1].fY), Scalar(0,0,255));
			line(mImg, Point2f(vTagInfo[i].tCorner[1].fX, vTagInfo[i].tCorner[1].fY),
				Point2f(vTagInfo[i].tCorner[2].fX, vTagInfo[i].tCorner[2].fY), Scalar(0,0,255));
			line(mImg, Point2f(vTagInfo[i].tCorner[2].fX, vTagInfo[i].tCorner[2].fY),
				Point2f(vTagInfo[i].tCorner[3].fX, vTagInfo[i].tCorner[3].fY), Scalar(0,255,0));
			line(mImg, Point2f(vTagInfo[i].tCorner[3].fX, vTagInfo[i].tCorner[3].fY),
				Point2f(vTagInfo[i].tCorner[0].fX, vTagInfo[i].tCorner[0].fY), Scalar(0,0,255));
		}
		
		cout << endl;
		
		imshow("Tags", mImg);
		
		if (waitKey(1) > 0)
		{
			cout << "Exit program normally." << endl;
			break;
		}
		
		cout << "Used time: " << (Timer() - dTimerStart) << "ms" << endl;
		dTimerStart = Timer();
	}
	
	return 0;
	
}
