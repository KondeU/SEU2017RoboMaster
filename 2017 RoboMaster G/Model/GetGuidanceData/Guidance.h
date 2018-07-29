#pragma once

#include <iostream>
#include <fstream>
#include <sys/time.h>
using namespace std;

#include "PropOpenCV.h"

#include "DJI_guidance.h"
#include "DJI_utility.h"

#define VBUS_D e_vbus1
//#define VBUS_D e_vbus5 // Vbus port is 5, enum value is 0, down
#define VBUS_F e_vbus1 // Vbus port is 1, enum value is 1, forward
#define VBUS_R e_vbus2 // Vbus port is 2, enum value is 2, right
#define VBUS_B e_vbus3 // Vbus port is 3, enum value is 3, back
#define VBUS_L e_vbus4 // Vbus port is 4, enum value is 4, left

#define IMGREY_L true
#define IMGREY_R false

class CGuidance
{
public:

	static const int iWidth  = 320;
	static const int iHeight = 240;
	static const int iImageSize = iHeight * iWidth;

	CGuidance()
	{
		fout.open("./Guidance.log");
	}
	~CGuidance()
	{
		fout.close();
	}

	double Timer();

	//-------- Custom function --------//
	// In Guidance.cpp
	// int GuidanceCallback(int iDataType, int iDataLen, char * byContent);
	//---------------------------------//

	bool Init();
	bool Deinit();
	void GetData();

private:

	ofstream fout;

	void GuidanceError(int iErr);
};
