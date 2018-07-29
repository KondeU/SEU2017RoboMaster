// FlightIndoor.cpp
// Version 1.0
// Writed by Deyou Kong, 2017-04-03
// Checked by Deyou Kong, 2017-05-04

#include "FlightIndoor.h"

CFlightIndoor::CFlightIndoor(ConboardSDKScript * pst) : CFlight(pst)
{
	#ifdef MODE_AUTOSTART
	fout.open("./autorun/2017RobomasterTask/Runlog/FlightIndoor.log");
	#else
	fout.open("./Runlog/FlightIndoor.log");
	#endif
};
CFlightIndoor::~CFlightIndoor()
{
	fout.close();
}

void CFlightIndoor::IdrFlyHeight(float fHeight)
{
	FlightData fd;
	fd.flag = 0x5B; // 0x5BHEX = 01011011BIN
	fd.x = 0;
	fd.y = 0;
	fd.z = fHeight;
	fd.yaw = 0;

	pScript->getFlight()->setFlight(&fd);
}

void CFlightIndoor::IdrFlyUp(float fSpeed)
{
	// Default parameter:
	// fSpeed = 0.7

	FlightData fd;
	fd.flag = 0x4B; // 0x4BHEX = 01001011BIN
	fd.x = 0;
	fd.y = 0;
	fd.z = fSpeed;
	fd.yaw = 0;

	pScript->getFlight()->setFlight(&fd);
}

void CFlightIndoor::IdrFlyDn(float fSpeed)
{
	// Default parameter:
	// fSpeed = 0.7

	FlightData fd;
	fd.flag = 0x4B; // 0x4BHEX = 01001011BIN
	fd.x = 0;
	fd.y = 0;
	fd.z = -fSpeed;
	fd.yaw = 0;

	pScript->getFlight()->setFlight(&fd);
}

void CFlightIndoor::IdrFlying(float fSpeedX, float fSpeedY, float fHeight, float fYawRate)
{
	// Default parameter:
	// fYawRate = 0.0

	FlightData fd;
	fd.flag = 0x5B; // 0x5BHEX = 01011011BIN
	fd.x = fSpeedX;
	fd.y = fSpeedY;
	fd.z = fHeight;
	fd.yaw = fYawRate;

	pScript->getFlight()->setFlight(&fd);
}

