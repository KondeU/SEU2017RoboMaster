// Flight.h
// Version 2.0
// Started by Deyou Kong, 2017-06-30
// Checked by Deyou Kong, 2017-06-30

#pragma once

#include "DJIHardDriverManifold.h"
#include "conboardsdktask.h"
#include "APIThread.h"

#include "Logger.h"

#define C_PI ((double)(3.1415926535897932))
#define C_EARTH ((double)(6378137.0))
#define CL_RADTODEG(rad) ((rad)*(180)/(C_PI))

#define GEAR_ERR -1
#define GEAR_DN 0
#define GEAR_UP 1

struct TGPS
{
	double dLatitude;
	double dLongitude;
};

struct TBaseLine
{
	TGPS tgpsOrg;
	TGPS tgpsPnt;
	double dX;
	double dY;
	double dK;
};

struct TFlightState
{
	BroadcastData bd;

	float fRoll, fPitch, fYaw;
	float fSpeedX, fSpeedY;
	float fHeight;
	int iGear;
};

class CFlight
{
public:

	CFlight(ConboardSDKScript * pst = nullptr) : cLog("Flight") { pScript = pst; }

	inline void SetScript(ConboardSDKScript * pst) { pScript = pst; }

	TGPS GetCrntGPS();

	void CalcOffset(double & x, double & y, const TGPS & tgpsBase, const TGPS & tgpsDest);
	void CalcOffset(double & x, double & y, const TGPS & tgpsDest);

	TGPS CalcGPS(const TGPS & tgpsBase, const double & x, const double & y);

	float CalcYaw(const double & x, const double & y);
	float CalcYaw(const TGPS & tgpsBase, const TGPS & tgpsDest);
	float CalcYaw(const TGPS & tgpsDest);

	bool IsNearDest(const TGPS & tgpsDest, float fNearThreshold = 0.5);

	void CalcBaseLine(TBaseLine & tbl, const TGPS & tgpsOrg, const TGPS & tgpsPnt, float fPrecision = 0.000001);

	double CalcDistance(TBaseLine & tbl);

	int IsAltitudeInRange(const float fMin, const float fMax, float * pfDiff = nullptr);

	int GetBattery();

	void SetCameraAngle(double dYaw, double dRoll, double dPitch);

	void FlyToward(TGPS tgpsDest, float fFlySpeed = 0.7, float fFlyHeight = 3.0, float * pfYaw = nullptr);
	
	void FlyYawing(float fYaw, float fFlyHeight = 3.0);

	void FlyForward (float fYaw, float fFlySpeed = 0.7, float fFlyHeight = 3.0);
	void FlyBack    (float fYaw, float fFlySpeed = 0.7, float fFlyHeight = 3.0);
	void FlyLeft    (float fYaw, float fFlySpeed = 0.7, float fFlyHeight = 3.0);
	void FlyRight   (float fYaw, float fFlySpeed = 0.7, float fFlyHeight = 3.0);

	void FlyUp   (float fYaw, float fUpSpeed = 0.7);
	void FlyDown (float fYaw, float fDownSpeed = 0.7);

	void CtrlObtain();
	void CtrlRelease();

	void Takeoff();
	void Land();

	int GetGear();

	void GetFlightState(TFlightState & tfs);

protected:

	ConboardSDKScript * pScript;

private:

	CLogger cLog;
};

/*
FlightData control value:
Control mode byte
    bit7:6
        0b00：HORI_ATTI_TILT_ANG
        0b01：HORI_VEL
        0b10：HORI_POS
    bit5:4
        0b00：VERT_VEL
        0b01：VERT_POS
        0b10：VERT_THRUST
    bit3
        0b0: YAW_ANG
        0b1: YAW_RATE
    bit2:1
        0b00：horizontal frame is ground frame
        0b01：horizontal frame is body frame
    bit0
        0b0：non-stable mode
        0b1：stable mode
Linker:(Control mode byte):http://developer.dji.com/onboard-sdk/documentation/appendix/index.html
*/

