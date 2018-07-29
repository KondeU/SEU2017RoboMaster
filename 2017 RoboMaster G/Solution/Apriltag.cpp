// Apriltag.cpp
// Version 2.0
// started by Deyou Kong, 2017-07-12
// Checked by Deyou Kong, 2017-07-13
// Updated by Deyou Kong, 2017-07-20

#include <vector>
#include <utility>
#include <cstring>
#include <sstream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

#include <eigen3/Eigen/Dense>
using namespace Eigen;

#include "apriltag.h"
#include "tag16h5.h"
#include "tag25h7.h"
#include "tag25h9.h"
#include "tag36h10.h"
#include "tag36h11.h"
#include "tag36artoolkit.h"

#include "Apriltag.h"


//==================================================================

// This part use "Apriltags C++" directly

#include <cmath>

#ifndef PI
const double PI = 3.14159265358979323846;
#endif
const double TWOPI = 2.0*PI;

/**
* Normalize angle to be within the interval [-pi,pi].
*/
inline double standardRad(double t) {
	if (t >= 0.) {
		t = fmod(t + PI, TWOPI) - PI;
	}
	else {
		t = fmod(t - PI, -TWOPI) + PI;
	}
	return t;
}

/**
* Convert rotation matrix to Euler angles
*/
void wRo_to_euler(const Eigen::Matrix3d& wRo, double& yaw, double& pitch, double& roll) {
	yaw = standardRad(atan2(wRo(1, 0), wRo(0, 0)));
	double c = cos(yaw);
	double s = sin(yaw);
	pitch = standardRad(atan2(-wRo(2, 0), wRo(0, 0)*c + wRo(1, 0)*s));
	roll = standardRad(atan2(wRo(0, 2)*s - wRo(1, 2)*c, -wRo(0, 1)*s + wRo(1, 1)*c));
}

//==================================================================

// This part adjusted by Deyou Kong, change from "Apriltags C++", 2017-07-17

//Eigen::Matrix4d TagDetection::getRelativeTransform(double tag_size, double fx, double fy, double px, double py) const {
Matrix4d getRelativeTransform(double tag_size, double fx, double fy, double px, double py, Point2f p[4])
{
  std::vector<cv::Point3f> objPts;
  std::vector<cv::Point2f> imgPts;
  double s = tag_size/2.;
  objPts.push_back(cv::Point3f(-s,-s, 0));
  objPts.push_back(cv::Point3f( s,-s, 0));
  objPts.push_back(cv::Point3f( s, s, 0));
  objPts.push_back(cv::Point3f(-s, s, 0));

  //std::pair<float, float> p1 = p[0];
  //std::pair<float, float> p2 = p[1];
  //std::pair<float, float> p3 = p[2];
  //std::pair<float, float> p4 = p[3];
  //imgPts.push_back(cv::Point2f(p1.first, p1.second));
  //imgPts.push_back(cv::Point2f(p2.first, p2.second));
  //imgPts.push_back(cv::Point2f(p3.first, p3.second));
  //imgPts.push_back(cv::Point2f(p4.first, p4.second));
  imgPts.push_back(p[0]);
  imgPts.push_back(p[1]);
  imgPts.push_back(p[2]);
  imgPts.push_back(p[3]);

  cv::Mat rvec, tvec;
  cv::Matx33f cameraMatrix(
                           fx, 0, px,
                           0, fy, py,
                           0,  0,  1);
  cv::Vec4f distParam(0,0,0,0); // all 0?
  cv::solvePnP(objPts, imgPts, cameraMatrix, distParam, rvec, tvec);
  cv::Matx33d r;
  cv::Rodrigues(rvec, r);
  Eigen::Matrix3d wRo;
  wRo << r(0,0), r(0,1), r(0,2), r(1,0), r(1,1), r(1,2), r(2,0), r(2,1), r(2,2);

  Eigen::Matrix4d T; 
  T.topLeftCorner(3,3) = wRo;
  T.col(3).head(3) << tvec.at<double>(0), tvec.at<double>(1), tvec.at<double>(2);
  T.row(3) << 0,0,0,1;

  return T;
}

//void TagDetection::getRelativeTranslationRotation(double tag_size, double fx, double fy, double px, double py,
//                                                  Eigen::Vector3d& trans, Eigen::Matrix3d& rot) const {
void getRelativeTranslationRotation(double tag_size, double fx, double fy, double px, double py,
                                    Vector3d & trans, Matrix3d & rot, Point2f p[4])
{
  Eigen::Matrix4d T =
    //getRelativeTransform(tag_size, fx, fy, px, py);
    getRelativeTransform(tag_size, fx, fy, px, py, p);

  // converting from camera frame (z forward, x right, y down) to
  // object frame (x forward, y left, z up)
  Eigen::Matrix4d M;
  M <<
    0,  0, 1, 0,
    -1, 0, 0, 0,
    0, -1, 0, 0,
    0,  0, 0, 1;
  Eigen::Matrix4d MT = M*T;
  // translation vector from camera to the April tag
  trans = MT.col(3).head(3);
  // orientation of April tag with respect to camera: the camera
  // convention makes more sense here, because yaw,pitch,roll then
  // naturally agree with the orientation of the object
  rot = T.block(0,0,3,3);
}

/* This part use OpenCV instead of Eigen, but note that these codes are not test.
   Writed by Tingkuan Pei, 2016
   Fork from Tingkuan Pei, mark it, Deyou Kong, 2017-07-13

//Eigen::Matrix4d TagDetection::getRelativeTransform(double tag_size, double fx, double fy, double px, double py) const {
Mat getRelativeTransform(double tag_size, double fx, double fy, double px, double py, Point2f p[4])
{
  std::vector<cv::Point3f> objPts;
  std::vector<cv::Point2f> imgPts;
  double s = tag_size/2.;
  objPts.push_back(cv::Point3f(-s,-s, 0));
  objPts.push_back(cv::Point3f( s,-s, 0));
  objPts.push_back(cv::Point3f( s, s, 0));
  objPts.push_back(cv::Point3f(-s, s, 0));

  //std::pair<float, float> p1 = p[0];
  //std::pair<float, float> p2 = p[1];
  //std::pair<float, float> p3 = p[2];
  //std::pair<float, float> p4 = p[3];
  //imgPts.push_back(cv::Point2f(p1.first, p1.second));
  //imgPts.push_back(cv::Point2f(p2.first, p2.second));
  //imgPts.push_back(cv::Point2f(p3.first, p3.second));
  //imgPts.push_back(cv::Point2f(p4.first, p4.second));
  imgPts.push_back(p[0]);
  imgPts.push_back(p[1]);
  imgPts.push_back(p[2]);
  imgPts.push_back(p[3]);

  cv::Mat rvec, tvec;
  cv::Matx33f cameraMatrix(
                           fx, 0, px,
                           0, fy, py,
                           0,  0,  1);
  cv::Vec4f distParam(0,0,0,0); // all 0?
  cv::solvePnP(objPts, imgPts, cameraMatrix, distParam, rvec, tvec);
  cv::Matx33d r;
  cv::Rodrigues(rvec, r);
  //Eigen::Matrix3d wRo;
  //wRo << r(0,0), r(0,1), r(0,2), r(1,0), r(1,1), r(1,2), r(2,0), r(2,1), r(2,2);
  //
  //Eigen::Matrix4d T; 
  //T.topLeftCorner(3,3) = wRo;
  //T.col(3).head(3) << tvec.at<double>(0), tvec.at<double>(1), tvec.at<double>(2);
  //T.row(3) << 0,0,0,1;

  Mat T(4, 4, CV_32F, Scalar(0));
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
	  T.at<float>(i, j) = r(i, j); 
	}
  }

  T.at<float>(0, 3) = tvec.at<float>(0);
  T.at<float>(1, 3) = tvec.at<float>(1);
  T.at<float>(2, 3) = tvec.at<float>(2);
	
  T.at<float>(3, 0) = 0;
  T.at<float>(3, 1) = 0;
  T.at<float>(3, 2) = 0;
  T.at<float>(3, 3) = 1;

  return T;
}

//void TagDetection::getRelativeTranslationRotation(double tag_size, double fx, double fy, double px, double py,
//                                                  Eigen::Vector3d& trans, Eigen::Matrix3d& rot) const {
void getRelativeTranslationRotation(double tag_size, double fx, double fy, double px, double py,
                                    Mat & trans, Mat & rot, Point2f p[4])
{
  //Eigen::Matrix4d T =
  //  getRelativeTransform(tag_size, fx, fy, px, py);
  Mat T = getRelativeTransform(tag_size, fx, fy, px, py, p);

  // converting from camera frame (z forward, x right, y down) to
  // object frame (x forward, y left, z up)
  //Eigen::Matrix4d M;
  //M <<
  //  0,  0, 1, 0,
  //  -1, 0, 0, 0,
  //  0, -1, 0, 0,
  //  0,  0, 0, 1;
  Mat M(4, 4, CV_32F, Scalar(0));
  M.at<float>(0, 2) =  1;
  M.at<float>(1, 0) = -1;
  M.at<float>(2, 1) = -1;
  M.at<float>(3, 3) =  1;
  //Eigen::Matrix4d MT = M*T;
  Mat MT = M*T;
  // translation vector from camera to the April tag
  //trans = MT.col(3).head(3);
  trans.at<float>(0)=MT.at<float>(0,3);
  trans.at<float>(1)=MT.at<float>(1,3);
  trans.at<float>(2)=MT.at<float>(2,3);
  // orientation of April tag with respect to camera: the camera
  // convention makes more sense here, because yaw,pitch,roll then
  // naturally agree with the orientation of the object
  //rot = T.block(0,0,3,3);
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      rot.at<float>(i, j) = T.at<float>(i, j); 
    }
  }
}
*/

//==================================================================


CApriltag::CApriltag(string szTagFamily, int iWidth, int iHeight,
	double dFX, double dFY, double dPX, double dPY, double dTagSize)
{
	this->szTagFamily = szTagFamily;

	this->iWidth  = iWidth;
	this->iHeight = iHeight;

	this->dFX = dFX;
	this->dFY = dFY;
	this->dPX = dPX;
	this->dPY = dPY;

	this->dTagSize = dTagSize;
	
	if (szTagFamily == "tag16h5")
	{
		pTagFam = tag16h5_create();
	}
	else if (szTagFamily == "tag25h7")
	{
		pTagFam = tag25h7_create();
	}
	else if (szTagFamily == "tag25h9")
	{
		pTagFam = tag25h9_create();
	}
	else if (szTagFamily == "tag36h10")
	{
		pTagFam = tag36h10_create();
	}
	else if (szTagFamily == "tag36h11")
	{
		pTagFam = tag36h11_create();
	}
	else if (szTagFamily == "tag36artoolkit")
	{
		pTagFam = tag36artoolkit_create();
	}
	else
	{
		pTagFam = tag16h5_create();
	}	
	pTagFam->black_border = 1; // Set tag family border size
	
	pTagDet = apriltag_detector_create();
	apriltag_detector_add_family(pTagDet, pTagFam);
	pTagDet->quad_decimate = 1.0; // Decimate input image by this factor
	pTagDet->quad_sigma    = 0.0; // Apply low-pass blur to input
	pTagDet->nthreads      = 4;   // Use this many CPU threads
	pTagDet->debug         = 0;   // Enable debugging output (slow) if true
	pTagDet->refine_edges  = 1;   // Spend more time trying to align edges of tags if true
	pTagDet->refine_decode = 0;   // Spend more time trying to decode tags if true
	pTagDet->refine_pose   = 0;   // Spend more time trying to precisely localize tags if true
	
	// Fault tolerance factor
	
	pTagDet->qtp.max_line_fit_mse     = 1.0;
	pTagDet->qtp.min_white_black_diff = 15;
}

CApriltag::~CApriltag()
{
	apriltag_detector_destroy(pTagDet);

	if (szTagFamily == "tag16h5")
	{
		tag16h5_destroy(pTagFam);
	}
	else if (szTagFamily == "tag25h7")
	{
		tag25h7_destroy(pTagFam);
	}
	else if (szTagFamily == "tag25h9")
	{
		tag25h9_destroy(pTagFam);
	}
	else if (szTagFamily == "tag36h10")
	{
		tag36h10_destroy(pTagFam);
	}
	else if (szTagFamily == "tag36h11")
	{
		tag36h11_destroy(pTagFam);
	}
	else if (szTagFamily == "tag36artoolkit")
	{
		tag36artoolkit_destroy(pTagFam);
	}
	else
	{
		tag16h5_destroy(pTagFam);
	}
}

int CApriltag::Detect(cv::Mat mImg, vector<TTagInfo> & vTagInfo)
{
	vTagInfo.clear();


	//==================================================================
	
	// This part adjusted by Deyou Kong, change from "Apriltags C", 2017-07-17
	
	// Make an image_u8_t header for the Mat data
	//image_u8_t im = { .width = gray.cols,
	//	.height = gray.rows,
	//	.stride = gray.cols,
	//	.buf = gray.data
	//};
	image_u8_t im =
	{
		.width = mImg.cols,
		.height = mImg.rows,
		.stride = mImg.cols,
		.buf = mImg.data
	};

	//zarray_t *detections = apriltag_detector_detect(td, &im);
	zarray_t *detections = apriltag_detector_detect(pTagDet, &im);
	//cout << zarray_size(detections) << " tags detected" << endl;
	int iTagFound = zarray_size(detections);

	// Draw detection outlines
	//for (int i = 0; i < zarray_size(detections); i++) {
	//	apriltag_detection_t *det;
	//	zarray_get(detections, i, &det);
	//	line(frame, Point(det->p[0][0], det->p[0][1]),
	//				Point(det->p[1][0], det->p[1][1]),
	//				Scalar(0, 0xff, 0), 2);
	//	line(frame, Point(det->p[0][0], det->p[0][1]),
	//				Point(det->p[3][0], det->p[3][1]),
	//				Scalar(0, 0, 0xff), 2);
	//	line(frame, Point(det->p[1][0], det->p[1][1]),
	//				Point(det->p[2][0], det->p[2][1]),
	//				Scalar(0xff, 0, 0), 2);
	//	line(frame, Point(det->p[2][0], det->p[2][1]),
	//				Point(det->p[3][0], det->p[3][1]),
	//				Scalar(0xff, 0, 0), 2);
	//
	//	stringstream ss;
	//	ss << det->id;
	//	String text = ss.str();
	//	int fontface = FONT_HERSHEY_SCRIPT_SIMPLEX;
	//	double fontscale = 1.0;
	//	int baseline;
	//	Size textsize = getTextSize(text, fontface, fontscale, 2, &baseline);
	//	putText(frame, text, Point(det->c[0]-textsize.width/2, det->c[1]+textsize.height/2),
	//			fontface, fontscale, Scalar(0xff, 0x99, 0), 2);
	//}
	for (int i = 0; i < zarray_size(detections); i++)
	{
		apriltag_detection_t *det;
		zarray_get(detections, i, &det);
		
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
		}
		*/
		
		TTagInfo tTagInfo;
		Point2f tCorner[4];
		tTagInfo.iTagID = det->id;
		tTagInfo.tCenter.fX = static_cast<float>(det->c[0]);
		tTagInfo.tCenter.fY = static_cast<float>(det->c[1]);
		for (int i = 0; i < 4; i++)
		{
			tTagInfo.tCorner[i].fX = static_cast<float>(det->p[i][0]);
			tTagInfo.tCorner[i].fY = static_cast<float>(det->p[i][1]);
			tCorner[i] = Point2f(tTagInfo.tCorner[i].fX, tTagInfo.tCorner[i].fY);
		}
		
		Vector3d eTranslation;
		Matrix3d eRotation;
		getRelativeTranslationRotation(dTagSize, dFX, dFY, dPX, dPY,
			eTranslation, eRotation, tCorner);

		Matrix3d eF;
		eF <<
			1,  0, 0,
			0, -1, 0,
			0,  0, 1;
		Matrix3d eFixedRot = eF * eRotation;

		wRo_to_euler(eFixedRot, tTagInfo.dYaw, tTagInfo.dPitch, tTagInfo.dRoll);

		tTagInfo.dDistance = eTranslation.norm();

		tTagInfo.dX = eTranslation(0);
		tTagInfo.dY = eTranslation(1);
		tTagInfo.dZ = eTranslation(2);

		vTagInfo.push_back(tTagInfo);
	}
	zarray_destroy(detections);
	
	//imshow("Tag Detections", frame);
	return iTagFound;

	//==================================================================
}
