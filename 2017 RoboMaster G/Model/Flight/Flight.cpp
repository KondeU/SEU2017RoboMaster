// Flight.cpp
// Version 2.0
// Started by Deyou Kong, 2017-06-30
// Checked by Deyou Kong, 2017-06-30

#include <iostream>
#include <sstream>
#include <cmath>
using namespace std;

#include "Flight.h"

TGPS CFlight::GetCrntGPS()
{
	TGPS tgpsCrnt;

	BroadcastData bd = pScript->getApi()->getBroadcastData();

	tgpsCrnt.dLatitude = bd.pos.latitude;
	tgpsCrnt.dLongitude = bd.pos.longitude;

	return tgpsCrnt;
}

void CFlight::CalcOffset(double & x, double & y, const TGPS & tgpsBase, const TGPS & tgpsDest)
{
	double dLatitude = tgpsDest.dLatitude - tgpsBase.dLatitude;
	double dLongitude = tgpsDest.dLongitude - tgpsBase.dLongitude;

	x = dLatitude * C_EARTH;
	y = dLongitude * C_EARTH * cos(tgpsDest.dLatitude / 2.0 + tgpsBase.dLatitude / 2.0);

	return;
}

void CFlight::CalcOffset(double & x, double & y, const TGPS & tgpsDest)
{
	CalcOffset(x, y, GetCrntGPS(), tgpsDest);
	return;
}

TGPS CFlight::CalcGPS(const TGPS & tgpsBase, const double & x, const double & y)
{
	TGPS tgpsDest;

	tgpsDest.dLatitude = x / C_EARTH + tgpsBase.dLatitude;
	tgpsDest.dLongitude = y / (C_EARTH * cos(tgpsDest.dLatitude / 2.0 + tgpsBase.dLatitude / 2.0)) + tgpsBase.dLongitude;

	return tgpsDest;
}

float CFlight::CalcYaw(const double & x, const double & y)
{
	float fYaw = 0; //Default direction is North

					/*

					Yaw angle calculate way:

					# : rad_to_deg(rad) (rad * 180 / PI)
					1 : x > 0, y > 0 :  90 - rad_to_deg(arctan(x/y))
					2 : x > 0, y < 0 : -90 + rad_to_deg(arctan(x/-y))
					3 : x > 0, y = 0 :   0
					4 : x < 0, y > 0 :  90 + rad_to_deg(arctan(-x/y))
					6 : x < 0, y = 0 : 180
					5 : x < 0, y < 0 : -90 - rad_to_deg(arctan(x/y))
					7 : x = 0, y > 0 :  90
					8 : x = 0, y < 0 : -90
					9 : x = 0, y = 0 : Error

					*/

	if (0 == y)
	{
		if (x > 0)
		{
			fYaw = 0;
		}
		else if (x < 0)
		{
			fYaw = 180;
		}
		else
		{
			cLog << prefix << "Error : In function CalcYaw, class CFlight : "
				 << "Both x and y value is zero, now set yaw with 0." << endl;
			fYaw = 0;
		}
	}
	else
	{
		fYaw = ((y > 0) ? 90 : -90) - CL_RADTODEG(atan(x / y));
	}

	return fYaw;
}

float CFlight::CalcYaw(const TGPS & tgpsBase, const TGPS & tgpsDest)
{
	double x, y;
	CalcOffset(x, y, tgpsBase, tgpsDest);
	return CalcYaw(x, y);
}

float CFlight::CalcYaw(const TGPS & tgpsDest)
{
	return CalcYaw(GetCrntGPS(), tgpsDest);
}

bool CFlight::IsNearDest(const TGPS & tgpsDest, float fNearThreshold)
{
	// Default parameter:
	// fNearThreshold = 0.5

	double x, y;

	CalcOffset(x, y, tgpsDest);

	if ((fabs(x) < fNearThreshold) && (fabs(y) < fNearThreshold))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CFlight::CalcBaseLine(TBaseLine & tbl, const TGPS & tgpsOrg, const TGPS & tgpsPnt, float fPrecision)
{
	// Default parameter:
	// fPrecision = 0.000001

	tbl.tgpsOrg = tgpsOrg;
	tbl.tgpsPnt = tgpsPnt;

	CalcOffset(tbl.dX, tbl.dY, tgpsOrg, tgpsPnt);

	if (fabs(tbl.dX) < fPrecision)
	{
		tbl.dX = ((tbl.dX < 0) ? -1 : 1) * fPrecision;
	}

	tbl.dK = tbl.dY / tbl.dX;
}

double CFlight::CalcDistance(TBaseLine & tbl)
{
	double dDis;
	double x, y;

	CalcOffset(x, y, tbl.tgpsOrg, GetCrntGPS());

	dDis = (tbl.dK * x - y) / (sqrt(tbl.dK * tbl.dK + 1));

	return dDis;
}

int CFlight::IsAltitudeInRange(const float fMin, const float fMax, float * pfDiff)
{
	BroadcastData bd = pScript->getApi()->getBroadcastData();
	
	if (bd.pos.height < fMin)
	{
		if (pfDiff)
		{
			*pfDiff = bd.pos.height - fMin;
		}
		return -1;
	}
	if (bd.pos.height > fMax)
	{
		if (pfDiff)
		{
			*pfDiff = bd.pos.height - fMax;
		}
		return 1;
	}

	return 0;
}

int CFlight::GetBattery()
{
	BroadcastData bd = pScript->getApi()->getBroadcastData();
	return bd.battery;
}

void CFlight::SetCameraAngle(double dYaw, double dRoll, double dPitch)
{
	void * data = (UserData)("0 0 -900 10");
	stringstream s;
	s << (char *)data;
	GimbalAngleData a;
	s >> a.yaw >> a.roll >> a.pitch >> a.duration;
	a.mode     = 1;
	a.yaw      = dYaw * 10;
	a.roll     = dRoll * 10;
	a.pitch    = dPitch * 10;
	a.duration = 10;
	pScript->getCamera()->setGimbalAngle(&a);
}

void CFlight::FlyToward(TGPS tgpsDest, float fFlySpeed, float fFlyHeight, float * pfYaw)
{
	// Default parameter:
	// fFlySpeed = 0.7
	// fFlyHeight = 3.0
	// pfYaw = nullptr; (pfYaw is a output parameter)

	float fYaw = CalcYaw(tgpsDest);
	if (pfYaw)
	{
		(*pfYaw) = fYaw;
	}
	
	FlightData fd;
	fd.flag = 0x53; // 0x53HEX = 01010011BIN
	fd.x = fFlySpeed;
	fd.y = 0;
	fd.z = fFlyHeight;
	fd.yaw = fYaw;

	pScript->getFlight()->setFlight(&fd);
}

void CFlight::FlyYawing(float fYaw, float fFlyHeight)
{
	// Default parameter:
	// fFlyHeight = 3.0

	FlightData fd;
	fd.flag = 0x91; // 0x91HEX = 10010001BIN
	fd.x = 0;
	fd.y = 0;
	fd.z = fFlyHeight;
	fd.yaw = fYaw;

	pScript->getFlight()->setFlight(&fd);
}

void CFlight::FlyForward(float fYaw, float fFlySpeed, float fFlyHeight)
{
	// Default parameter:
	// fFlySpeed = 0.7
	// fFlyHeight = 3.0

	FlightData fd;
	fd.flag = 0x53; // 0x53HEX = 01010011BIN
	fd.x = fFlySpeed;
	fd.y = 0;
	fd.z = fFlyHeight;
	fd.yaw = fYaw;

	pScript->getFlight()->setFlight(&fd);
}

void CFlight::FlyBack(float fYaw, float fFlySpeed, float fFlyHeight)
{
	// Default parameter:
	// fFlySpeed = 0.7
	// fFlyHeight = 3.0

	FlightData fd;
	fd.flag = 0x53; // 0x53HEX = 01010011BIN
	fd.x = -fFlySpeed;
	fd.y = 0;
	fd.z = fFlyHeight;
	fd.yaw = fYaw;

	pScript->getFlight()->setFlight(&fd);
}

void CFlight::FlyLeft(float fYaw, float fFlySpeed, float fFlyHeight)
{
	// Default parameter:
	// fFlySpeed = 0.7
	// fFlyHeight = 3.0

	FlightData fd;
	fd.flag = 0x53; // 0x53HEX = 01010011BIN
	fd.x = 0;
	fd.y = -fFlySpeed;
	fd.z = fFlyHeight;
	fd.yaw = fYaw;

	pScript->getFlight()->setFlight(&fd);
}

void CFlight::FlyRight(float fYaw, float fFlySpeed, float fFlyHeight)
{
	// Default parameter:
	// fFlySpeed = 0.7
	// fFlyHeight = 3.0

	FlightData fd;
	fd.flag = 0x53; // 0x53HEX = 01010011BIN
	fd.x = 0;
	fd.y = fFlySpeed;
	fd.z = fFlyHeight;
	fd.yaw = fYaw;

	pScript->getFlight()->setFlight(&fd);
}

void CFlight::FlyUp(float fYaw, float fUpSpeed)
{
	// Default parameter:
	// fUpSpeed = 0.7

	FlightData fd;
	fd.flag = 0x43; // 0x43HEX = 01000011BIN
	fd.x = 0;
	fd.y = 0;
	fd.z = fUpSpeed;
	fd.yaw = fYaw;

	pScript->getFlight()->setFlight(&fd);
}

void CFlight::FlyDown(float fYaw, float fDownSpeed)
{
	// Default parameter:
	// fDownSpeed = 0.7

	FlightData fd;
	fd.flag = 0x43; // 0x43HEX = 01000011BIN
	fd.x = 0;
	fd.y = 0;
	fd.z = -fDownSpeed;
	fd.yaw = fYaw;

	pScript->getFlight()->setFlight(&fd);
}

void CFlight::CtrlObtain()
{
	pScript->getApi()->setControl(true);
	cLog << prefix << "Obtain aircraft control." << endl;
	sleep(1);
}

void CFlight::CtrlRelease()
{
	pScript->getApi()->setControl(false);
	cLog << prefix << "Release aircraft control." << endl;
	sleep(1);
}

void CFlight::Takeoff()
{
	if (cLog.bFlightMove)
	{
		int flag = 4;
		pScript->getFlight()->task((DJI::onboardSDK::Flight::TASK)flag);
		cLog << prefix << "Take off!" << endl;
		sleep(3);
	}
	else
	{
		cLog << prefix << "Virtual take off!" << endl;
	}
}

void CFlight::Land()
{
	if (cLog.bFlightMove)
	{
		int flag = 6;
		pScript->getFlight()->task((DJI::onboardSDK::Flight::TASK)flag);
		cLog << prefix << "Land!" << endl;
		sleep(3);
	}
	else
	{
		cLog << prefix << "Virtual land!" << endl;
	}
}

int CFlight::GetGear()
{
	BroadcastData bd = pScript->getApi()->getBroadcastData();

	switch (bd.rc.gear)
	{
	case -10000:
		return GEAR_UP;

	case -4545:
		return GEAR_DN;

	default:
		return GEAR_ERR;
	}
}

void CFlight::GetFlightState(TFlightState & tfs)
{
	tfs.bd = pScript->getApi()->getBroadcastData();

	tfs.fRoll  = atan2(2.0 * (tfs.bd.q.q3 * tfs.bd.q.q2 + tfs.bd.q.q0 * tfs.bd.q.q1), 1.0 - 2.0 * (tfs.bd.q.q1 * tfs.bd.q.q1 + tfs.bd.q.q2 * tfs.bd.q.q2));
	tfs.fPitch = asin(2.0 * (tfs.bd.q.q2 * tfs.bd.q.q0 - tfs.bd.q.q3 * tfs.bd.q.q1));
	tfs.fYaw   = atan2(2.0 * (tfs.bd.q.q3 * tfs.bd.q.q0 + tfs.bd.q.q1 * tfs.bd.q.q2), -1.0 + 2.0 * (tfs.bd.q.q0 * tfs.bd.q.q0 + tfs.bd.q.q1 * tfs.bd.q.q1));
	tfs.fRoll  = tfs.fRoll  * 180.0 / C_PI;
	tfs.fPitch = tfs.fPitch * 180.0 / C_PI;
	tfs.fYaw   = tfs.fYaw   * 180.0 / C_PI;

	tfs.fSpeedX = tfs.bd.v.x;
	tfs.fSpeedY = tfs.bd.v.y;

	tfs.fHeight = tfs.bd.pos.height;

	switch (tfs.bd.rc.gear)
	{
	case -10000:
		tfs.iGear = GEAR_UP;
		break;

	case -4545:
		tfs.iGear = GEAR_DN;
		break;

	default:
		tfs.iGear = GEAR_ERR;
		break;
	}
}

