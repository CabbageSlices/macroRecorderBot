#pragma once

#include <Windows.h>
#include <SFML/System.hpp>
#include <memory>
#include <vector>

using std::shared_ptr;
using std::vector;

extern const sf::Vector2f screenToNormalizedMouseCoords;

class Input {

public:

    enum Type {
        ButtonDown,
        ButtonUp,
        ButtonDownUp //button press should be sent in a single array in sendinput
    };

    enum MouseButtonId : unsigned int {
        Left = 1,
        Right = 2
    };

    static shared_ptr<Input> CreateKeyboardInput(unsigned int scanCode, Type inputType, sf::Time inputTime);
    static shared_ptr<Input> CreateKeyboardInput(unsigned int scanCode, Type inputType);
    static shared_ptr<Input> CreateMouseInput(MouseButtonId buttonId, Type inputType, sf::Time inputTime, sf::Vector2u _mouseScreenPos);

    static vector<INPUT> GenerateKeyboardInputStructure(Input &keyboardInput);
    static vector<INPUT> GenerateMouseInputStructure(Input &mouseInput);
    static vector<INPUT> GenerateInputStructure(Input &input);


    static INPUT GenerateKeyboardInputStructureSingle(Input &keyboardInput);
    static INPUT GenerateMouseInputStructureSingle(Input &mouseInput);

    vector<INPUT> generateRawInputStructure();
    void sendToSystem();

    bool isSameButtonReleased(Input &other);
    bool isSameInput(Input &other);
    void print();

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
        Mouse,
        Keyboard
    };

    Input();
    Input(unsigned int _buttonId, Type _type, sf::Time _time, sf::Vector2u _mousePos, Device _device);

    Type type;

    //scan code for keyboard input, mouse button id for mouse input
    unsigned int buttonId;

    //position of the mouse, normalized coordiantes between 1-65555 as per the sendinput docs
    sf::Vector2u mousePos;

    Device device;

    //generates the rawinput object used by send input
    INPUT (*_generateRawInputStructureSingle)(Input &input);
};

inline bool compareInputPointers(shared_ptr<Input> i, shared_ptr<Input> j) {

    return (i->time < j->time);
}