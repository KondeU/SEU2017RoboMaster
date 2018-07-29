// Serial.cpp
// Version 2.0
// Started by Deyou Kong, 2017-07-10
// Checked by Deyou Kong, 2017-07-11

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
using namespace std;

#include "Serial.h"

CSerial::CSerial() : cLog("Serial")
{
	fdSerial = open("/dev/ttyTHS2", O_RDWR | O_NOCTTY | O_NONBLOCK);
	
	cLog << prefix << "Open serial of \"/dev/ttyTHS2\"" << endl;
	
	cLog << prefix;
	perror("UART3: ");

	if (fdSerial == -1)
	{
		cLog << prefix << "Open serial failed." << endl;
		return;
	}

	termios tOption;

	tcgetattr(fdSerial, &tOption);
	cfmakeraw(&tOption);
	cfsetispeed(&tOption, B230400); // Recv baud rate
	cfsetospeed(&tOption, B230400); // Send baud rate
	tcsetattr(fdSerial, TCSANOW, &tOption);

	tOption.c_cflag &= ~PARENB;
	tOption.c_cflag &= ~CSTOPB;
	tOption.c_cflag &= ~CSIZE;
	tOption.c_cflag |= CS8;
	tOption.c_cflag &= ~INPCK;
	tOption.c_cflag |= (CLOCAL | CREAD);
	tOption.c_cflag &= ~(INLCR | ICRNL);
	tOption.c_cflag &= ~(IXON);
	tOption.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tOption.c_oflag &= ~OPOST;
	tOption.c_oflag &= ~(ONLCR | OCRNL);
	tOption.c_iflag &= ~(ICRNL | INLCR);
	tOption.c_iflag &= ~(IXON | IXOFF | IXANY);
	tOption.c_cc[VTIME] = 0;
	tOption.c_cc[VMIN]  = 0;

	tcflush(fdSerial, TCIOFLUSH);

	memset(&tFrame, 0, sizeof(tFrame));
	tFrame.bySOF = 0x66;
	tFrame.byEOF = 0x88;

	cLog << prefix << "Serial preparation is successful." << endl;
}

CSerial::~CSerial()
{
	tcflush(fdSerial, TCIOFLUSH);

	if (-1 == close(fdSerial))
	{
		cLog << prefix << "Close serial failed." << endl;
	}
}

void CSerial::SetLight(byte byLight, byte byR, byte byG, byte byB)
{
	tFrame.byDataType = FRMDAT_TYPE_LIGHT;
	tFrame.uData.tLight.byLight = byLight;
	tFrame.uData.tLight.byR = byR;
	tFrame.uData.tLight.byG = byG;
	tFrame.uData.tLight.byB = byB;

	if (!Send())
	{
		cLog << prefix << "Serial data send error! SetLight fail." << endl;
		return;
	}

	#ifdef DEBUG_SERIAL
	cLog << prefix << "Serial->SetLight:"
		 << " byLight: " << static_cast<int>(byLight)
		 << " byR: " << static_cast<int>(byR)
		 << " byG: " << static_cast<int>(byG)
		 << " byB: " << static_cast<int>(byB)
		 << endl;
	#endif
}

void CSerial::SetMotor(float fAngle)
{
	tFrame.byDataType = FRMDAT_TYPE_MOTOR;
	tFrame.uData.fMotorAngle = fAngle;

	if (!Send())
	{
		cLog << prefix << "Serial data send error! SetMotor fail." << endl;
		return;
	}

	#ifdef DEBUG_SERIAL
	cLog << prefix << "Serial->SetMotor:"
		 << " fAngle: " << fAngle << endl;
	#endif
}

float CSerial::GetUltrasonicDistance()
{
	tFrame.byDataType = FRMDAT_TYPE_ULTRS;
	tFrame.uData.fUltrsDist = -1;

	tcflush(fdSerial, TCIFLUSH);

	if (!Send())
	{
		cLog << prefix << "Serial data send error! GetUltrasonicDistance fail." << endl;
		return -1;
	}

	if (!Recv())
	{
		cLog << prefix << "Serial data recv error! GetUltrasonicDistance fail." << endl;
		return -1;
	}

	if (tFrame.byDataType != FRMDAT_TYPE_ULTRS)
	{
		cLog << prefix << "Data type is not correct. GetUltrasonicDistance fail."
			 << " Should be: 0x" << hex << FRMDAT_TYPE_ULTRS
			 << " Real recv: 0x" << hex << static_cast<int>(tFrame.byDataType)
			 << endl;
		return -1;
	}

	float fUltrsDist = tFrame.uData.fUltrsDist;

	#ifdef DEBUG_SERIAL
	cLog << prefix << "Serial->GetUltrasonicDistance:"
		 << " fUltrsDist: " << fUltrsDist << endl;
	#endif

	return fUltrsDist;
}

bool CSerial::Send()
{
	int iWrited = write(fdSerial, &tFrame, sizeof(tFrame));

	if (iWrited == -1)
	{
		cLog << "Write data to serial failed." << endl;
		return false;
	}
	else if (iWrited < int(sizeof(TFrame)))
	{
		cLog << prefix
			 << "Occur error in writing data to serial."
			 << "Need write: " << sizeof(tFrame) << ", "
			 << "Real write: " << iWrited << ". "
			 << "It may be serial output buffer has fulled."
			 << endl;
		return false;
	}

	#if DEBUG_SERIAL
	cLog << prefix << "Write data to serial successfully." << endl;
	#endif

	return true;
}

bool CSerial::Recv()
{
	int iReaded = 0;

	double dTimerStart = cLog.Timer();

	while (iReaded < int(sizeof(tFrame)))
	{
		double dUsedTime = cLog.Timer() - dTimerStart;

		#if DEBUG_SERIAL
		cLog << prefix << "Used time:" << dUsedTime << endl;
		#endif

		if (dUsedTime > 3) // Time limit is 3ms
		{
			cLog << prefix << "Read data from serial timeout." << endl;
			return false;
		}

		int iOnceReaded = read(fdSerial, ((unsigned char *)(&tFrame)) + iReaded, sizeof(tFrame) - iReaded);

		if (iOnceReaded == -1)
		{
			if (errno == EAGAIN)
			{
				continue;
			}

			cLog << prefix << "Read data from serial failed." << endl;
			return false;
		}

		iReaded += iOnceReaded;
	}

	if (tFrame.bySOF != 0x66 || tFrame.byEOF != 0x88)
	{
		cLog << prefix << "Data SOF or EOF is not correct." << endl;
		tcflush(fdSerial, TCIFLUSH);
		return false;
	}

	#if DEBUG_SERIAL
	cLog << prefix << "Success in reading data to serial." << endl;
	#endif
	
	cout << hex << (int)tFrame.uData.byData[0] << endl
		 << (int)tFrame.uData.byData[1] << endl
		 << (int)tFrame.uData.byData[2] << endl
		 << (int)tFrame.uData.byData[3] << endl;

	return true;
}

