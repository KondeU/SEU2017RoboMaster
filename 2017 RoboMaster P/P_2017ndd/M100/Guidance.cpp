// Guidance.cpp
// Version 1.0
// Writed by Deyou Kong, 2017-04-17
// Checked by Deyou Kong, 2017-05-04

#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/time.h>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include "DJI_guidance.h"

#include "DJI_utility.h" // use DJI_lock and DJI_event

#include "Guidance.h"
#include "Mode.h"

#ifdef MODE_DEBUG_GUIDANCE
#ifdef VBUS_D
#undef VBUS_D
#endif
#define VBUS_D e_vbus1 // Using in debug mode, use forward instead
#endif

#define RETURN_IF_ERR(iRet) {if(iRet){release_transfer(); GuidanceError(iRet); return false;}}

int GuidanceCallback(int iDataType, int iDataLen, char * byContent);

extern float fGuidFocal; // Custom data
extern float fGuidCU; // Custom data
extern float fGuidCV; // Custom data

DJI_lock  cLock;
DJI_event cEvent;

CGuidance::CGuidance()
{
	#ifdef MODE_AUTOSTART
	fout.open("./autorun/2017RobomasterTask/Runlog/Guidance.log");
	#else
	fout.open("./Runlog/Guidance.log");
	#endif
}

CGuidance::~CGuidance()
{
	fout.close();
}

double CGuidance::Timer()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return ((double(tv.tv_sec)*1000) + (double(tv.tv_usec)/1000));
}

bool CGuidance::Init()
{
	int iRet = 0;

	// Reset select
	reset_config();

	// Create transfer thread
	iRet = init_transfer();
	RETURN_IF_ERR(iRet);

	// Guidance online state
	int iGuidanceState[CAMERA_PAIR_NUM];
	iRet = get_online_status(iGuidanceState);
	RETURN_IF_ERR(iRet);
	cout << "$ Sensor online status: ";
	for (int i = 0; i<CAMERA_PAIR_NUM; i++)
		cout << iGuidanceState[i] << " ";
	cout << endl;
	fout << "Sensor online status: ";
	for (int i = 0; i<CAMERA_PAIR_NUM; i++)
		fout << iGuidanceState[i] << " ";
	fout << "\n" << endl;

	// Get cali param
	stereo_cali tCali[CAMERA_PAIR_NUM];
	iRet = get_stereo_cali(tCali);
	RETURN_IF_ERR(iRet);
	cout << "$ cu\t\tcv\t\tfocal\tbaseline" << endl;
	for (int i = 0; i<CAMERA_PAIR_NUM; i++)
	{
		cout << "$ " << tCali[i].cu << "\t" << tCali[i].cv << "\t" << tCali[i].focal << "\t" << tCali[i].baseline << endl;
	}
	fout << "cu\t\tcv\t\tfocal\tbaseline" << endl;
	for (int i = 0; i<CAMERA_PAIR_NUM; i++)
	{
		fout << tCali[i].cu << "\t" << tCali[i].cv << "\t" << tCali[i].focal << "\t" << tCali[i].baseline << endl;
	}
	fout << endl;

	// Select data
	iRet = select_greyscale_image(VBUS_D, IMGREY_L);
	RETURN_IF_ERR(iRet);
	//iRet = select_greyscale_image(VBUS_D, IMGREY_R);
	//RETURN_IF_ERR(iRet);

	//err_code = select_depth_image(sensor_id);
	//RETURN_IF_ERR(err_code);
	//err_code = select_disparity_image(sensor_id);
	//RETURN_IF_ERR(err_code);

	//select_imu();
	select_ultrasonic();
	//select_obstacle_distance();
	//select_velocity();
	//select_motion();

	// Device type
	e_device_type dt;
	get_device_type(&dt);
	cout << "$ Device type: " << (dt == Guidance ? "Guidance" : "GuidanceLite") << endl;
	fout << "Device type: " << (dt == Guidance ? "Guidance" : "GuidanceLite") << endl;

	// Image size
	int iWidth = 0, iHeight = 0;
	get_image_size(&iWidth, &iHeight);
	cout << "$ Standard:(width, height)= " << this->iWidth << ", " << this->iHeight << endl;
	cout << "$ GetSensor:(width, height)= " << iWidth << ", " << iHeight << endl;
	fout << "Standard:(width, height)= " << this->iWidth << ", " << this->iHeight << endl;
	fout << "GetSensor:(width, height)= " << iWidth << ", " << iHeight << endl;

	// Set exposure
	exposure_param tExpo;
	tExpo.m_step = 10;
	tExpo.m_exposure_time = 7.25;
	tExpo.m_expected_brightness = 100;
	tExpo.m_is_auto_exposure = 1;
	tExpo.m_camera_pair_index = VBUS_D;
	set_exposure_param(&tExpo);

	// Set callback function
	iRet = set_sdk_event_handler(GuidanceCallback);
	RETURN_IF_ERR(iRet);

	// Set frequency
	iRet = set_image_frequecy(e_frequecy_20); // Freq:20Hz
	RETURN_IF_ERR(iRet);

	// Start
	iRet = start_transfer();
	RETURN_IF_ERR(iRet);

	// TODO...
	
	fGuidFocal = tCali[VBUS_D].focal;
	fGuidCU = tCali[VBUS_D].cu;
	fGuidCV = tCali[VBUS_D].cv;

	return true;
}

bool CGuidance::Deinit()
{
	int iRet = 0;

	// Stop
	iRet = stop_transfer();
	RETURN_IF_ERR(iRet);

	// Delete transfer thread
	iRet = release_transfer();
	RETURN_IF_ERR(iRet);

	return true;
}

void CGuidance::GetData()
{
	cEvent.wait_event();
}

// Extra function, date 2017-05-10
void CGuidance::SetExposure(int iExpectedBrightness)
{
	exposure_param tExpo;
	tExpo.m_step = 10;
	tExpo.m_exposure_time = 7.25;
	tExpo.m_expected_brightness = iExpectedBrightness;
	tExpo.m_is_auto_exposure = 1;
	tExpo.m_camera_pair_index = VBUS_D;
	set_exposure_param(&tExpo);
}

void CGuidance::GuidanceError(int iErr)
{
	char szOutput[1024] = "Error occur in guidance: ";

	switch ((e_sdk_err_code)iErr)
	{
	default:
		strcpy(szOutput, "Unknown");
		break;

	case e_timeout: // -7
		strcpy(szOutput, "USB translate timeout");
		break;

	case e_libusb_io_err: // -1
		strcpy(szOutput, "libusb library IO error");
		break;

	//case e_sdk_no_err: // 0
	case 0:
		strcpy(szOutput, "No error");
		break;

	case e_load_libusb_err: // 1
		strcpy(szOutput, "Loaded libusb occur error");
		break;

	case e_sdk_not_inited: // 2
		strcpy(szOutput, "GuidanceSDK software is not ready");
		break;

	case e_hardware_not_ready: // 3
		strcpy(szOutput, "Guidance hardware is not ready");
		break;

	case e_disparity_not_allowed: // 4
		strcpy(szOutput, "Disparity or Depth not allowed");
		break;

	case e_image_frequency_not_allowed: // 5
		strcpy(szOutput, "Image frequency is not a type of e_image_data_frequecy");
		break;

	case e_config_not_ready: // 6
		strcpy(szOutput, "Config is not ready");
		break;

	case e_online_flag_not_ready: // 7
		strcpy(szOutput, "Online flag is not ready");
		break;

	case e_stereo_cali_not_ready: // 8
		strcpy(szOutput, "Stereo cali value is not ready");
		break;
	}

	cout << "$ " << szOutput << endl;
	fout << szOutput << "\n" << endl;
}

extern Mat mDownGreyL; // Custom data
//extern Mat mDownGreyR; // Custom data

extern short sGuidDist; // Custom data

int GuidanceCallback(int data_type, int data_len, char * content)
{
	// Lock
	cLock.enter();
	
	// TODO...
	
	if (e_image == data_type && NULL != content)
	{
		image_data * data = (image_data*)content;

		if (data->m_greyscale_image_left[VBUS_D])
		{
			mDownGreyL.release();
			mDownGreyL = Mat::zeros(CGuidance::iHeight, CGuidance::iWidth, CV_8UC1);
			memcpy(mDownGreyL.data, data->m_greyscale_image_left[VBUS_D], CGuidance::iImageSize);
		}
		/*if (data->m_greyscale_image_right[VBUS_D])
		{
			mDownGreyR.release();
			mDownGreyR = Mat::zeros(CGuidance::iHeight, CGuidance::iWidth, CV_8UC1);
			memcpy(mDownGreyR.data, data->m_greyscale_image_right[VBUS_D], CGuidance::iImageSize);
		}*/
	}
	
	if (e_ultrasonic == data_type && NULL != content)
	{
		ultrasonic_data * data = (ultrasonic_data*)content;
		
		if (data->reliability[VBUS_D])
		{
			sGuidDist = data->ultrasonic[VBUS_D];
		}
	}

	// Unlock and set an event
	cLock.leave();
	cEvent.set_event();
	return 0;
}

