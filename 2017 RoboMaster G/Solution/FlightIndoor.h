// FlightIndoor.h
// Version 2.0
// Started by Deyou Kong, 2017-06-30
// Checked by Deyou Kong, 2017-06-30
// Updated by Deyou Kong, 2017-07-20

#pragma once

#include "Flight.h"

class CFlightIndoor : public CFlight
{
public:

	CFlightIndoor(ConboardSDKScript * pst = nullptr) : CFlight(pst), cLog("FlightIndoor") {};

	void IdrFlyHeight(float fHeight);
	void IdrFlyUp(float fSpeed = 0.7);
	void IdrFlyDn(float fSpeed = 0.7);
	void IdrFlying(float fSpeedX, float fSpeedY, float fHeight, float fYawRate = 0.0);

private:

	CLogger cLog;
};

