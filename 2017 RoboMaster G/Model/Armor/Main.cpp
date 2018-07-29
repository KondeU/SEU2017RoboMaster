#include <iostream>
#include <vector>
using namespace std;

#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>

#include <libv4l2.h>
#include <linux/videodev2.h>

#include <opencv2/opencv.hpp>
using namespace cv;

#include "Armor.h"

#define PRE(s) ("$ " s)

double Timer()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return ((double(tv.tv_sec) * 1000) + (double(tv.tv_usec) / 1000));
}

int main()
{
// V4L2 to set exposure

	char szCameraVideo[16];
	sprintf(szCameraVideo, "/dev/video%d", 0); // Device ID is 0
	
	int fdDevice = v4l2_open(szCameraVideo, O_RDWR | O_NONBLOCK);
	if (fdDevice == -1)
	{
		cout << "Camera V4L2 device open failed." << endl;
	}
	
	int iExposure = 1400; // Exposure
	struct v4l2_control tV4L2;
	tV4L2.id = V4L2_CID_EXPOSURE_AUTO;
	tV4L2.value = 1; // Here set the M/A, 1 is manual, 3 is auto
	if (v4l2_ioctl(fdDevice, VIDIOC_S_CTRL, &tV4L2) != 0)
	{
		cout << "Failed to set... " << strerror(errno) << endl;
	}
	cout << "Set exposure: " << iExposure << endl;
	
	v4l2_set_control(fdDevice, V4L2_CID_EXPOSURE_ABSOLUTE, iExposure);
	
	v4l2_close(fdDevice);

// Armor detect

	CArmor cArmor;

    VideoCapture cCaptrue(0);

    if (!cCaptrue.isOpened())
    {
		cout << PRE("Fail to open camera capture!") << endl;
		exit(1);
    }

	cCaptrue.set(CV_CAP_PROP_FRAME_WIDTH,  640);
	cCaptrue.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

    Mat mFrame;

    while (true)
    {
		double dTimeStart = Timer();

		if (! (cCaptrue.read(mFrame)))
		{
			cout << PRE("Fail to read camera data!") << endl;
			exit(2);
		}

		imshow("Frame", mFrame);

		static int iLog = 1;

		char cKey = waitKey(1);
		switch (cKey)
		{
		case 'q':
		case 'Q':
			goto FINI;

		case 's':
		case 'S':
			char szFileName[16];
			sprintf(szFileName, "Cam%d.bmp", iLog);
			imwrite(szFileName, mFrame);
			cout << "Save cam pic: " << szFileName << endl;
			iLog++;

		default:
			goto LOOP;
		}

		LOOP:
		
		Point tArmor;    
		cArmor.FindArmorBase(mFrame, tArmor);
		
		Mat mArmor;
		mFrame.copyTo(mArmor);
		circle(mArmor, tArmor, 4, Scalar(0, 0, 255), 3);
		imshow("Armor", mArmor);

		//cout << PRE("Use time: ") << Timer() - dTimeStart << endl;
		cout << PRE("FPS: ") << 1000 / (Timer() - dTimeStart) << endl;
    }

	FINI:

	cCaptrue.release();
	
	return 0;
}

