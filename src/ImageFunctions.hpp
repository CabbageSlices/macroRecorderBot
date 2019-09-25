#pragma once

#include <Windows.h>
#include <gdiplus.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <algorithm>
#include "SFML/System.hpp"
#include <iostream>

using namespace std;
using namespace cv;
using namespace Gdiplus;

//TAKEN FROM
//https://docs.microsoft.com/en-ca/windows/win32/gdiplus/-gdiplus-retrieving-the-class-identifier-for-an-encoder-use
inline int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}


//https://docs.microsoft.com/en-us/windows/win32/gdi/capturing-an-image
//take screenshot of primary monitor
inline bool takeScreenshot(Mat &destination) {

   //get handle to monitor where screenshot is being taken
   HDC window = GetWindowDC(NULL);

   HDC compatible = CreateCompatibleDC(window);

   int width = GetDeviceCaps(window, HORZRES);
   int height = GetDeviceCaps(window, VERTRES);

   HBITMAP bitmap = CreateCompatibleBitmap(window, width, height);
   HBITMAP oldbitmap = (HBITMAP) SelectObject(compatible, bitmap);

   int x = GetSystemMetrics(SM_XVIRTUALSCREEN);//top left corner of main desktop
   int y = GetSystemMetrics(SM_YVIRTUALSCREEN);

   BitBlt(compatible, 0, 0, width, height, window, x, y, SRCCOPY);
   bitmap = (HBITMAP) SelectObject(compatible, oldbitmap);

   DeleteDC(compatible);
   DeleteDC(window);

   //convert to bmp to save it
   Gdiplus::Bitmap img(bitmap, NULL);
   CLSID id;
   GetEncoderClsid(L"image/bmp", &id);
   img.Save(L"test.bmp", &id, NULL);

   //load it in opencv mat
   destination = imread("test.bmp", IMREAD_COLOR);

   return true;
    
}

//looks through imageToSearchIn and tries to find a match with imageToLookFor
//returns the position of the object within the image
//if successful the retruend vector will be a positive value indicating the position within the image where the 
//target was found
//if the function cannot find a match then the returned vector will be set to -1,-1
inline sf::Vector2i findObjectInImage(Mat &imageToSearchIn, Mat &targetImage, bool drawDebugInfo = true) {

   // sf::Clock timerr;
   // using namespace cv::xfeatures2d;

   // static Ptr<SURF> detector = SURF::create(400);
   // static Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);

   // //keypoint and descriptors
   // std::vector<KeyPoint> keypointsTarget, keypointsImage;
   // Mat descriptorsTarget, descriptorsImage;

   // detector->detectAndCompute(targetImage, noArray(), keypointsTarget, descriptorsTarget);
   // detector->detectAndCompute(imageToSearchIn, noArray(), keypointsImage, descriptorsImage);

   // std::vector< std::vector<DMatch> > matches;
   // matcher->knnMatch(descriptorsTarget, descriptorsImage, matches, 2);

   // const float ratioThreshold = 0.7f;
   // vector<DMatch> goodMatches;

   // for(unsigned i = 0; i < matches.size(); ++i) {
   //    if(matches[i][0].distance / matches[i][1].distance < ratioThreshold)
   //       goodMatches.push_back(matches[i][0]);
   // }

   // cout << "search time:" << timerr.getElapsedTime().asMilliseconds() << endl;

   // //not enough matches, let's say nothing found
   // if(goodMatches.size() < 2)
   //    return sf::Vector2i(-1, -1);

   // //probably enough matches, compute position based on median
   // vector<int> xPositions, yPositions;
   // for(auto &match : goodMatches) {
   //    cv::Point2d point = keypointsImage[match.trainIdx].pt;
   //    xPositions.push_back(point.x);
   //    yPositions.push_back(point.y);  
   // }

   // std::sort(xPositions.begin(), xPositions.end());
   // std::sort(yPositions.begin(), yPositions.end());

   // int medX = xPositions[xPositions.size() / 2];
   // int medY = yPositions[yPositions.size() / 2];

   // if(!drawDebugInfo)
   //    return sf::Vector2i(medX, medY);

   // Mat imgMatches;
   // drawMatches(targetImage, keypointsTarget, imageToSearchIn, keypointsImage, goodMatches, imgMatches, Scalar::all(-1), Scalar::all(-1),
   // vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

   // imshow("matches2", imgMatches);

   // return sf::Vector2i(medX, medY);

   sf::Clock timer;
   int resultRows = imageToSearchIn.rows - targetImage.rows + 1;
   int resultCols = imageToSearchIn.cols - targetImage.cols + 1;

   Mat result;
   result.create(resultRows, resultCols, CV_32FC1);

   matchTemplate(imageToSearchIn, targetImage, result, TM_CCOEFF_NORMED);

   // normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

   double min;
   double max;
   cv::Point positionMin;
   cv::Point positionMax;

   minMaxLoc(result, &min, &max, &positionMin, &positionMax, Mat());

   cv::Point matchPos = positionMax;

   //match threshold
   if(max > 0.35) {

      rectangle( imageToSearchIn, matchPos, cv::Point( matchPos.x + targetImage.cols , matchPos.y + targetImage.rows ), Scalar::all(0), 2, 8, 0 );

      cout << max << endl;

      return sf::Vector2i(matchPos.x + targetImage.cols / 2, matchPos.y + targetImage.rows / 2);
   }
      //center point


      cout << timer.getElapsedTime().asMilliseconds() << endl;

   return sf::Vector2i(-1,-1);
}