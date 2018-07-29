// X3.h
// Version 1.0
// Writed by Deyou Kong, 2017-05-01
// Checked by Deyou Kong, 2017-05-04

#pragma once

#include <fstream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

class CX3
{
public:

	CX3();
	~CX3();

	bool Init(Mat & mFrame); // The "mFrame" will store X3 frame data
	void Deinit(); // Mention: "Deinit" function will send the ctrl + c signal

	bool GetFrame();

private:

	ofstream fout;

	Mat mFrame;
};

