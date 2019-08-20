#pragma once
#include <Windows.h>
#include <thread>
#include <mutex>
#include <vector>
#include <SFML/System.hpp>
#include "Input.hpp"
#include <memory>

using std::vector;
using std::shared_ptr;

enum State {
    Idle,
    Recording,
    RecordingPaused,
    Playing,
    PlaybackPaused
};

class MacroRecorder {

public:

    MacroRecorder(sf::Clock *_globalClock): globalClock(_globalClock){}

    void processInput(shared_ptr<Input> input);

    void update();

    //if block is set to true then funcitons will lock a mutex while executing
    void startRecording(bool block = false);
    void play(bool block = false);

    //stops current action (playing/ recording)
    void stop(bool block= false);
    void clearMacro(bool block = false);

    void printKeys(bool block = false);

private:

    //after inputs are recorded, need to process them to make them cleaner
    //stuff like sorting by timestamp, removing repeated keydowns, etc
    void cleanupInputs();

    State currentState; 
    vector<shared_ptr<Input> > inputs;

    shared_ptr<Input> playKey = Input::CreateKeyboardInput(0x3E, Input::ButtonDown);
    shared_ptr<Input> recordKey = Input::CreateKeyboardInput(0x3C, Input::ButtonDown);
    shared_ptr<Input> stopKey = Input::CreateKeyboardInput(0x3D, Input::ButtonDown);

    //scan codes
    // KeyboardInput playKey = KeyboardInput(WM_KEYDOWN, 0x3B);
    // KeyboardInput recordKey = KeyboardInput(WM_KEYDOWN, 0x3C);
    // KeyboardInput stopKey = KeyboardInput(WM_KEYDOWN, 0x3D);

    //virtual key
    // KeyboardInput playKey = KeyboardInput(WM_KEYDOWN, 0x70);
    // KeyboardInput recordKey = KeyboardInput(WM_KEYDOWN, 0x71);
    // KeyboardInput stopKey = KeyboardInput(WM_KEYDOWN, 0x72);

    int currentInputIndex = 0;

    sf::Clock *globalClock;

    sf::Time startTime;

    std::mutex mutex;
};