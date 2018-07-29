// Main.cpp
// Version 2.0
// Started by Deyou Kong, 2017-07-13
// Checked by Deyou Kong, 2017-07-13

#include <iostream>
#include <unistd.h>
#include <sys/time.h>
using namespace std;

#include "Serial.h"

double Timer()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return ((double(tv.tv_sec) * 1000) + (double(tv.tv_usec) / 1000));
}

int main()
{
	CSerial cSerial;

	cSerial.SetLight(FRMDAT_LIGHT_LEFT, 255, 0, 0);
	//sleep(2);
	
	int i = 0;
	
	cin >> i;
	
	cSerial.SetMotor(i);
	//sleep(2);
	
	usleep(50000);
	cSerial.SetLight(FRMDAT_LIGHT_RIGHT, 255, 0, 0);
	//cSerial.SetLight(FRMDAT_LIGHT_LEFT, 0, 255, 255);
	//cSerial.SetLight(FRMDAT_LIGHT_RIGHT, 0, 0, 0);
	
	double dTimeStart = Timer();
	float fDist;
	//for (int i = 0; i < 10; i++)
	//{
		fDist = cSerial.GetUltrasonicDistance();
	//}
	double d = Timer() - dTimeStart;
	cout << "Ultra use time: " << d << endl;
	cout << "Get ultrasonic distance: " << fDist << endl;

	//usleep(40000);
	
	//dTimeStart = Timer();
	//fDist = cSerial.GetUltrasonicDistance();
	//d = Timer() - dTimeStart;
	//cout << "Ultra use time: " << d << endl;
	//cout << "Get ultrasonic distance: " << fDist << endl;
	
	
	return 0;
}
