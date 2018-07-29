#include "Guidance.h"

Mat mDownGreyL(CGuidance::iHeight, CGuidance::iWidth, CV_8UC1);
//Mat mDownGreyL;
//Mat mDownGreyR;

ofstream fout("GuidOutput.log");

int main()
{
	CGuidance cGuid;

	VideoWriter pVideo("guidance.avi", CV_FOURCC('D', 'I', 'V', 'X'),
			10/*FPS set to 10*/, Size(CGuidance::iWidth, CGuidance::iHeight), false);

	namedWindow("Guidance");
	imshow("Guidance", mDownGreyL);

	cGuid.Init();

	double dLastTime = cGuid.Timer();
	double dCrntTime = cGuid.Timer();

	while (true)
	{
		char k = waitKey(1);
		
		//cout << k << endl;
		
		if('q' == k)
			break;
	
		cGuid.GetData();

		imshow("Guidance", mDownGreyL);
		pVideo << mDownGreyL;

		dCrntTime = cGuid.Timer();
		cout << "Times:" << dCrntTime - dLastTime << endl;
		fout << "Times:" << dCrntTime - dLastTime << endl;
		dLastTime = dCrntTime;
	}

	//delete pVideo;
	
	cGuid.Deinit();

	return 0;
}
