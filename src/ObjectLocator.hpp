#pragma once

#include <Windows.h>
#include <gdiplus.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include "SFML/System.hpp"

class ObjectLocator {

public:


    ObjectLocator(){}
    ObjectLocator(std::string objectImagePath, std::vector<std::string> &imageNames);

    void loadTargetImages(std::string objectImagePath, std::vector<std::string> &imageNames);

    //finds the position of a object, if there is any, and returns its position within the image
    //returns -1,-1 if there is no object
    //only positive positions since images only have positive pixel positions
    sf::Vector2i getObjectLocation(cv::Mat &screenshot);

private:

    void loadImages(std::string objectImagePath, std::vector<std::string> &imageNames);

    //image of all objects
    std::vector<std::shared_ptr<cv::Mat> > objectImages;
};