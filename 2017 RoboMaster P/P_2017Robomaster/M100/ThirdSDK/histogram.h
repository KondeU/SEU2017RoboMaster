#ifndef HISTOGRAM
#define HISTOGRAM
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace std;
class Histogram1D {

  private:

    int histSize[1];
	float hranges[2];
    const float* ranges[1];
    int channels[1];

  public:

	  Histogram1D();
	  void setChannel(int c);
	  int getChannel();
	  void setRange(float minValue, float maxValue);
	  float getMinValue();
	  float getMaxValue();
	  void setNBins(int nbins);
	  int getNBins();
	  cv::MatND getHistogram(const cv::Mat &image);
	  cv::Mat getHistogramImage(const cv::Mat &image);
	  cv::Mat equalize(const cv::Mat &image);
	  cv::Mat stretch(const cv::Mat &image, int minValue);
	  cv::Mat applyLookUp(const cv::Mat& image, const cv::MatND& lookup);

};
#endif
