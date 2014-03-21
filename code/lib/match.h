#include <stdio.h>
#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/features2d.hpp"

using namespace std;
using namespace cv;

class Match
{
    private:
    Point LastMatch;
    public:
    Point SimpleMatch(Mat& sample, Mat& frame);
    Point SimpleMatch2(Mat& sample, Mat& frame);
    Point SurfMatch(Mat& sample, Mat& frame);
};

Point Match::SimpleMatch(Mat& sample, Mat& frame)
{
    Mat result;
    int match_method=1; //1-5

    matchTemplate( sample, frame, result, match_method );
    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    /// Localizing the best match with minMaxLoc
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    Point matchLoc;
    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );


    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED )
        { matchLoc = minLoc; }
    else
        { matchLoc = maxLoc; }

    return  matchLoc;
}

// http://stackoverflow.com/revisions/20180073/3
// alguns bugs
Point Match::SimpleMatch2(Mat& sample, Mat& frame)
{
    Mat frame_copy = frame.clone();
    flip( frame_copy, frame_copy, 1 );
    GaussianBlur( frame_copy, frame_copy, Size(7,7), 3.0 );

    Mat result;
    matchTemplate( frame_copy, sample, result, CV_TM_SQDIFF_NORMED );
    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    Point match;
    double minVal, maxVal;
    Point  minLoc, maxLoc;

    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
    match = minLoc;

    return match;
}

// http://docs.opencv.org/doc/tutorials/features2d/feature_homography/feature_homography.html
// SURF detector + descriptor + FLANN Matcher + FindHomography
Point Match::SurfMatch(Mat& sample, Mat& frame)
{
    //-- Step 1: Detect the keypoints using SURF Detector
  int minHessian = 15;

  SurfFeatureDetector detector( minHessian );

  std::vector<KeyPoint> keypoints_object, keypoints_scene;

  detector.detect( sample, keypoints_object );
  detector.detect( frame, keypoints_scene );

  //-- Step 2: Calculate descriptors (feature vectors)
  SurfDescriptorExtractor extractor;

  Mat descriptors_object, descriptors_scene;

  extractor.compute( sample, keypoints_object, descriptors_object );
  extractor.compute( frame, keypoints_scene, descriptors_scene );

  //-- Step 3: Matching descriptor vectors using FLANN matcher
  FlannBasedMatcher matcher;
  std::vector< DMatch > matches;
  matcher.match( descriptors_object, descriptors_scene, matches );

  double max_dist = 0; double min_dist = 15;

  //-- Quick calculation of max and min distances between keypoints
  for( int i = 0; i < descriptors_object.rows; i++ )
  { double dist = matches[i].distance;
    if( dist < min_dist ) min_dist = dist;
    if( dist > max_dist ) max_dist = dist;
  }

  printf("-- Max dist : %f \n", max_dist );
  printf("-- Min dist : %f \n", min_dist );

  //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
  std::vector< DMatch > good_matches;

  for( int i = 0; i < descriptors_object.rows; i++ )
  { if( matches[i].distance < 3*min_dist )
    { good_matches.push_back( matches[i]); }
  }

  Mat img_matches;
  drawMatches( sample, keypoints_object, frame, keypoints_scene,
               good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
               vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );


  //-- Localize the object from img_1 in img_2
  std::vector<Point2f> obj;
  std::vector<Point2f> scene;

  for( size_t i = 0; i < good_matches.size(); i++ )
  {
    //-- Get the keypoints from the good matches
    obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
    scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
  }

  Point match;

  if(good_matches.size() >= 4)
  {
      Mat H = findHomography( obj, scene, RANSAC );

      //-- Get the corners from the image_1 ( the object to be "detected" )
      std::vector<Point2f> obj_corners(4); 
      obj_corners[0] = Point(0,0); obj_corners[1] = Point( sample.cols, 0 );
      obj_corners[2] = Point( sample.cols, sample.rows ); obj_corners[3] = Point( 0, sample.rows );
      std::vector<Point2f> scene_corners(4);

      perspectiveTransform( obj_corners, scene_corners, H);


      //-- Draw lines between the corners (the mapped object in the scene - image_2 )
      Point2f offset( (float)sample.cols, 0);
      line( img_matches, scene_corners[0] + offset, scene_corners[1] + offset, Scalar(0, 255, 0), 4 );
      line( img_matches, scene_corners[1] + offset, scene_corners[2] + offset, Scalar( 0, 255, 0), 4 );
      line( img_matches, scene_corners[2] + offset, scene_corners[3] + offset, Scalar( 0, 255, 0), 4 );
      line( img_matches, scene_corners[3] + offset, scene_corners[0] + offset, Scalar( 0, 255, 0), 4 );

      //-- Show detected matches
      imshow( "Good Matches & Object detection", img_matches );

      LastMatch.x=match.x=(scene_corners[0].x+scene_corners[1].x+scene_corners[2].x+scene_corners[3].x)/4 + offset.x;
      LastMatch.y=match.y=(scene_corners[0].y+scene_corners[1].y+scene_corners[2].y+scene_corners[3].y)/4 + offset.y;
  }
  else
  {
    if(LastMatch.x==0 && LastMatch.y==0)
    {
        match.x=sample.cols/2;
        match.y=sample.rows/2;
    }
        match=LastMatch;
  }
  return match;
}