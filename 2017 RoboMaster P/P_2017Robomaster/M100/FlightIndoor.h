// FlightIndoor.h
// Version 1.0
// Writed by Deyou Kong, 2017-04-03
// Checked by Deyou Kong, 2017-05-04

#pragma once

#include "Flight.h"

class CFlightIndoor : public CFlight
{
public:

	CFlightIndoor(ConboardSDKScript * pst = nullptr);
	~CFlightIndoor();

	void IdrFlyHeight(float fHeight);
	void IdrFlyUp(float fSpeed = 0.7);
	void IdrFlyDn(float fSpeed = 0.7);
	void IdrFlying(float fSpeedX, float fSpeedY, float fHeight, float fYawRate = 0.0);

private:

	ofstream fout;
};

