// Guidance.h
// Version 1.0
// Writed by Deyou Kong, 2017-04-17
// Checked by Deyou Kong, 2017-05-04

#pragma once

#include <fstream>
using namespace std;

#define VBUS_D e_vbus5 // Vbus port is 5, enum value is 0, down
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

	CGuidance();
	~CGuidance();

	double Timer();

	//-------- Custom function --------//
	// In Guidance.cpp
	// int GuidanceCallback(int iDataType, int iDataLen, char * byContent);
	// is a custom function, need to implement.
	//---------------------------------//

	bool Init();
	bool Deinit();
	void GetData();

	// Extra function, date 2017-05-10
	void SetExposure(int iExpectedBrightness);

private:

	ofstream fout;

	void GuidanceError(int iErr);
};

