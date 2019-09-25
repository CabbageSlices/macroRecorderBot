#include <Windows.h>
#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <thread>
#include <gdiplus.h>
#include "ImageFunctions.hpp"
#include "MacroRecorder.hpp"
#include "ObjectLocator.hpp"
#include <memory>

using namespace std;
using namespace cv;
using namespace Gdiplus;

sf::Clock globalTimer;
MacroRecorder recorder(&globalTimer);
bool isProgramRunning = true;

void processKeyboardInput(sf::Time time, RAWKEYBOARD rawInput) {

    Input::Type type = rawInput.Flags & RI_KEY_BREAK ? Input::ButtonUp : Input::ButtonDown;

    bool isExtendedKey = (rawInput.Flags & RI_KEY_E0);


    shared_ptr<Input> input = Input::CreateKeyboardInput(rawInput.MakeCode, type, time, isExtendedKey);
    recorder.processInput(input);
}

void processMouseInput(sf::Time time, WPARAM wparam, MSLLHOOKSTRUCT rawInput) {

    Input::MouseButtonId id;
    Input::Type type;

    switch(wparam) {
        case WM_LBUTTONDOWN: {
            id = Input::Left;
            type = Input::ButtonDown;
            break;
        }

        case WM_LBUTTONUP: {
            id = Input::Left;
            type = Input::ButtonUp;
            break;
        }

        case WM_RBUTTONDOWN: {
            id = Input::Right;
            type = Input::ButtonDown;
            break;
        }

        case WM_RBUTTONUP: {
            id = Input::Right;
            type = Input::ButtonUp;
            break;
        }

        default:
            return;
    }

    sf::Vector2u mousePos(rawInput.pt.x, rawInput.pt.y);

    shared_ptr<Input> input = Input::CreateMouseInput(id, type, time, mousePos);
    
    recorder.processInput(input);
}

LRESULT CALLBACK lowLevelMouseProc(int nCode, WPARAM wparam, LPARAM lparam) {

    if(nCode < 0) {
        return CallNextHookEx(NULL, nCode, wparam, lparam);
    }

    sf::Time inputTime = globalTimer.getElapsedTime();
    auto inputData = (MSLLHOOKSTRUCT*)lparam;

    thread inputProcessor( processMouseInput, inputTime, wparam, *inputData);
    inputProcessor.detach();

    return CallNextHookEx(NULL, nCode, wparam, lparam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

bool bitmapToOpencvMat(Gdiplus::Bitmap &bitmap, Mat &mat) {
    return true;
}

//https://stackoverflow.com/questions/3291167/how-can-i-take-a-screenshot-in-a-windows-application
//https://stackoverflow.com/questions/24644709/c-hbitmap-bitmap-into-bmp-file
int main() {

    // bool takeScreenShot(bitmapDestination);

    // ScreenObjectFinder objectFinder{
    //     ("rune", runeImages),
    //     ("player", playerImage)
    // }

    // sf::Vector2f runePos = objectFinder.find("rune");

    // if(runePos.x < 0)
    //     cout << "no rune present" << endl;

    // Mat screenshot;
    // takeScreenshot(screenshot);

    // position = objectFinder.find(screenShot, rune);

    // if position is valid:
    //     playerPosition = objectFinder.find(screenshot, player)
        
    // runeIccccccccccccccccccccccccccccccmage;
    // position =     objectFinder.find(screenShot, rune);

    Mat img = imread("runedarkness3.jpg");
    Mat target = imread("runes/rune2.png", IMREAD_COLOR);

    using namespace cv;
    using namespace cv::xfeatures2d;
    int minHessian = 400;
    Ptr<SURF> detector = SURF::create(minHessian);

    //keypoints
    vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;
    detector->detectAndCompute(target, noArray(), keypoints1, descriptors1);
    detector->detectAndCompute(img, noArray(), keypoints2, descriptors2);


    //matching
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    vector< std::vector<DMatch> >matches;
    matcher->knnMatch(descriptors1, descriptors2, matches, 2);

    const float ratioThreshold = 0.7f;
    vector<DMatch> goodMatches;

    for(unsigned i = 0; i < matches.size(); ++i) {
        if(matches[i][0].distance / matches[i][1].distance < ratioThreshold)
            goodMatches.push_back(matches[i][0]);
    }

    //draw matches
    // Mat imgMatches;
    // drawMatches(target, keypoints1, img, keypoints2, goodMatches, imgMatches, Scalar::all(-1), Scalar::all(-1),
    // vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    // imshow("matches1", imgMatches);

    findObjectInImage(img, target);

    vector<string> imageNames = {"rune1.png", "rune2.png"};
    ObjectLocator locator("runes", imageNames);
    // sf::Vector2i runeLocation = locator.getObjectLocation(img);

    // runeNavigator pather("players/ark");
    // pather.navigateToRune(screenShot, runePos, onFinishCallback);
    // pather.stop();

    GdiplusStartupInput input;
    ULONG_PTR gdiplusToken;
    auto status = GdiplusStartup(&gdiplusToken, &input, NULL);

    if(status != Gdiplus::Ok) {
        cout << "FAILED" << endl;
        return -1;
    }

    
    sf::Clock clock;
    
    Mat screenshot;
    takeScreenshot(screenshot);




    cout << "SCREENSHOT TIME: " << clock.restart().asMilliseconds() << " MS" << endl;

    imshow("result", img);
    waitKey();
    // Mat fullimage = imread("test.bmp", IMREAD_COLOR);

    // cout << "IMAGE LOAD TIME:  " << clock.getElapsedTime().asMilliseconds() << " MS" << endl;

    // imshow("result", fullimage);
    // waitKey();
    // int ss = 0;

    GdiplusShutdown(gdiplusToken);
    return 0;
}

// int main() {

//     //create the window
//     WNDCLASS wc = {0};
//     const char* name = "TEST";

//     wc.lpfnWndProc = WndProc;
//     wc.hInstance = GetModuleHandleA(NULL);
//     wc.lpszClassName = name;

//     RegisterClass(&wc);

//     HWND hwnd = CreateWindowEx(0, wc.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);

//     //register raw input 
//     RAWINPUTDEVICE Rid;
//     Rid.usUsagePage = 0x01;
//     Rid.usUsage = 6;
//     Rid.hwndTarget = hwnd;
//     Rid.dwFlags = RIDEV_INPUTSINK;

//     if(RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE)) == FALSE) {
//         cout << "REgistration raw input device failed" << endl;
//         return 0;
//     }

//     //start receiving mouse input
//     HHOOK mouse = SetWindowsHookEx(WH_MOUSE_LL, &lowLevelMouseProc, NULL, 0);

//     if( !mouse) {
//         cout << "HOOK FAILED" << endl;

//         UnhookWindowsHookEx(mouse);
//     }

//     //window message loop
//     MSG msg;

//     while(GetMessage(&msg, NULL, 0, 0) != 0) {
//         TranslateMessage(&msg);
//         DispatchMessage(&msg);
//     }

//     isProgramRunning = false;

//     UnhookWindowsHookEx(mouse);

//     return 0;
// }

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch(message) {
        case WM_INPUT: 
        {
            sf::Time inputTime = globalTimer.getElapsedTime();

            UINT dataSize;
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER)); //get size of input buffer

            //buffer to hold the rawinput
            char *buffer = new char[dataSize];
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer, &dataSize, sizeof(RAWINPUTHEADER)); //get input data into the buffer

            RAWINPUT *input = (RAWINPUT*)buffer;
            
            if(input->header.dwType == RIM_TYPEKEYBOARD) {

                thread inputProcessor( processKeyboardInput, inputTime, input->data.keyboard);
                inputProcessor.detach();
            }


            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}


// Mat image, imageGrey, dst, cannyOutput;

// void cannyThreshold(int thing, void* thing2) {
//     int threshold = *(int*)thing2;

    
//     Canny(imageGrey, cannyOutput, threshold, threshold * 3, 3);
//     dst = Scalar::all(0);
//     imageGrey.copyTo(dst, cannyOutput);

//     imshow("test", dst);
// }


// int main2() {

    // HHOOK keyboard = SetWindowsHookEx(WH_KEYBOARD_LL, &lowLevelKeyboardProc, NULL, 0);
    // HHOOK mouse = SetWindowsHookEx(WH_MOUSE_LL, &lowLevelMouseProc, NULL, 0);


    // if(!keyboard || !mouse) {
    //     cout << "HOOK FAILED" << endl;

    // }

    // MSG msg;

    // Mat frame;

    // // while(GetMessage(&msg, NULL, 0, 0)) {

    // //     TranslateMessage(&msg);
    // //     DispatchMessage(&msg);
    // // }

    // int threshold = 6;
    // namedWindow("test", WINDOW_AUTOSIZE);
    // createTrackbar("min threshold:", "test", &threshold, 100, cannyThreshold, &threshold);

    // image = imread("arrows4.png", IMREAD_COLOR);

    // GaussianBlur(image, image, Size(5,5), 0, 0, BORDER_DEFAULT);
    // cvtColor(image, imageGrey, COLOR_BGR2GRAY);
    
    // // Laplacian(imageGrey, dst, CV_16S, 5, 1, 0, BORDER_DEFAULT);
    // // convertScaleAbs(dst, dst);
    // // imwrite("bw.png", image);


    // Mat rune, runemask, fullimage, result;
    // rune = imread("rune2.png", IMREAD_COLOR);
    // runemask = imread("runemask.png", IMREAD_COLOR);
    // fullimage = imread("runedarkness3.jpg", IMREAD_COLOR);

    // namedWindow("template match", WINDOW_AUTOSIZE);

    // int result_cols = fullimage.cols - rune.cols + 1;
    // int result_rows = fullimage.rows - rune.rows + 1;

    // result.create(result.rows, result.cols, CV_32FC1);

    // matchTemplate(fullimage, rune, result, TM_CCOEFF);

    // // normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

    

    // double minv, maxv;
    // Point minP, maxP, matchP;

    // minMaxLoc(result, &minv, &maxv, &minP, &maxP, Mat());
    // cout << minv;
    // matchP = maxP;

    // rectangle( fullimage, matchP, Point( matchP.x + rune.cols , matchP.y + rune.rows ), Scalar::all(0), 2, 8, 0 );

    // imshow("template match", result);
    // imshow("1", rune);
    // imshow("2", runemask);
    // imshow("3", fullimage);

    // waitKey();

    // UnhookWindowsHookEx(keyboard);
    // UnhookWindowsHookEx(mouse);
    
//     return 0;
// }