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
#include "ObjectLocator.hpp"

using std::vector;
using std::string;

class RuneNavigator {

public:

    RuneNavigator(const string &pathToRuneImages, const string &pathToPlayerImages);

    void navigateToRune(cv::Mat &screenshot);

private:

    static vector<string> playerImageNames;

    ObjectLocator runeFinder;
    ObjectLocator playerFinder;
};