// Main.cpp
// Version 1.0
// Writed by Deyou Kong, 2016-12-05
// Checked by Deyou Kong, 2017-05-04

#include "Headers.h"

#define ERR_SUCCESS 0
#define ERR_NOROOT  -1
#define ERR_UNKVER  1
#define ERR_STARTUP 2
#define ERR_CLEANUP 3
#define ERR_LOADSS  4
#define ERR_ACTIVE  5

#ifdef MODE_AUTOSTART
// Please make sure key file existed in the Home location.
const char szKeyLocation[32] = "./autorun/key.txt"; // Dir:/home/ubuntu/Autorun/key.txt
#else
const char szKeyLocation[32] = "./key.txt";
#endif

const char szDeviceName[32] = "/dev/ttyTHS1"; // UART2

const unsigned int uiBaudRate = 230400;

const Version verTarget = versionM100_31;

bool Startup();
bool Cleanup();
int Flightask(ConboardSDKScript & sdkScript);

#ifdef MODE_DATATRANSPARENT
const unsigned int uiDataBufferSize = 100;
void DataTransparent(DJI::onboardSDK::CoreAPI * api, Header * pHeader, void * pData);
#endif

int main(int argc, char *argv[])
{
	#ifdef MODE_AUTOSTART
	ofstream fout("./autorun/2017RobomasterTask/Runlog/Main.log");
	#else
	ofstream fout("./Runlog/Main.log");
	#endif

	// Make sure run as root
	if (0 != geteuid())
	{
		cout << "# Please run this execute as root!" << endl;
		fout << "Please run this execute as root! And program exited." << endl;
		fout.close();
		return ERR_NOROOT;
	}

	// Output prop info to file
	fout << "Prop:" << endl;
	fout << "\tKeyLocation: " << szKeyLocation << endl;
	fout << "\tDeviceName: "  << szDeviceName  << endl;
	fout << "\tBaudRate: "    << uiBaudRate    << endl;
	if (versionM100_31 == verTarget)
	{
		fout << "\tTargetVersion: M100 with SDK_3.1" << endl;
	}
	else
	{
		cout << "# Target version is Unknown! Program will exit." << endl;
		fout << "\tTargetVersion: Unknown!" << endl;
		fout << "And program exited." << endl;
		fout.close();
		return ERR_UNKVER;
	}
	#ifdef MODE_AUTOSTART
	fout << "\tAutoStart: true" << endl;
	#else
	fout << "\tAutoStart: false" << endl;
	#endif
	#ifdef MODE_FLIGHTMOVE
	fout << "\tFlightMove: true" << endl;
	#else
	fout << "\tFlightMove: false" << endl;
	#endif
	cout << "# M100 config is valid and program run successfully." << endl;
	fout << "M100 config is valid and program run successfully.\n" << endl;

	if (!Startup()) // Extend function
	{
		cout << "# Startup function return error. Program will exit." << endl;
		fout << "Startup function return error. Program exited." << endl;
		fout.close();
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
		cout << "# Could not load key or problem in format. "
			 << "Make sure your key is in location "
			 << szKeyLocation
			 << " and in the right format. "
			 << "And then retry activation."
			 << endl;
		fout << "Could not load key or problem in format. "
			 << "Make sure your key is in location " 
			 << szKeyLocation
			 << " and in the right format. "
			 << "And then retry activation."
			 << endl;
		fout.close();
		exit(ERR_LOADSS);
	}
	else // Load SS successfully
	{
		bool Active(ConboardSDKScript & sdkScript);
		if (!Active(sdkScript))
		{
			cout << "# Could not Active. Program will exit." << endl;
			fout << "Could not Active. Program exited." << endl;
			fout.close();
			return ERR_ACTIVE;
		}
	}

	cout << "# Load key and active successfully." << endl;
	fout << "Load key and active successfully.\n" << endl;

	#ifdef MODE_DATATRANSPARENT
	uint8_t byData[uiDataBufferSize] = { 0 };
	CallBackHandler hFromMobileEntrance;
	hFromMobileEntrance.callback = DataTransparent;
	hFromMobileEntrance.userData = &byData;
	api.setFromMobileCallback(hFromMobileEntrance);
	#endif

	int iTaskRet = Flightask(sdkScript); // Extend function
	cout << "# Flightask function return. Value is " << iTaskRet << "." << endl;
	fout << "Flightask function return. Value is " << iTaskRet << ".\n" << endl;

	if (!Cleanup()) // Extend function
	{
		cout << "# Cleanup function return error. Program will exit." << endl;
		fout << "Cleanup function return error. Program exited." << endl;
		fout.close();
		return ERR_CLEANUP;
	}

	fout.close();

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

