// Main.cpp
// Version 2.0
// Started by Deyou Kong, 2017-06-30
// Checked by Deyou Kong, 2017-06-30
// Updated by Deyou Kong, 2017-07-20

#include "DJIHardDriverManifold.h"
#include "APIThread.h"
#include "conboardsdktask.h"
using namespace DJI;
using namespace DJI::onboardSDK;

#include "Logger.h"

#define ERR_SUCCESS 0
#define ERR_NOROOT  -1
#define ERR_UNKVER  1
#define ERR_STARTUP 2
#define ERR_CLEANUP 3
#define ERR_LOADSS  4
#define ERR_ACTIVE  5

const char szKeyLocation[16] = "./key.txt";

const char szDeviceName[16] = "/dev/ttyTHS1"; // UART2

const unsigned int uiBaudRate = 230400;

const Version verTarget = versionM100_31;

bool Startup();
bool Cleanup();
int Flightask(ConboardSDKScript & sdkScript);

#define MODE_HAVEFLIGHT
//#define MODE_DATATRANSPARENT // Communicate between Onboard and Remote, another define in _Task.cpp
#ifdef MODE_DATATRANSPARENT
const unsigned int uiDataBufferSize = 100;
void DataTransparent(DJI::onboardSDK::CoreAPI * api, Header * pHeader, void * pData);
#endif

// Framework logger output prefix
CLogger & framework(CLogger & cLogger)
{
	cout << "# ";
	return cLogger;
}

int main(int argc, char *argv[])
{
	CLogger cLog("Framework");

	// Make sure run as root
	if (0 != geteuid())
	{
		cLog << framework << "Program exited. Please run this execute as root!" << endl;
		return ERR_NOROOT;
	}

	// Output prop info
	cLog << framework << "Prop:" << endl;
	cLog << "\tKeyLocation: " << szKeyLocation << endl;
	cLog << "\tDeviceName: "  << szDeviceName  << endl;
	cLog << "\tBaudRate: "    << uiBaudRate    << endl;
	if (versionM100_31 == verTarget)
	{
		cLog << "\tTargetVersion: M100 with SDK_3.1" << endl;
	}
	else
	{
		cLog << "\tTargetVersion: Unknown!" << endl;
		cLog << framework << "Program exited. Target version is Unknown!" << endl;
		return ERR_UNKVER;
	}
	if (cLog.bFlightMove)
	{
		cLog << "\tFlightMove: true" << endl;
	}
	else
	{
		cLog << "\tFlightMove: false" << endl;
	}
	cLog << framework << "M100 config is valid and program run successfully." << endl;

	if (!Startup()) // Extend function
	{
		cLog << framework << "Program exited. Startup function return error." << endl;
		return ERR_STARTUP;
	}

	HardDriverManifold driver(szDeviceName, uiBaudRate);
	driver.init();

	CoreAPI api(&driver);
	ConboardSDKScript sdkScript(&api);
	api.setVersion(verTarget);

	ScriptThread st(&sdkScript);

	// &sdkScript <=> st.script

	APIThread send(&api, 1);
	APIThread read(&api, 2);
	send.createThread();
	read.createThread();

	char * pszLoad = new char [strlen("--SS load ") + strlen(szKeyLocation) + 1];
	sprintf(pszLoad, "--SS load %s", szKeyLocation);

	if (!loadSS(&sdkScript, (UserData)pszLoad))
	{
		cLog << framework
			 << "Could not load key or problem in format. "
			 << "Make sure your key is in location " 
			 << szKeyLocation
			 << " and in the right format. "
			 << "And then retry activation."
			 << endl;
		exit(ERR_LOADSS);
	}
	else // Load SS successfully
	{
		bool Active(ConboardSDKScript & sdkScript);
		if (!Active(sdkScript))
		{
			cLog << framework << "Could not Active. Program exited." << endl;
			return ERR_ACTIVE;
		}
	}

	cLog << framework << "Load key and active successfully." << endl;

	#ifdef MODE_DATATRANSPARENT
	uint8_t byData[uiDataBufferSize] = { 0 };
	CallBackHandler hFromMobileEntrance;
	hFromMobileEntrance.callback = DataTransparent;
	hFromMobileEntrance.userData = &byData;
	api.setFromMobileCallback(hFromMobileEntrance);
	#endif

	int iTaskRet = Flightask(sdkScript); // Extend function
	cLog << framework << "Flightask function return. Value is " << iTaskRet << "." << endl;

	if (!Cleanup()) // Extend function
	{
		cLog << framework << "Program exited. Cleanup function return error." << endl;
		return ERR_CLEANUP;
	}

	return 0;
}

#ifdef MODE_HAVEFLIGHT

bool Active(ConboardSDKScript & sdkScript)
{
	// All "std::" are omitted.

	bool bActiveSuccess = false; // Addition

	uint8_t activationStatus = 99;

	void activateCallbackUser(DJI::onboardSDK::CoreAPI *api, Header *protocolHeader, void * activationStatus); // Addition
	sdkScript.getApi()->activate(&sdkScript.adata, activateCallbackUser, &activationStatus);
	usleep(500000);

	switch (activationStatus)
	{
	case ACK_ACTIVE_SUCCESS:
		bActiveSuccess = true; // Addition
		cout << "Automatic activation successful." << endl;
		usleep(3000000);
		break;
	case ACK_ACTIVE_NEW_DEVICE:
		cout << "Your activation did not go through. \nThis is a new device. \nMake sure DJI GO is turned on and connected to the internet \nso you can contact the server for activation. \nActivate with '--SS load ../key.txt' followed by '--CA ac'." << endl;
		usleep(3000000);
		break;
	case ACK_ACTIVE_PARAMETER_ERROR:
		cout << "Your activation did not go through. \nThere was a parameter error. \nPlease check your setup and retry." << endl;
		usleep(3000000);
		break;
	case ACK_ACTIVE_ENCODE_ERROR:
		cout << "Your activation did not go through. \nThere was an encoding error. \nPlease check your setup and retry." << endl;
		usleep(3000000);
		break;
	case ACK_ACTIVE_APP_NOT_CONNECTED:
		cout << "Your activation did not go through. \nDJI Go does not seem to be connected. \nPlease check your mobile device and activate with '--SS load ../key.txt' followed by '--CA ac'." << endl;
		usleep(3000000);
		break;
	case ACK_ACTIVE_NO_INTERNET:
		cout << "Your activation did not go through. \nYour mobile device doesn't seem to be connected to the internet. \nPlease check your connection and activate with '--SS load ../key.txt' followed by '--CA ac'." << endl;
		usleep(3000000);
		break;
	case ACK_ACTIVE_SERVER_REFUSED:
		cout << "Your activation did not go through. \nThe server refused your credentials. \nPlease check your DJI developer account details in ../key.txt." << endl;
		usleep(3000000);
		break;
	case ACK_ACTIVE_ACCESS_LEVEL_ERROR:
		cout << "Your activation did not go through. \nYou don't seem to have the right DJI SDK permissions. \nPlease check your DJI developer account details." << endl;
		usleep(3000000);
		break;
	case ACK_ACTIVE_VERSION_ERROR:
		cout << "Your activation did not go through. \nYour SDK version in User_config.h does not match the one reported by the drone. \nPlease correct that, rebuild and activate." << endl;
		usleep(3000000);
		break;
	default:
		cout << "There was an error with the activation command. Perhaps your drone is not powered on? \nPlease check your setup and retry." << endl;
		usleep(3000000);
		break;
	}

	return bActiveSuccess; // Addition
}

void activateCallbackUser(DJI::onboardSDK::CoreAPI *api, Header *protocolHeader, void * activationStatus)
{
	uint8_t ack_data;
	if (protocolHeader->length - EXC_DATA_SIZE <= 2)
	{
		memcpy((unsigned char *)&ack_data, ((unsigned char *)protocolHeader) + sizeof(Header),
			(protocolHeader->length - EXC_DATA_SIZE));
		if (ack_data == ACK_ACTIVE_SUCCESS)
		{
			if (api->getAccountData().encKey)
				api->setKey(api->getAccountData().encKey);
		}
		uint8_t *acStatPtr = (uint8_t *)activationStatus;
		*acStatPtr = ack_data;
	}
	else
	{
		//std::cout << "Automatic Activation Unsuccessful. Please try with '--SS load' and '--CA ac' " << std::endl;
		cout << "Automatic Activation Unsuccessful! Please try again." << endl; // Addition
	}
}

#else

bool Active(ConboardSDKScript & sdkScript)
{
	cout << "Donot find a linked flight." << endl;
	return true;
}

#endif

