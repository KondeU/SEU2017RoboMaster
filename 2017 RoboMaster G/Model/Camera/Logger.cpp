// Logger.cpp
// Version 2.0
// started by Deyou Kong, 2017-06-30
// Checked by Deyou Kong, 2017-06-30

#include <climits>
#include <sys/time.h>

#include "Logger.h"

char szVideologFile[NAME_MAX + 1] = "./Runlog/Videolog"; // Used in CVideolog.cpp to create Videolog file

int CLogger::iCurrentLogger = 0;

CVideolog * CLogger::pVideolog = nullptr;

int CLogger::iLogCounter = 0;
bool CLogger::bTeam = TEAM_RED;
bool CLogger::bFlightMove = false;
bool CLogger::bShowVideolog = true;
bool CLogger::bSaveVideolog = true;

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

			cCfg.SetInteger("default", "iLogCounter", cCfg.GetInteger("default", "iLogCounter") + 1);
			cCfg.SaveFile();
		}
		else // Load config file error, use default params
		{
			sprintf(szVideologFile, "%s.avi", szVideologFile);

			iLogCounter = 0; // Make sure "iLogCounter > 0" in config file

			bTeam = true; // Mention! Default is red
			//bTeam = false; // Mention! Default is blue

			bFlightMove = false;

			bShowVideolog = true;
			bSaveVideolog = true;
		}

		pVideolog = new CVideolog(240, 320, bShowVideolog, bSaveVideolog, CV_8UC3);
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
