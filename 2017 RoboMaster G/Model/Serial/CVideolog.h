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

// CVideolog.h
// Require OpenCV2, Can run in multi OS.
// Started by Deyou Kong, 2017-06-27
// Checked by Deyou Kong, 2017-06-28

#pragma once

#include <opencv2/opencv.hpp>

class CVideolog
{
public:

	CVideolog(int iFrameHeight, int iFrameWidth,
		bool bShowVideolog = true, bool bSaveVideolog = true,
		int iMatType = CV_8UC1, int iRowCnt = 2, int iColCnt = 2,
		double dTxtRgnWidthScale = 1);
	~CVideolog();

	void AddFrame(int iIdxRow, int iIdxCol, cv::Mat & mSmlFrm);
	void AddText(const char * szText);

	void Flash();

private:

	int iSmlFrmHeight, iSmlFrmWidth;
	int iRowCnt, iColCnt;
	int iMatType; // Gray: CV_8UC1, Color: CV_8UC3

	double dTxtRgnWidthScale;

	bool bShowVideolog, bSaveVideolog;

	cv::VideoWriter * pVideo;

	cv::Mat mFrame;
	int iTextCrntRow;
};

