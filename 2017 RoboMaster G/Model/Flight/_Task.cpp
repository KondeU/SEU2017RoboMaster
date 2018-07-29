// Task.cpp
// Version 2.0
// Writed by Deyou Kong, 2017-06-30
// Checked by Deyou Kong, 2017-06-30

//#define MODE_DATATRANSPARENT // Communicate between Onboard and Remote, another define in _Main.cpp
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
	int RMTask(ConboardSDKScript & sdkScript);
	RMTask(sdkScript);
	return 0;
}

