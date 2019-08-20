#include <Windows.h>
#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <thread>
#include "MacroRecorder.hpp"
#include <memory>

using namespace std;
using namespace cv;

sf::Clock globalTimer;
MacroRecorder recorder(&globalTimer);
bool isProgramRunning = true;

void processKeyboardInput(sf::Time time, RAWKEYBOARD rawInput) {

    Input::Type type = rawInput.Flags & RI_KEY_BREAK ? Input::ButtonUp : Input::ButtonDown;

    // shared_ptr<Input> input = make_shared<KeyboardInput>(wparam, rawInput->vkCode);
    shared_ptr<Input> input = Input::CreateKeyboardInput(rawInput.MakeCode, type, time);
    // make_shared<KeyboardInput>(rawInput.MakeCode, type, time);
    //input->print();

    // KeyboardInput input(wparam, rawInput->vkCode);
    recorder.processInput(input);
}

// void processKeyboardInput2(sf::Time time, WPARAM wparam, KBDLLHOOKSTRUCT *rawInput) {

//     wparam = wparam == WM_SYSKEYDOWN ? WM_KEYDOWN : wparam;
//     wparam = wparam == WM_SYSKEYUP ? WM_KEYUP : wparam;

//     // shared_ptr<Input> input = make_shared<KeyboardInput>(wparam, rawInput->vkCode);
//     shared_ptr<KeyboardInput> input = make_shared<KeyboardInput>(wparam, rawInput->scanCode);
//     input->time = time;
//     //input->print();

//     // KeyboardInput input(wparam, rawInput->vkCode);
//     recorder.processInput(input);
// }

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

    // if(wparam == WM_MOUSEMOVE || wparam == WM_MOUSEWHEEL || wparam == WM_MOUSEWHEEL)
    //     return CallNextHookEx(NULL, nCode, wparam, lparam);

    thread inputProcessor( processMouseInput, inputTime, wparam, *inputData);
    inputProcessor.detach();

    return CallNextHookEx(NULL, nCode, wparam, lparam);
}

Mat image, imageGrey, dst, cannyOutput;

void cannyThreshold(int thing, void* thing2) {
    int threshold = *(int*)thing2;

    
    Canny(imageGrey, cannyOutput, threshold, threshold * 3, 3);
    dst = Scalar::all(0);
    imageGrey.copyTo(dst, cannyOutput);

    imshow("test", dst);
}

void recorderLoop() {

    while(isProgramRunning) {
        recorder.update();
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int main() {

    thread recorderThread(recorderLoop);

    WNDCLASS wc = {0};
    const char* name = "TEST";

    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = name;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);

    RAWINPUTDEVICE Rid;
    Rid.usUsagePage = 0x01;
    Rid.usUsage = 6;
    Rid.hwndTarget = hwnd;
    Rid.dwFlags = RIDEV_INPUTSINK;

    if(RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE)) == FALSE) {
        cout << "REgistration raw input device failed" << endl;
        return 0;
    }

    HHOOK mouse = SetWindowsHookEx(WH_MOUSE_LL, &lowLevelMouseProc, NULL, 0);

    if( !mouse) {
        cout << "HOOK FAILED" << endl;

        UnhookWindowsHookEx(mouse);
    }

    MSG msg;

    while(GetMessage(&msg, NULL, 0, 0) != 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    isProgramRunning = false;
    recorderThread.join();

    UnhookWindowsHookEx(mouse);

    return 0;
}

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

int main5() {

    HHOOK mouse = SetWindowsHookEx(WH_MOUSE_LL, &lowLevelMouseProc, NULL, 0);

    if( !mouse) {
        cout << "HOOK FAILED" << endl;

        UnhookWindowsHookEx(mouse);
    }

    MSG msg;

    while(GetMessage(&msg, NULL, 0, 0)) {

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(mouse);
}

int main3() {

    // sf::Window window(sf::VideoMode(800, 600), "OpenGL",
	// 	sf::Style::Default, sf::ContextSettings(24, 8, 4, 4, 5, sf::ContextSettings::Core));

    // thread hooks(manageHooks);


    // sf::Event event;

    // bool isOpen = true;
    
    // sf::Clock clock;
    // while(isOpen) {

    //     while(window.pollEvent(event)) {
    //         if(event.type == sf::Event::Closed)
    //             isOpen = false;
    //     }

    //     recorder.update();
        
    // }

    // window.close();

    // hooks.join();
}

int main2() {

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
    
    return 0;
}