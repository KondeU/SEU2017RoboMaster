/*

BSD 2-Clause License

Copyright (c) 2017, KondeU
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

// CVideolog.cpp
// Require OpenCV2, Can run in multi OS.
// Started by Deyou Kong, 2017-06-27
// Checked by Deyou Kong, 2017-06-28

#include <iostream>
#include <fstream>
#include <climits>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include "CVideolog.h"

#define ERRPRE(s) ("$ " s) // Error output prefix

//const char szVideologFile[] = "./Runlog/Videolog.avi"; // Local named
extern char szVideologFile[NAME_MAX + 1]; // Extern named

CVideolog::CVideolog(int iFrameHeight, int iFrameWidth, bool bShow, bool bSave,
	int iInputMatType, int iRowCount, int iColCount, double dTextRegionWidthScale)
{
	iSmlFrmHeight = iFrameHeight;
	iSmlFrmWidth  = iFrameWidth;

	iRowCnt = iRowCount;
	iColCnt = iColCount;

	iMatType = iInputMatType;

	dTxtRgnWidthScale = dTextRegionWidthScale;

	bShowVideolog = bShow;
	bSaveVideolog = bSave;

	try
	{
		mFrame = Mat::zeros((iRowCnt + dTxtRgnWidthScale) * iSmlFrmHeight, iColCnt * iSmlFrmWidth, iMatType);

		iTextCrntRow = 0;

		if (bShowVideolog)
		{
			namedWindow("Videolog");
		}

		if (bSaveVideolog)
		{		
			pVideo = new VideoWriter(szVideologFile, CV_FOURCC('D', 'I', 'V', 'X'), 10/*FPS set to 10*/,
				Size(iColCnt * iSmlFrmWidth, (iRowCnt + dTxtRgnWidthScale) * iSmlFrmHeight),
				((iMatType == CV_8UC1) ? false : true));
			
			if (! (pVideo->isOpened()))
			{
				cout << ERRPRE("Create video writer error!") << endl;
			}
		}
		else
		{
			pVideo = nullptr;
		}
	}
	catch (cv::Exception)
	{
		cout << ERRPRE("Catched exception in CVideolog::CVideolog(..) : CV Exception") << endl;
	}
	catch (...)
	{
		cout << ERRPRE("Catched exception in CVideolog::CVideolog(..) : Unknown") << endl;
	}
}
CVideolog::~CVideolog()
{
	if (bShowVideolog)
	{
		destroyWindow("Videolog");
	}

	if (bSaveVideolog && pVideo)
	{
		delete pVideo;
	}
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
		cout << ERRPRE("Catched exception in CVideolog::AddFrame(..) : CV Exception") << endl;
	}
	catch (...)
	{
		cout << ERRPRE("Catched exception in CVideolog::AddFrame(..) : Unknown") << endl;
	}
}

void CVideolog::AddText(const char * szText)
{	
	const int iLineHeight = 14; // Text height

	iTextCrntRow++;

	try
	{
		putText(mFrame, szText, Point(2, (iRowCnt * iSmlFrmHeight + iTextCrntRow * iLineHeight)),
			FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255));
	}
	catch (cv::Exception)
	{
		cout << ERRPRE("Catched exception in CVideolog::AddText(..) : CV Exception") << endl;
	}
	catch (...)
	{
		cout << ERRPRE("Catched exception in CVideolog::AddText(..) : Unknown") << endl;
	}
}

void CVideolog::Flash()
{
	try
	{
		iTextCrntRow = 0;

		if (bShowVideolog)
		{
			imshow("Videolog", mFrame);
		}

		if (bSaveVideolog)
		{
			(*pVideo) << mFrame;
		}
		
		mFrame.release();
		mFrame = Mat::zeros((iRowCnt + dTxtRgnWidthScale) * iSmlFrmHeight, iColCnt * iSmlFrmWidth, iMatType);
	}
	catch (cv::Exception)
	{
		cout << ERRPRE("Catched exception in CVideolog::Flash(..) : CV Exception") << endl;
	}
	catch (...)
	{
		cout << ERRPRE("Catched exception in CVideolog::Flash(..) : Unknown") << endl;
	}
}

