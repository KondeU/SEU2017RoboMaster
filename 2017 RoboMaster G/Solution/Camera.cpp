// Camera.cpp
// Version 2.0
// started by Deyou Kong, 2017-07-02
// Checked by Deyou Kong, 2017-07-02
// Updated by Deyou Kong, 2017-07-20

#include <unistd.h>
#include <fcntl.h>

#include <opencv2/opencv.hpp>
using namespace cv;

#include <libv4l2.h>
#include <linux/videodev2.h>

#include "Camera.h"
#include "Logger.h"

// Use muti-thread to get camera data

void * CameraThread(void * pData); // Thread process

class CLock
{
public:

	void Lock()
	{
		while (bLock) usleep(1);	
		usleep(1);
		bLock = true;
		//cout << "Lock" << endl;
	}
	
	void Unlock()
	{	
		usleep(1);
		bLock = false;
		//cout << "Unlock" << endl;
	}

private:

	static bool bLock;
};
bool CLock::bLock = false;

class CCameraThread
{
public:

	CCameraThread() : cLog("Camera"), mCameraData(480, 640, CV_8UC3)
	{
		cLog << prefix << "Enable muti-thread to get camera data." << endl;

		bExit = false;

		pCapture = new VideoCapture(0);

		if (!(pCapture->isOpened()))
		{
			cLog << prefix << "Camera capture open camera error!" << endl;
			return;
		}

		pCapture->set(CV_CAP_PROP_FRAME_WIDTH,  640);
		pCapture->set(CV_CAP_PROP_FRAME_HEIGHT, 480);

		pthread_create(&tThread, nullptr, CameraThread, this);
		pthread_detach(tThread);
	}

	~CCameraThread()
	{
		bExit = true;

		if (pCapture)
		{
			delete pCapture;
		}
	}

	CLogger cLog;

	Mat mCameraData;
	VideoCapture * pCapture;

	bool bExit;
	pthread_t tThread;
};

void * CameraThread(void * pData)
{
	CLock cLock;
	CCameraThread * pCamThr = (CCameraThread *)pData;

	while (! pCamThr->bExit)
	{
		cLock.Lock();

		if (! pCamThr->pCapture->read(pCamThr->mCameraData))
		{
			pCamThr->cLog << prefix << "Read camera capture stream fail!" << endl;
		}

		cLock.Unlock();
	}

	return nullptr;
}

// Get camera data directly

class CCameraDirect
{
public:

	CCameraDirect() : cLog("Camera"), mCameraData(480, 640, CV_8UC3)
	{
		cLog << prefix << "Disenable muti-thread to get camera data." << endl;

		pCapture = new VideoCapture(0);

		if (!(pCapture->isOpened()))
		{
			cLog << prefix << "Camera capture open camera error!" << endl;
			return;
		}

		pCapture->set(CV_CAP_PROP_FRAME_WIDTH,  640);
		pCapture->set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	}

	~CCameraDirect()
	{
		if (pCapture)
		{
			delete pCapture;
		}
	}

	CLogger cLog;

	Mat mCameraData;
	VideoCapture * pCapture;
};

// Interface: CCamera

#ifdef CAMERA_ENABLE_THREAD

CCameraThread cCam;

bool CCamera::Grab()
{
	CLock cLock;
	cLock.Lock();
	cCam.mCameraData.copyTo(mCamRaw);
	pyrDown(mCamRaw, mCamDec);
	cLock.Unlock();
	
	return true;
}

#else

CCameraDirect cCam;

bool CCamera::Grab()
{
	if (cCam.pCapture->read(cCam.mCameraData))
	{
		cCam.mCameraData.copyTo(mCamRaw);
		pyrDown(mCamRaw, mCamDec);
	}
	else
	{
		cCam.cLog << prefix << "Read camera capture stream fail!" << endl;
		return false;
	}
	
	return true;
}

#endif

bool CCamera::SetExposure(bool bAuto, int iExposure)
{
	char szCameraVideo[16];
	sprintf(szCameraVideo, "/dev/video%d", 0); // Device ID is 0
	
	int fdDevice = v4l2_open(szCameraVideo, O_RDWR | O_NONBLOCK);
	if (fdDevice == -1)
	{
		cCam.cLog << prefix << "Camera V4L2 device open failed." << endl;
		return false;
	}
	
	struct v4l2_control tV4L2;
	tV4L2.id = V4L2_CID_EXPOSURE_AUTO;
	tV4L2.value = bAuto ? 3 : 1; // Here set the M/A, 1 is manual, 3 is auto
	
	if (v4l2_ioctl(fdDevice, VIDIOC_S_CTRL, &tV4L2) != 0)
	{
		 cCam.cLog << prefix << "Failed to set... " << strerror(errno) << endl;
		 v4l2_close(fdDevice);
		 return false;
	}
	
	if (!bAuto)
	{
		cCam.cLog << prefix << "Set exposure: " << iExposure << endl;
		v4l2_set_control(fdDevice, V4L2_CID_EXPOSURE_ABSOLUTE, iExposure);
	}

	v4l2_close(fdDevice);
	return true;
}

