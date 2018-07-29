#include <iostream>
#include <sys/time.h>
using namespace std;

#include<opencv2/opencv.hpp>
using namespace cv;

#define PRE(s) ("$ " s)

#define SHOW_USED_TIME false
#define SHOW_CRNT_FPS  false

double Timer()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return ((double(tv.tv_sec) * 1000) + (double(tv.tv_usec) / 1000));
}

int main()
{
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
			sprintf(szFileName, "./imgsave/camgrab%d.bmp", iLog);
			imwrite(szFileName, mFrame);
			cout << "Save cam pic: " << szFileName << endl;
			iLog++;

		default:
			goto LOOP;
		}

		LOOP:
		; // Must have a ";" because the mark "LOOP"

		#if SHOW_USED_TIME
		cout << PRE("Used time: ") << Timer() - dTimeStart << endl;
		#endif
		
		#if SHOW_CRNT_FPS
		cout << PRE("Current FPS: ") << 1000 / (Timer() - dTimeStart) << endl;
		#endif
    }

	FINI:

	cCaptrue.release();
	
	return 0;
}

