// Videolog.h
// Writed by Deyou Kong, 2017-04-20

#pragma once

#include "Headers.h"

class CVideolog
{
public:
	CVideolog(int iFrameHeight, int iFrameWidth, int iMatType = CV_8UC1, int iRowCnt = 2, int iColCnt = 2)
	{
		iSmlFrmHeight = iFrameHeight;
		iSmlFrmWidth  = iFrameWidth;

		this->iRowCnt = iRowCnt;
		this->iColCnt = iColCnt;

		this->iMatType = iMatType;

		mFrame = Mat::zeros((this->iRowCnt + 1) * iSmlFrmHeight, this->iColCnt * iSmlFrmWidth, this->iMatType);
		//!!
		mFrmShow = Mat::zeros((this->iRowCnt + 1) * iSmlFrmHeight, this->iColCnt * iSmlFrmWidth, this->iMatType);
		iTextCrntRow = 0;

		#ifdef MODE_SAVEVIDEOLOG
		#ifdef MODE_AUTOSTART
		pVideo = new VideoWriter("./autorun/RobomasterTask/Runlog/Videolog.avi", CV_FOURCC('D', 'I', 'V', 'X'),
			10/*FPS set to 10*/, Size(this->iColCnt * iSmlFrmWidth, (this->iRowCnt + 1) * iSmlFrmHeight), false);
		#else
		pVideo = new VideoWriter("../Runlog/Videolog.avi", CV_FOURCC('D', 'I', 'V', 'X'),
			10/*FPS set to 10*/, Size(this->iColCnt * iSmlFrmWidth, (this->iRowCnt + 1) * iSmlFrmHeight), false);
		#endif
		#endif

		#ifdef MODE_SHOWVIDEOLOG
		namedWindow("Videolog");
		#endif
	}
	~CVideolog()
	{
		#ifdef MODE_SAVEVIDEOLOG
		delete pVideo;
		#endif

		#ifdef MODE_SHOWVIDEOLOG
		destroyWindow("Videolog");
		#endif
	}

	void AddFrame(int iIdxRow, int iIdxCol, Mat & mSmlFrm);
	void AddText(char * szText);

	void Flash();

private:
	int iSmlFrmHeight, iSmlFrmWidth;
	int iRowCnt, iColCnt;
	int iMatType;

	VideoWriter * pVideo;
	
	//!!
	Mat mFrame, mFrmShow;
	int iTextCrntRow;
};
