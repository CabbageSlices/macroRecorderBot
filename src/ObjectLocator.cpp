#include "ObjectLocator.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "ImageFunctions.hpp"

using std::vector;
using std::string;
using std::shared_ptr;
using namespace cv;

ObjectLocator::ObjectLocator(std::string objectImagesPath, vector<string> &imageNames) {
    loadImages(objectImagesPath, imageNames);   
}

void ObjectLocator::loadTargetImages(std::string objectImagePath, std::vector<std::string> &imageNames) {
    loadImages(objectImagePath, imageNames);
}

sf::Vector2i ObjectLocator::getObjectLocation(cv::Mat &screenshot) {

    for(auto &image : objectImages) {

        sf::Vector2i pos = findObjectInImage(screenshot, *image);

        if(pos.x >= 0 && pos.y >= 0)
            return pos;
    }

    return sf::Vector2i(-1,-1);
}

void ObjectLocator::loadImages(std::string objectImagesPath, vector<string> &imageNames) {
    
    for(string imageName : imageNames) {
        string fullFileName = objectImagesPath + "/" + imageName;

        shared_ptr<cv::Mat> img(new cv::Mat());
        *img = imread(fullFileName, IMREAD_COLOR);

        //file didn't load, ignore
        if(img->rows == 0 || img->cols == 0)
            continue;

        objectImages.push_back(img);
    }
}