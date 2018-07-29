// Logger.h
// Version 2.0
// started by Deyou Kong, 2017-06-30
// Checked by Deyou Kong, 2017-06-30

#pragma once

#include "CConfig.h"
#include "CVideolog.h"

class CLogger
{
#define TEAM_BLUE false
#define TEAM_RED  true

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
