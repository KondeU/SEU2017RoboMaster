// Videolog.cpp
// Version 1.0
// Writed by Deyou Kong, 2017-04-20
// Checked by Deyou Kong, 2017-05-04

#include <iostream>
#include <fstream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include "Videolog.h"
#include "Mode.h"

CVideolog::CVideolog(int iFrameHeight, int iFrameWidth, int iMatType, int iRowCnt, int iColCnt)
{
	#ifdef MODE_AUTOSTART
	fout.open("./autorun/2017RobomasterTask/Runlog/Videolog.log");
	#else
	fout.open("./Runlog/Videolog.log");
	#endif

	iSmlFrmHeight = iFrameHeight;
	iSmlFrmWidth  = iFrameWidth;

	this->iRowCnt = iRowCnt;
	this->iColCnt = iColCnt;

	this->iMatType = iMatType;

	// Use double buffer
	mFrame      = Mat::zeros((this->iRowCnt + 1) * iSmlFrmHeight, this->iColCnt * iSmlFrmWidth, this->iMatType);
	mFrmForward = Mat::zeros((this->iRowCnt + 1) * iSmlFrmHeight, this->iColCnt * iSmlFrmWidth, this->iMatType);
	iTextCrntRow = 0;

	#ifdef MODE_SAVEVIDEOLOG
	#ifdef MODE_AUTOSTART
	pVideo = new VideoWriter("./autorun/2017RobomasterTask/Runlog/Videolog.avi", CV_FOURCC('D', 'I', 'V', 'X'),
		10/*FPS set to 10*/, Size(this->iColCnt * iSmlFrmWidth, (this->iRowCnt + 1) * iSmlFrmHeight), false);
	#else
	pVideo = new VideoWriter("./Runlog/Videolog.avi", CV_FOURCC('D', 'I', 'V', 'X'),
		10/*FPS set to 10*/, Size(this->iColCnt * iSmlFrmWidth, (this->iRowCnt + 1) * iSmlFrmHeight), false);
	#endif
	#endif

	#ifdef MODE_SHOWVIDEOLOG
	namedWindow("Videolog");
	#endif
}
CVideolog::~CVideolog()
{
	#ifdef MODE_SAVEVIDEOLOG
	delete pVideo;
	#endif

	#ifdef MODE_SHOWVIDEOLOG
	destroyWindow("Videolog");
	#endif
}

void CVideolog::AddFrame(int iIdxRow, int iIdxCol, Mat & mSmlFrm)
{
	try
	{
		Mat mROI = mFrame(Rect(iIdxCol * iSmlFrmWidth, iIdxRow * iSmlFrmHeight, iSmlFrmWidth, iSmlFrmHeight));
		mSmlFrm.copyTo(mROI);
	}
	catch (cv::Exception)
	{
		cout << "$ CV Exception occur in function CVideolog::AddFrame(..)" << endl;
		fout << "CV Exception occur in function CVideolog::AddFrame(..)\n" << endl;
	}
}

void CVideolog::AddText(char * szText)
{	
	try
	{
		const int iLineHeight = 14;

		iTextCrntRow++;

		putText(mFrame, szText, Point(10, (iRowCnt * iSmlFrmHeight + iTextCrntRow * iLineHeight)),
			FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255));

	}
	catch (cv::Exception)
	{
		cout << "$ CV Exception occur in function CVideolog::AddText(..)" << endl;
		fout << "CV Exception occur in function CVideolog::AddText(..)\n" << endl;
	}
}

void CVideolog::Flash()
{
	try
	{
		mFrame.copyTo(mFrmForward);
		mFrame.release();
		mFrame = Mat::zeros((iRowCnt + 1) * iSmlFrmHeight, iColCnt * iSmlFrmWidth, iMatType);

		iTextCrntRow = 0;

		#ifdef MODE_SAVEVIDEOLOG
		(*pVideo) << mFrmForward;
		#endif

		#ifdef MODE_SHOWVIDEOLOG
		imshow("Videolog", mFrmForward);
		#endif
	}
	catch (cv::Exception)
	{
		cout << "$ CV Exception occur in function CVideolog::Flash(..)" << endl;
		fout << "CV Exception occur in function CVideolog::Flash(..)\n" << endl;
	}
}

