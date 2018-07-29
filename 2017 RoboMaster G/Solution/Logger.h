// Logger.h
// Version 2.0
// started by Deyou Kong, 2017-06-30
// Checked by Deyou Kong, 2017-06-30
// Updated by Deyou Kong, 2017-07-20

#pragma once

#include <opencv2/opencv.hpp>

#include "CConfig.h"
#include "CVideolog.h"

class CLogger
{
#define TEAM_BLUE false
#define TEAM_RED  true

#define LOC_ORG 7
#define LOC_LIM 8
#define LOC_CRNT 9
#define LOC_DEST 11

public:

	static int iCurrentLogger;

	CLogger(string szLogFile);
	~CLogger();

	static CVideolog * pVideolog; // Video logger, must be single, use it directly

	ofstream fout; // File logger

	template<typename T> friend CLogger & operator<<(CLogger & cLogger, const T & t);
	CLogger & operator<<(CLogger & (*pFunc)(CLogger&));

	static int iLogCounter;
	static bool bTeam;

	static bool bFlightMove;
	static bool bShowVideolog;
	static bool bSaveVideolog;
	static bool bSaveRaw;
	static double dTagSize;

	static double dGolf[10];
	static cv::Point2d tLoc[12]; // Tag location and current and target location

	double Timer();
};

template<typename T>
inline CLogger & operator<<(CLogger & cLogger, const T & t)
{
	cout << t;
	cLogger.fout << t;
	return cLogger;
}

inline CLogger & CLogger::operator<<(CLogger &(*pFunc)(CLogger &))
{
	return pFunc(*this);
}

inline CLogger & endl(CLogger & cLogger)
{
	cout << endl;
	cLogger.fout << endl;
	return cLogger;
}

inline CLogger & ends(CLogger & cLogger)
{
	cout << ends;
	cLogger.fout << ends;
	return cLogger;
}

inline CLogger & prefix(CLogger & cLogger)
{
	cout << "$ ";
	return cLogger;
}
