#include "Guidance.h"

//#include "GuidanceSDK/DJI_utility.h"

#define RETURN_IF_ERR(iRet) {if(iRet){release_transfer(); GuidanceError(iRet); return false;}}

int GuidanceCallback(int iDataType, int iDataLen, char * byContent);

DJI_lock  cLock;
DJI_event cEvent;

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
	cout << "$ cu\tcv\tfocal\tbaseline" << endl;
	for (int i = 0; i<CAMERA_PAIR_NUM; i++)
	{
		cout << "$ " << tCali[i].cu << "\t\t" << tCali[i].cv << "\t" << tCali[i].focal << "\t" << tCali[i].baseline << endl;
	}
	fout << "cu\tcv\tfocal\tbaseline" << endl;
	for (int i = 0; i<CAMERA_PAIR_NUM; i++)
	{
		fout << tCali[i].cu << "\t\t" << tCali[i].cv << "\t" << tCali[i].focal << "\t" << tCali[i].baseline << endl;
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
	//select_ultrasonic();
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
	cout << "$ Standard:(width, height)=" << this->iWidth << ", " << this->iHeight << endl;
	cout << "$ GetSensor:(width, height)=" << iWidth << ", " << iHeight << endl;
	fout << "Standard:(width, height)=" << this->iWidth << ", " << this->iHeight << endl;
	fout << "GetSensor:(width, height)=" << iWidth << ", " << iHeight << endl;

	// Set exposure
	exposure_param tExpo;
	tExpo.m_step = 10;
	tExpo.m_exposure_time = 7.25;
	tExpo.m_expected_brightness = 120;
	tExpo.m_is_auto_exposure = 1;
	tExpo.m_camera_pair_index = VBUS_D;
	set_exposure_param(&tExpo);

	// Set callback function
	iRet = set_sdk_event_handler(GuidanceCallback);
	RETURN_IF_ERR(iRet);

	// Start
	iRet = start_transfer();
	RETURN_IF_ERR(iRet);

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
		
	/*
		enum e_sdk_err_code
		{
			e_timeout = -7,						// USB传输超时
			e_libusb_io_err = -1,				// libusb库IO错误
			e_sdk_no_err = 0,					// 成功，没有错误
			e_load_libusb_err=1,				// 加载的libusb库错误
			e_sdk_not_inited=2,					// SDK软件还没有准备好
			e_hardware_not_ready=3,				// Guidance硬件还没有准备好
			e_disparity_not_allowed=4,			// 视差图或深度图不允许被选择
			e_image_frequency_not_allowed=5,	// 图像频率必须是枚举类型e_image_data_frequecy之一
			e_config_not_ready=6,				// 配置没有准备好
			e_online_flag_not_ready=7,			// 在线标志没有准备好
			e_stereo_cali_not_ready = 8,    	// 摄像头标定参数没有准备好
			e_max_sdk_err = 100					// 错误最大数量
		};
	*/
	}

	cout << "$ " << szOutput << endl;
	fout << szOutput << "\n" << endl;
}

extern Mat mDownGreyL;
//extern Mat mDownGreyR;

int GuidanceCallback(int data_type, int data_len, char * content)
{
	cLock.enter();

	if (e_image == data_type && NULL != content)
	{
		image_data * data = (image_data*)content;

		if (data->m_greyscale_image_left[VBUS_D])
		{
			mDownGreyL = Mat::zeros(CGuidance::iHeight, CGuidance::iWidth, CV_8UC1);
			memcpy(mDownGreyL.data, data->m_greyscale_image_left[VBUS_D], CGuidance::iImageSize);
		}
		else
		{
			mDownGreyL.release();
		}
		/*if (data->m_greyscale_image_right[VBUS_D])
		{
			mDownGreyR = Mat::zeros(CGuidance::iHeight, CGuidance::iWidth, CV_8UC1);
			memcpy(mDownGreyR.data, data->m_greyscale_image_right[VBUS_D], CGuidance::iImageSize);
		}
		else
		{
			mDownGreyR.release();
		}*/
	}

	cLock.leave();
	cEvent.set_event();
	return 0;
}
