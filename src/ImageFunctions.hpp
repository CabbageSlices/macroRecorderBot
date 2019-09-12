#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "SFML/System.hpp"
#include <iostream>

using namespace std;
using namespace cv;

//looks through imageToSearchIn and tries to find a match with imageToLookFor
//returns the position of the object within the image
//if successful the retruend vector will be a positive value indicating the position within the image where the 
//target was found
//if the function cannot find a match then the returned vector will be set to -1,-1
inline sf::Vector2i findObjectInImage(Mat &imageToSearchIn, Mat &imageToLookFor) {

    int resultRows = imageToSearchIn.rows - imageToLookFor.rows + 1;
    int resultCols = imageToSearchIn.cols - imageToLookFor.cols + 1;

    Mat result;
    result.create(resultRows, resultCols, CV_32FC1);

    matchTemplate(imageToSearchIn, imageToLookFor, result, TM_CCOEFF_NORMED);

    // normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

    double min;
    double max;
    Point positionMin;
    Point positionMax;

    minMaxLoc(result, &min, &max, &positionMin, &positionMax, Mat());

    Point matchPos = positionMax;

    //match threshold
    if(max > 0.35) {

        rectangle( imageToSearchIn, matchPos, Point( matchPos.x + imageToLookFor.cols , matchPos.y + imageToLookFor.rows ), Scalar::all(0), 2, 8, 0 );

        cout << max << endl;
        return sf::Vector2i(matchPos.x + imageToLookFor.cols / 2, matchPos.y + imageToLookFor.rows / 2);
    }
        //center point

    

    return sf::Vector2i(-1,-1);
}