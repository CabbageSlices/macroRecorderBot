#pragma once

#include <Windows.h>
#include <SFML/System.hpp>
#include <memory>
#include <vector>
#include <string>
#include <iostream>

using std::ostream;
using std::string;
using std::shared_ptr;
using std::vector;

extern const sf::Vector2f screenToNormalizedMouseCoords;

class Input {

public:

    enum Type {
        ButtonDown = 0,
        ButtonUp,
        ButtonDownUp //button press should be sent in a single array in sendinput
    };

    enum MouseButtonId : unsigned int {
        Left = 1,
        Right = 2
    };

    static shared_ptr<Input> CreateKeyboardInput(unsigned int scanCode, Type inputType, sf::Time inputTime, bool _isExtendedKey = false);
    static shared_ptr<Input> CreateKeyboardInput(unsigned int scanCode, Type inputType, bool _isExtendedKey = false);
    static shared_ptr<Input> CreateMouseInput(MouseButtonId buttonId, Type inputType, sf::Time inputTime, sf::Vector2u _mousePos, bool normalizeCoordinates = true);

    static shared_ptr<Input> CreateInputFromDescriptionString(string description);

    static vector<INPUT> GenerateKeyboardInputStructure(Input &keyboardInput);
    static vector<INPUT> GenerateMouseInputStructure(Input &mouseInput);
    static vector<INPUT> GenerateInputStructure(Input &input);

    static INPUT GenerateKeyboardInputStructureSingle(Input &keyboardInput);
    static INPUT GenerateMouseInputStructureSingle(Input &mouseInput);

    const static string KeyboardFormatDescriptionString;
    const static string MouseFormatDescriptionString;

    vector<INPUT> generateRawInputStructure();
    void sendToSystem();
    void sendToSystemAsType(Type _type);

    //if the given input is the same button as this one, but the other one is a button up event, this returns true
    bool isSameButtonReleased(Input &other);

    //returns true if the device, button, and aciton is the same
    bool isSameInput(Input &other);
    void print();

    friend ostream& operator<<(ostream &os, const Input &input);

    Type getType() {
        return type;
    }

    void setType(Type _type) {
        type = _type;
    }

    unsigned int getButtonId() {
        return buttonId;
    }

    sf::Time time;

private:

    enum Device {
        Mouse = 0,
        Keyboard
    };

    Input();
    Input(unsigned int _buttonId, Type _type, sf::Time _time, sf::Vector2u _mousePos, Device _device, bool _isExtendedButton = false);

    Type type;

    //scan code for keyboard input, mouse button id for mouse input
    unsigned int buttonId;

    //if set to true then need to set KEYEVENTTF_EXTENDEDKEY in sendinput
    bool isExtendedButton;

    //position of the mouse, normalized coordiantes between 1-65555 as per the sendinput docs
    sf::Vector2u mousePos;

    Device device;

    //generates the rawinput object used by send input
    INPUT (*_generateRawInputStructureSingle)(Input &input);
};

inline bool compareInputPointers(shared_ptr<Input> i, shared_ptr<Input> j) {

    return (i->time < j->time);
}

inline ostream& operator<<(ostream& os, const Input &input) {
    
    if(input.device == Input::Keyboard)
        os << input.device << " " << input.type << " " << input.time.asMilliseconds() << " " << input.buttonId << " " << input.isExtendedButton;
    else
        os << input.device << " " << input.type << " " << input.time.asMilliseconds() << " " << input.buttonId << " " << input.mousePos.x << " " << input.mousePos.y;
}