#include "Input.hpp"
#include <string>
#include <iostream>
#include "StringMethods.h"

using std::cout;
using std::stoi;
using std::stoul;
using std::endl;
using std::string;
using std::make_shared;


const sf::Vector2f screenToNormalizedMouseCoords = sf::Vector2f(1.f/GetSystemMetrics(SM_CXSCREEN) * 65555.f, 1.f/GetSystemMetrics(SM_CYSCREEN) * 65555.f);
const string Input::KeyboardFormatDescriptionString = "format for keyboard input: deviceType inputType inputTime(milliseconds) scanCode isExtendedButton";
const string Input::MouseFormatDescriptionString = "format for mouse input: deviceType inputType inputType(milliseconds) buttonId MousePositionX(between 1-65555) MousePosiitonY(between 1-65555)";

vector<INPUT> Input::GenerateInputStructure(Input &input) {

    vector<INPUT> inputs;

    if(input.type == Input::ButtonDownUp) {
        Input downVersion = input;
        downVersion.type = ButtonDown;

        Input upVersion = input;
        upVersion.type = ButtonUp;

        inputs.push_back(input._generateRawInputStructureSingle(downVersion));
        inputs.push_back(input._generateRawInputStructureSingle(upVersion));

    } else {

        inputs.push_back(input._generateRawInputStructureSingle(input));
    }

    return inputs;
}

INPUT Input::GenerateKeyboardInputStructureSingle(Input &input) {

    INPUT rawInput;
    rawInput.type = INPUT_KEYBOARD;
    rawInput.ki.wVk = 0;
    rawInput.ki.wScan = input.buttonId;
    rawInput.ki.dwFlags =  KEYEVENTF_SCANCODE;

    if(input.type == Input::ButtonUp)
        rawInput.ki.dwFlags = rawInput.ki.dwFlags | KEYEVENTF_KEYUP;

    if(input.isExtendedButton)
        rawInput.ki.dwFlags = rawInput.ki.dwFlags | KEYEVENTF_EXTENDEDKEY;

    return rawInput;
}


INPUT Input::GenerateMouseInputStructureSingle(Input &input) {

    INPUT rawInput = {0};
    rawInput.type = INPUT_MOUSE;
    rawInput.mi.dx = input.mousePos.x;
    rawInput.mi.dy = input.mousePos.y;

    DWORD buttonFlag = 0;

    switch(input.type) {
        case ButtonDown:
            buttonFlag = (input.buttonId == Input::Left ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN);
            break;
        case ButtonUp:
            buttonFlag = (input.buttonId == Input::Left ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP);
            break;
        default:
            buttonFlag = 0;
            break;
    }

    
    rawInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | buttonFlag;

    return rawInput;
}

shared_ptr<Input> Input::CreateKeyboardInput(unsigned int scanCode, Type inputType, sf::Time inputTime, bool _isExtendedKey) {

    shared_ptr<Input> input(new Input(scanCode, inputType, inputTime, sf::Vector2u(0,0), Keyboard, _isExtendedKey));
    input->_generateRawInputStructureSingle = Input::GenerateKeyboardInputStructureSingle;

    return input;
}

shared_ptr<Input> Input::CreateKeyboardInput(unsigned int scanCode, Type inputType, bool _isExtendedKey) {

    shared_ptr<Input> input(new Input(scanCode, inputType, sf::Time(), sf::Vector2u(0,0), Keyboard, _isExtendedKey));
    input->_generateRawInputStructureSingle = Input::GenerateKeyboardInputStructureSingle;

    return input;
}

shared_ptr<Input> Input::CreateMouseInput(MouseButtonId buttonId, Type inputType, sf::Time inputTime, sf::Vector2u mousePos, bool normalizeCoordinates) {

    sf::Vector2u normalizedPos = mousePos;
    
    if(normalizeCoordinates) {
        normalizedPos = sf::Vector2u(mousePos.x * screenToNormalizedMouseCoords.x, mousePos.y * screenToNormalizedMouseCoords.y);
    }
    shared_ptr<Input> input(new Input((unsigned int) buttonId, inputType, inputTime, normalizedPos, Mouse));
    input->_generateRawInputStructureSingle = Input::GenerateMouseInputStructureSingle;

    return input;
}

shared_ptr<Input> Input::CreateInputFromDescriptionString(string description) {

    vector<string> items;
    separateIntoWords(description, items, ' ');
    
    Device device = (Device)stoi(items[0]);

    if(device == Keyboard) {
        Type type = (Type)stoi(items[1]);
        sf::Time inputTime = sf::milliseconds(stoi(items[2]));
        unsigned int scanCode = stoul(items[3]);
        bool isExtendedButton = (bool)stoi(items[4]);

        return CreateKeyboardInput(scanCode, type, inputTime, isExtendedButton);
    } else {

        Type type = (Type)stoi(items[1]);
        int time = stoi(items[2]);
        sf::Time inputTime = sf::milliseconds(time);
        MouseButtonId buttonId = (MouseButtonId)stoul(items[3]);
        unsigned int mouseX = stoul(items[4]);
        unsigned int mouseY = stoul(items[5]);

        return CreateMouseInput(buttonId, type, inputTime, sf::Vector2u(mouseX, mouseY), false);
    }
}

vector<INPUT> Input::generateRawInputStructure() {

    return GenerateInputStructure(*this);
}

void Input::sendToSystem() {

    vector<INPUT> inputs = generateRawInputStructure();
    SendInput(inputs.size(), inputs.data(), sizeof(INPUT));
}

void Input::sendToSystemAsType(Type _type) {
    Type oldType = type;
    type = _type;
    print();
    sendToSystem();
    type = oldType;
}

void Input::print() {
    string deviceName = (device == Keyboard ? "Keybord" : "Mouse");
    string action = "pressed down";

    if(type == ButtonUp)
        action = "released";
    else if(type == ButtonDownUp)
        action = "DOWN AND UP";

    string button;

    if(device == Keyboard) {

        auto vk = MapVirtualKeyA(buttonId, MAPVK_VSC_TO_VK);
        char cha = MapVirtualKeyA(vk, MAPVK_VK_TO_CHAR);
        button = string(1, cha);
    }

    if(device == Mouse)
        button = (buttonId == Left ? "Button Left" : "Button Right");
    
    cout << deviceName << " " << button << " has been " << action << " at " << time.asMilliseconds() << endl;

}

Input::Input(unsigned int _buttonId, Type _type, sf::Time _time, sf::Vector2u _mousePos, Device _device, bool _isExtendedButton) :
    type(_type),
    buttonId(_buttonId),
    time(_time),
    mousePos(_mousePos),
    device(_device),
    isExtendedButton(_isExtendedButton)
    {

    }

bool Input::isSameButtonReleased(Input &other) {

    if(device != other.device)
        return false;

    if(buttonId != other.buttonId)
        return false;

    return other.type == ButtonUp;
}

bool Input::isSameInput(Input &other) {

    if(device != other.device)
        return false;

    if(buttonId != other.buttonId)
        return false;

    return type == other.type;
}