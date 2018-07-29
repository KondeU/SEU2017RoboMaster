// Serial.h
// Version 2.0
// Started by Deyou Kong, 2017-07-10
// Checked by Deyou Kong, 2017-07-11
// Updated by Deyou Kong, 2017-07-26

#pragma once

#include "Logger.h"

#define DEBUG_SERIAL false

class CSerial
{
#define FRMDAT_TYPE_LIGHT 0x01
#define FRMDAT_TYPE_MOTOR 0x02
#define FRMDAT_TYPE_ULTRS 0x03

#define FRMDAT_LIGHT_LEFT  1
#define FRMDAT_LIGHT_RIGHT 2

#define LGCLR_NONE   0
#define LGCLR_RED    1
#define LGCLR_GREEN  2
#define LGCLR_BLUE   3
#define LGCLR_YELLOW 4
#define LGCLR_WHITE  5
#define LGCLR_CYAN   6

	typedef unsigned char byte;

public:

	CSerial();
	~CSerial();

	void SetLight(byte byLight, byte byR, byte byG, byte byB);
	void SetMotor(float fAngle);
	float GetUltrasonicDistance();

	void SetLight(byte byLight, int iColor);

private:

	CLogger cLog;

	int fdSerial;

	bool Send();
	bool Recv();

#pragma pack(push, 1)
	struct TFrame
	{
		byte bySOF; // The Start of Frame flag, must be 0x66, 1 byte

		byte byDataType; // Data type

		union UData
		{
			struct TLight
			{
				byte byLight; // Left light(is 1) or Right light(is 2)
				byte byR;
				byte byG;
				byte byB;
			} tLight;          // Light show control data, Manifold->STM32
			float fMotorAngle; // Motor control angle, Manifold->STM32
			float fUltrsDist;  // Ultrasonic distance data, STM32->Manifold
			byte byData[4]; // Equivalent byte length
		} uData;

		byte byReserved;

		byte byEOF; // The End of Frame flag, must be 0x66, 1 byte
	} tFrame;
#pragma pack(pop)
};


