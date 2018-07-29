// Videolog.h
// Version 1.0
// Writed by Deyou Kong, 2017-04-20
// Checked by Deyou Kong, 2017-05-04

#pragma once

#include <fstream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

class CVideolog
{
public:

	CVideolog(int iFrameHeight, int iFrameWidth, int iMatType = CV_8UC1, int iRowCnt = 2, int iColCnt = 2);
	~CVideolog();

	void AddFrame(int iIdxRow, int iIdxCol, Mat & mSmlFrm);
	void AddText(char * szText);

	void Flash();
	
	int Random();

private:

	ofstream fout;

	int iSmlFrmHeight, iSmlFrmWidth;
	int iRowCnt, iColCnt;
	int iMatType;

	VideoWriter * pVideo;
	
	Mat mFrame, mFrmForward;
	int iTextCrntRow;
	
	int iRandom;
};

