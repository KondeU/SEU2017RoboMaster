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

#include "Apriltag.h"
#include "Armor.h"

double Timer()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return ((double(tv.tv_sec) * 1000) + (double(tv.tv_usec) / 1000));
}

int main()
{
	char szCameraVideo[16];
	sprintf(szCameraVideo, "/dev/video%d", 0); // Device ID is 0
	
	int fdDevice = v4l2_open(szCameraVideo, O_RDWR | O_NONBLOCK);
	if (fdDevice == -1)
	{
		cout << "Camera V4L2 device open failed." << endl;
	}
	
	int iExposure = 1000; // Exposure
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

	VideoCapture cCaptrue(0);
	
	if (!cCaptrue.isOpened())
    {
		cout << "Fail to open camera!" << endl;
        exit(1);
    }
    
    cCaptrue.set(CV_CAP_PROP_FRAME_WIDTH,  640);
 	cCaptrue.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
 	
	CApriltag cTag("tag16h5", 320, 240, 462.2708 / 2, 460.3916 / 2, 310.8862 / 2, 221.2809 / 2, 0.167);
	CArmor cArmor;
	
	Mat mFrame, mImg, mImgGray, mImgGrayPyrd;
	
	while (true)
	{
		double dTimerStart = Timer();
	
		if (!(cCaptrue.read(mFrame)))
		{
			cout << "Read camera data failed!" << endl;
			break;
		}
		
		//imshow("Frame", mFrame);
		
		pyrDown(mFrame, mImg);
		//imshow("Image", mImg);
		
		cvtColor(mImg, mImgGray, CV_BGR2GRAY);
		//imshow("Gray", mImgGray);
		
		pyrDown(mImgGray, mImgGrayPyrd);
		
		vector<TTagInfo> vTagInfo;
		cTag.Detect(mImgGrayPyrd, vTagInfo);
		Mat mTag = mImgGrayPyrd;
		
		/*
		struct TTagInfo
		{
			int iTagID;

			double dDistance;
			double dX, dY, dZ;
			double dYaw, dPitch, dRoll;

			struct TPoint
			{
				float fX, fY;
			} tCenter, tCorner[4];
		};
		*/

		cout << "Tag found: " << vTagInfo.size() << endl;
		for (size_t i = 0; i < vTagInfo.size(); i++)
		{
			cout << " " << "ID:" << vTagInfo[i].iTagID << endl;
			
			cout << " " << "Dis:" << vTagInfo[i].dDistance << "m" << endl;
			cout << " " << "X:"  << vTagInfo[i].dX
						<< ",Y:" << vTagInfo[i].dY
						<< ",Z:" << vTagInfo[i].dZ
						<< endl;
			
			cout << " " << "Yaw:"    << vTagInfo[i].dYaw
						<< ",Pitch:" << vTagInfo[i].dPitch
						<< ",Roll:"  << vTagInfo[i].dRoll
						<< endl;
			
			circle(mTag, cv::Point2f(vTagInfo[i].tCenter.fX, vTagInfo[i].tCenter.fY), 4, cv::Scalar(255, 0, 0), 2);
			
			line(mTag, Point2f(vTagInfo[i].tCorner[0].fX, vTagInfo[i].tCorner[0].fY),
				Point2f(vTagInfo[i].tCorner[1].fX, vTagInfo[i].tCorner[1].fY), Scalar(0,0,255));
			line(mTag, Point2f(vTagInfo[i].tCorner[1].fX, vTagInfo[i].tCorner[1].fY),
				Point2f(vTagInfo[i].tCorner[2].fX, vTagInfo[i].tCorner[2].fY), Scalar(0,0,255));
			line(mTag, Point2f(vTagInfo[i].tCorner[2].fX, vTagInfo[i].tCorner[2].fY),
				Point2f(vTagInfo[i].tCorner[3].fX, vTagInfo[i].tCorner[3].fY), Scalar(0,255,0));
			line(mTag, Point2f(vTagInfo[i].tCorner[3].fX, vTagInfo[i].tCorner[3].fY),
				Point2f(vTagInfo[i].tCorner[0].fX, vTagInfo[i].tCorner[0].fY), Scalar(0,0,255));
		}
		imshow("Tag", mTag);
		
		Point tArmor;
		cArmor.FindArmor(mFrame, tArmor);
		
		Mat mArmor;
		mFrame.copyTo(mArmor);
		circle(mArmor, tArmor, 4, Scalar(0, 0, 255), 3);
		cout << "Armor detect(x, y): " << tArmor.x << "," << tArmor.y << endl;
		imshow("Armor", mArmor);
		
		if (waitKey(1) > 0)
		{
			cout << "Exit program normally." << endl;
			break;
		}
		
		//cout << "Use time: " << Timer() - dTimerStart << endl;
		cout << "FPS: " << 1000 / (Timer() - dTimerStart) << endl;
		cout << endl;
	}
	
	return 0;
	
}
