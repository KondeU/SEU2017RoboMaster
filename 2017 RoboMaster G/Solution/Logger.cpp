// Logger.cpp
// Version 2.0
// started by Deyou Kong, 2017-06-30
// Checked by Deyou Kong, 2017-06-30
// Updated by Deyou Kong, 2017-07-20

#include <climits>
#include <sys/time.h>

#include <opencv2/opencv.hpp>
using namespace cv;

#include "Logger.h"

char szVideologFile[NAME_MAX + 1] = "./Runlog/Videolog"; // Used in CVideolog.cpp to create Videolog file

int CLogger::iCurrentLogger = 0;

CVideolog * CLogger::pVideolog = nullptr;

int CLogger::iLogCounter = 0;
bool CLogger::bTeam = TEAM_RED; // Default team is red

bool CLogger::bFlightMove = false;
bool CLogger::bShowVideolog = true;
bool CLogger::bSaveVideolog = true;
bool CLogger::bSaveRaw = false;
double CLogger::dTagSize = 0.167;

double CLogger::dGolf[10] =
	{
		10, -10,
		20, -20,
		30, -30,
		40, -40,
		50, -50
	};

Point2d CLogger::tLoc[12] = 
	{
		// Point(x, y)
		Point(1.9, 0.1), // TagID = 0
		Point(1.9,   1), // TagID = 1
		Point(1.9, 1.9), // TagID = 2
		Point(  1, 0.1), // TagID = 3
		Point(  1, 1.9), // TagID = 4
		Point(0.1, 0.1), // TagID = 5
		Point(0.1,   1), // TagID = 6
		Point(  0,   0), // Orgin
		Point(  2,   2), // Limit
		Point(  1,   1), // Current
		Point(0.1, 1.9), // TagID = 10
		Point(  1,   1)  // Target
	};

CLogger::CLogger(string szLogFile)
{
	if (iCurrentLogger == 0) // The first instance
	{
		// Load config file to get some params

		CConfig cCfg("Run.cfg");

		if (cCfg.LoadFile()) // Load config file successfully
		{
			sprintf(szVideologFile, "%s%s.avi",
				szVideologFile, cCfg.GetString("default", "iLogCounter").c_str());

			iLogCounter = cCfg.GetInteger("default", "iLogCounter");

			bTeam = cCfg.GetBoolean("default", "bIsRed", true);

			bFlightMove = cCfg.GetBoolean("Aircraft", "bFlightMove");

			bShowVideolog = cCfg.GetBoolean("Aircraft", "bShowVideolog", true);
			bSaveVideolog = cCfg.GetBoolean("Aircraft", "bSaveVideolog", true);

			bSaveRaw = cCfg.GetBoolean("Aircraft", "bSaveRaw", false);

			dTagSize = cCfg.GetDouble("Aircraft", "dTagSize");

			for (int i = 0; i < 10; i++)
			{
				char szKey[8];
				sprintf(szKey, "dGolf%d", i);
				dGolf[i] = cCfg.GetDouble("Motor", szKey);
			}
			
			// TagID from 0-6
			for (int i = 0; i < 7; i++)
			{
				char szKey[8];
				sprintf(szKey, "dTag%dx", i);
				tLoc[i].x = cCfg.GetDouble("Location", szKey);
				sprintf(szKey, "dTag%dy", i);
				tLoc[i].y = cCfg.GetDouble("Location", szKey);
			}
			// TagID 10
			tLoc[10].x = cCfg.GetDouble("Location", "dTag7x");
			tLoc[10].y = cCfg.GetDouble("Location", "dTag7y");

			tLoc[LOC_ORG].x = cCfg.GetDouble("Location", "dMapOrgX", 0.0);
			tLoc[LOC_ORG].y = cCfg.GetDouble("Location", "dMapOrgY", 0.0);

			tLoc[LOC_LIM].x = cCfg.GetDouble("Location", "dMapLimX", 2.0);
			tLoc[LOC_LIM].y = cCfg.GetDouble("Location", "dMapLimY", 2.0);

			cCfg.SetInteger("default", "iLogCounter", cCfg.GetInteger("default", "iLogCounter") + 1);
			cCfg.SaveFile();
		}
		else // Load config file error, use default params
		{
			sprintf(szVideologFile, "%s.avi", szVideologFile);
		}

		pVideolog = new CVideolog(240, 320, bShowVideolog, bSaveVideolog, CV_8UC1); // 8UC1: Gray
	}

	if (iLogCounter > 0) // First instance load config successfully
	{
		stringstream ss;
		ss << "./Runlog/" << szLogFile << iLogCounter << ".log";
		ss >> szLogFile;
	}
	else
	{
		stringstream ss;
		ss << "./Runlog/" << szLogFile << ".log";
		ss >> szLogFile;
	}
	fout.open(szLogFile);

	iCurrentLogger++;
}

CLogger::~CLogger()
{
	iCurrentLogger--;

	fout.close();

	if (iCurrentLogger <= 0) // The last instance
	{
		if (pVideolog)
		{
			delete pVideolog;
		}
	}
}

double CLogger::Timer()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return ((double(tv.tv_sec) * 1000) + (double(tv.tv_usec) / 1000));
}
