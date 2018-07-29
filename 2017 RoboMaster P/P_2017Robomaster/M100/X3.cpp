// X3.cpp
// Version 1.0
// Writed by Deyou Kong, 2017-05-01
// Checked by Deyou Kong, 2017-05-04

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

extern "C"
{
#include "X3SDK/manifold_cam/djicam.h"
}

#include "X3.h"
#include "Mode.h"

CX3::CX3()
{
	#ifdef MODE_AUTOSTART
	fout.open("./autorun/2017RobomasterTask/Runlog/X3.log");
	#else
	fout.open("./Runlog/X3.log");
	#endif
}

CX3::~CX3()
{
	fout.close();
}

bool CX3::Init(Mat & mFrame)
{
	if (!manifold_cam_init(GETBUFFER_MODE | TRANSFER_MODE))
	{
		cout << "$ X3 init successfully." << endl;
		fout << "X3 init successfully.\n" << endl;
	}
	else
	{
		cout << "$ X3 init failed." << endl;
		fout << "X3 init failed.\n" << endl;
		return false;
	}

	mFrame.create(720, 1280, CV_8UC3);
	this->mFrame = mFrame;
	
	return true;
}

void CX3::Deinit()
{
	kill(getpid(), SIGINT);

	while (!manifold_cam_exit())
	{
		usleep(10000);
	}
}

bool CX3::GetFrame()
{
	static unsigned char pFrmData[1280*720*3/2+8];
	
	Mat mNV12YUV(720 * 3 / 2, 1280, CV_8UC1, pFrmData);
	
	unsigned int uiFrmCnt = 0;
	int iRet = manifold_cam_read(pFrmData, &uiFrmCnt, CAM_NON_BLOCK);
	
	if (iRet > 0)
	{
		fout << "Success in frame: " << uiFrmCnt << " with size: " << iRet << endl;
	}
	else if (iRet == 0)
	{
		fout << "Not yet update" << endl;
	}
	else
	{
		cout << "$ Catch a quit signal!" << endl;
		fout << "Catch a quit signal!\n" << endl;
		
		return false;
	}
	
	cvtColor(mNV12YUV, mFrame, CV_YUV2BGR_NV12);
	
	return true;
}

