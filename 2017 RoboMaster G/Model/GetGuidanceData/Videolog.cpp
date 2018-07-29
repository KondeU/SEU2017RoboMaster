// Videolog.cpp
// Writed by Deyou Kong, 2017-04-20

#include "Videolog.h"

void CVideolog::AddFrame(int iIdxRow, int iIdxCol, Mat & mSmlFrm)
{
	Mat mROI = mFrame(Rect(iIdxCol * iSmlFrmWidth, iIdxRow * iSmlFrmHeight, iSmlFrmWidth, iSmlFrmHeight));
	mSmlFrm.copyTo(mROI);
}

void CVideolog::AddText(char * szText)
{
	iTextCrntRow++;

	//!!
	putText(mFrame, szText, Point(10, (iRowCnt * iSmlFrmHeight + iTextCrntRow * 12)),
		FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255));
}

void CVideolog::Flash()
{
	//!!
	mFrame.copyTo(mFrmShow);
	mFrame = Mat::zeros((iRowCnt + 1) * iSmlFrmHeight, iColCnt * iSmlFrmWidth, iMatType);

	#ifdef MODE_SAVEVIDEOLOG
	(*pVideo) << mFrmShow;
	#endif

	#ifdef MODE_SHOWVIDEOLOG
	imshow("Videolog", mFrmShow);
	#endif

	//!! mFrame = Mat::zeros((iRowCnt + 1) * iSmlFrmHeight, iColCnt * iSmlFrmWidth, iMatType);
	iTextCrntRow = 0;
}
