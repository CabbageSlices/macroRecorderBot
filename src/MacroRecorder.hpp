#pragma once
#include <Windows.h>
#include <thread>
#include <mutex>
#include <vector>
#include <SFML/System.hpp>
#include "Input.hpp"
#include <memory>
#include "Macro.hpp"

using std::vector;
using std::shared_ptr;

enum State {
    Idle,
    Recording,
    RecordingPaused,
    Playing,
    PlaybackPaused,
    Saving,
    Loading,
    Paused,
};

class MacroRecorder {

public:

    MacroRecorder(sf::Clock *_globalClock);

    void processInput(shared_ptr<Input> input);

    //to be used in it's own thread
    void update();

    //if block is set to true then funcitons will lock a mutex while executing
    void startRecording();
    void play();

    //stops current action (playing/ recording)
    void stop();
    void pause();
    void unpause();
    void clearMacro();

    void printKeys();

    void save();
    void load();

private:

    //after inputs are recorded, need to process them to make them cleaner
    //stuff like sorting by timestamp, removing repeated keydowns, etc
    void cleanupInputs();

    State currentState; 
    Macro currentMacro;

    vector<shared_ptr<Input> > inputs;

    //scancodes
    shared_ptr<Input> playKey = Input::CreateKeyboardInput(0x3E, Input::ButtonDown);//f4
    shared_ptr<Input> recordKey = Input::CreateKeyboardInput(0x3C, Input::ButtonDown);//f2
    shared_ptr<Input> stopKey = Input::CreateKeyboardInput(0x3D, Input::ButtonDown);//f3
    shared_ptr<Input> pauseKey = Input::CreateKeyboardInput(0x40, Input::ButtonDown);//f6
    shared_ptr<Input> saveKey = Input::CreateKeyboardInput(0x41, Input::ButtonDown);//f7
    shared_ptr<Input> loadKey = Input::CreateKeyboardInput(0x42, Input::ButtonDown);//f8

    vector<shared_ptr<Input> > hotkeys;

    //virtual key
    // KeyboardInput playKey = KeyboardInput(WM_KEYDOWN, 0x70);
    // KeyboardInput recordKey = KeyboardInput(WM_KEYDOWN, 0x71);
    // KeyboardInput stopKey = KeyboardInput(WM_KEYDOWN, 0x72);

    int currentInputIndex = 0;

    sf::Clock *globalClock;//pointer to the global clock

    sf::Time startTime;

    std::mutex mutex;
};