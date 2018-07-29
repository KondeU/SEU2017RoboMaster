// Camera.cpp
// Version 2.0
// started by Deyou Kong, 2017-07-02
// Checked by Deyou Kong, 2017-07-02

#include <unistd.h>

#include <opencv2/opencv.hpp>
using namespace cv;

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

CCameraThread cCamThr;

void CCamera::Grab()
{
	CLock cLock;
	cLock.Lock();
	cCamThr.mCameraData.copyTo(mCamRaw);
	pyrDown(mCamRaw, mCamDec);
	cLock.Unlock();
}

#else

CCameraDirect cCamDir;

void CCamera::Grab()
{
	if (cCamDir.pCapture->read(cCamDir.mCameraData))
	{
		cCamDir.mCameraData.copyTo(mCamRaw);
		pyrDown(mCamRaw, mCamDec);
	}
	else
	{
		cCamDir.cLog << prefix << "Read camera capture stream fail!" << endl;
	}
}

#endif


