#include "featuresdetect.h"

FeaturesDetect::FeaturesDetect(QObject *parent) :
  QObject(parent),
  maxNum(200)
{
}

void FeaturesDetect::setMaxNum(int num)
{
  this->maxNum = num;
}

void FeaturesDetect::mainloop(cv::Mat &frame)
{

  cv::cvtColor(frame,gray,CV_RGB2GRAY);
  //查找特征点
  if(grayPrevious.empty())//第一帧
    {
      gray.copyTo(grayFirst);
      gray.copyTo(grayPrevious);
      cv::goodFeaturesToTrack(gray,
                            pointsToTrack[0],
                            maxNum,
                            0.01,
                            100.0);
    }
  else
    {
        // if not enough points, add new points to track
        if(pointsToTrack[0].size()<80){
            pointsToTrack[1].clear();
            cv::goodFeaturesToTrack(gray,               // the image
                                    pointsToTrack[1],   // the output detected features
                                    maxNum,             // the maximum number of features
                                    0.01,               // quality level
                                    10.0);              // min distance between two features
            pointsToTrack[0].insert(pointsToTrack[0].end(),pointsToTrack[1].begin(),pointsToTrack[1].end());
            pointsToTrack[1].clear();
          }

        // tracking process

	cv::calcOpticalFlowPyrLK(
		grayFirst, gray, // 2 consecutive images
		pointsToTrack[0], // input point positions in first image
		pointsToTrack[1], // output point positions in the 2nd image
		status, // tracking success
		err); // tracking error

	gray.copyTo(grayPrevious);

//	for(uint i= 0; i < pointsToTrack[1].size(); i++ ) {
//		//double motion = sqrt(pow(pointsToTrack[0][i].x-pointsToTrack[1][i].x,2)+pow(pointsToTrack[0][i].y-pointsToTrack[1][i].y,2));

//		// do we keep this point?
//		if (status[i] && motion < 20) {
//			// keep this point in vector
//			initialPoints.push_back(pointsToTrack[0][i]);
//			trackedPoints.push_back(pointsToTrack[1][i]);
//		}
//	}

//        pointsToTrack[0] = trackedPoints;
    }

    // for all tracked points
    for(uint i= 0; i < pointsToTrack[1].size(); i++ ) {
        // draw line and circle
//        cv::line(frame,
//                 pointsToTrack[0][i], // initial position
//                 pointsToTrack[1][i], // new position
//                 cv::Scalar(0,255,0));
        cv::circle(frame,pointsToTrack[1][i],3,cv::Scalar(0,255,0),2,8,0);

      }
}
