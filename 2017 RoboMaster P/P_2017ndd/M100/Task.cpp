// Task.cpp
// Version 1.0
// Writed by Deyou Kong, 2016-12-05
// Checked by Deyou Kong, 2016-05-04

#include "Headers.h"

#ifdef MODE_DATATRANSPARENT
// If use a structure, should use the "#pragma pack(1)"
void DataTransparent(DJI::onboardSDK::CoreAPI * api, Header * pHeader, void * pData)
{
	// Data start is "((char*)(pHeader))[14]"
	// TODO
	void RM_DataTransparent(void * pData);
	RM_DataTransparent(&(pHeader[14]));
	return;
}
#endif

bool Startup()
{
	// TODO
	return true;
}

bool Cleanup()
{
	// TODO
	return true;
}

int Flightask(ConboardSDKScript & sdkScript)
{
	// TODO
	int RM_Task(ConboardSDKScript & sdkScript);
	RM_Task(sdkScript);
	return 0;
}

