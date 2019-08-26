#include "Macro.hpp"
#include "StringMethods.h"
#include <iostream>
#include <fstream>
#include <algorithm>

using std::fstream;
using std::cout;
using std::ios;
using std::endl;

void Macro::beginPlayback(sf::Time startTime) {

    if(currentState != Stopped)
        return;

    _mutex.lock();
    playbackStartTime = startTime;
    currentState = Playing;

    _mutex.unlock();
}

void Macro::updatePlayback(sf::Time currentTime) {

    if(currentState != Playing)
        return;

    sf::Time elapsedTime = elapsedPlaybackTime(currentTime);

    while(currentInputIndex < inputs.size() &&
        elapsedTime >= inputs[currentInputIndex]->time) {
        
    cout << elapsedTime.asMilliseconds() << endl;
        _mutex.lock();
        inputs[currentInputIndex]->sendToSystem();
        currentInputIndex++;
        _mutex.unlock();
    }

    if(currentInputIndex >= inputs.size())
        stop();
}

void Macro::stop() {
    _mutex.lock();
    currentInputIndex = 0;
    currentState = Stopped;
    keysToPressAfterUnpause.clear();
    _mutex.unlock();
}

void Macro::pause(sf::Time currentTime) {

    if(currentState != Playing) {
        cout << "paused while not playing" << endl;
        return;
    }

    _mutex.lock();
    pausedTime = currentTime;
    currentState = Paused;
    _mutex.unlock();


    //special processing for pausing
    //need to unpress all buttons while they are paused
    //next, if a key is held down (down happens and up won't happen for a long time) then we want to send a keyup event, AND insert another down event for hte same key so that
    //it's pressed down again when the macro is resumed
    //THEY LOCK MUTEX THEMSELVES, DON"T PUT IN LOCK
    releaseDownedButtonsforPause(currentTime);
    duplicateHeldKeysForUnpause(currentTime);
}

void Macro::resume(sf::Time currentTime) {

    if(currentState != Paused)
        return;
        
    //figure out how long it was paused for
    sf::Time pausedInterval = currentTime - pausedTime;

    _mutex.lock();
    //offset playback time by paused time because it's used to calculate when inputs occur
    playbackStartTime += pausedInterval;

    //press any buttons that need to be held down
    for(unsigned i = 0; i < keysToPressAfterUnpause.size(); ++i) {
        keysToPressAfterUnpause[i]->print();
        keysToPressAfterUnpause[i]->sendToSystem();
    }

    keysToPressAfterUnpause.clear();
    currentState = Playing;

    _mutex.unlock();
}

sf::Time Macro::getTimeToNextInput(sf::Time currentTime) {

    if(currentInputIndex < inputs.size())
        return inputs[currentInputIndex]->time - elapsedPlaybackTime(currentTime);

    return sf::milliseconds(0);
}

void Macro::sort() {

    _mutex.lock();
    std::sort(inputs.begin(), inputs.end(), compareInputPointers);
    _mutex.unlock();
}

bool Macro::save(string fileName) {

    std::lock_guard<std::mutex> _lock(_mutex);

    fstream file;
    file.open(fileName, ios::out);

    if(!file) {
        return false;
    }

    file << "#" << Input::KeyboardFormatDescriptionString << endl;
    file << "#" << Input::MouseFormatDescriptionString << endl;
    file << "#first line is macro name" << endl;
    file << name << endl;

    for(unsigned i = 0; i < inputs.size(); ++i) {
        file << *inputs[i] << endl;
    }

    file.close();

    return true;
}

bool Macro::load(string fileName) {
    _mutex.lock();

    string file = loadFileToString(fileName);
    
    if(file == "") {
        return false;
    }

    vector<string> lines;
    separateIntoWords(file, lines, '\n');

    if(lines.size() == 0) {
        cout << "Error, no DATA to load" << endl;
        return false;
    }

    //get rid of comments
    //need to do this separately so the name is the first entry
    for(unsigned i = 0; i < lines.size();) {
        if(lines[i][0] == '#') {
            lines.erase(lines.begin() + i);
            continue;
        }

        ++i;
    }

    //first non comment line should be the name
    name = lines[0];

    //start from the 2nd line since the first line isn't an input
    for(unsigned i = 1; i < lines.size(); ++i) {
        auto input = Input::CreateInputFromDescriptionString(lines[i]);
        inputs.push_back(input);
    }

    _mutex.unlock();

    sort();

    return true;
}

void Macro::insertInput(shared_ptr<Input> input) {
    _mutex.lock();
    inputs.push_back(input);
    _mutex.unlock();
}

void Macro::reset() {
    _mutex.lock();

    inputs.clear();
    keysToPressAfterUnpause.clear();
    currentInputIndex = 0;
    currentState = Stopped;
    name = "";

    _mutex.unlock();
}

sf::Time Macro::elapsedPlaybackTime(sf::Time currentTime) {
    return currentTime - playbackStartTime;
}

void Macro::getDownedKeysWhilePaused(vector<shared_ptr<Input> > &downedKeys) {

    //get all the buttons that have been held down until the pause happened
    //currentInputIndex represents the input to be played after it's unpaused, so only need to look for inputs until then
    for(unsigned i = 0; i < currentInputIndex ; ++i) {
        
        //button not presed down, ignore
        if(inputs[i]->getType() != Input::ButtonDown)
            continue;
        
        bool isReleased = false;
        //button down, check if theres a corresponding button up before the pause
        for(unsigned j = i + 1; j < currentInputIndex; ++j) {

            if(inputs[i]->isSameButtonReleased(*inputs[j]) ) {
                //button was released, ignore it
                isReleased = true;
                break;
            }
        }

        if(!isReleased)
            downedKeys.push_back(inputs[i]);
    }

    cout << "DOWNED COUNT: " <<downedKeys.size() << endl;
}

    
void Macro::releaseDownedButtonsforPause(sf::Time currentTime) {

    vector<shared_ptr<Input> > downedKeys;

    getDownedKeysWhilePaused(downedKeys);

    //find a corresponding keyup event for the given keys within 110 seconds of them, if there is one then release them since it was a tap
    sf::Time tapThreshold = sf::milliseconds(110);

    for(unsigned i = 0; i < downedKeys.size(); ++i) {

        auto downedKey = downedKeys[i];

        _mutex.lock();
        downedKey->sendToSystemAsType(Input::ButtonUp);
        _mutex.unlock();
        // for(unsigned j = currentInputIndex; j < inputs.size() && inputs[j]->time - downedKey->time < tapThreshold; ++j) {
            
        //     //found the release button within the given time frame, release it.
        //     if(downedKey->isSameButtonReleased(*inputs[j])) {
        //         _mutex.lock();
        //         inputs[j]->sendToSystem();
        //         _mutex.unlock();
        //         break;
        //     }
        // }
    }
}

void Macro::duplicateHeldKeysForUnpause(sf::Time currentTime) {
    vector<shared_ptr<Input> > downedKeys;

    getDownedKeysWhilePaused(downedKeys);

    //find a corresponding keyup event for the given keys within 110 seconds of them
    //if there is none then it must be pressed again after unpausing because it is being held down
    sf::Time tapThreshold = sf::milliseconds(110);

    for(unsigned i = 0; i < downedKeys.size(); ++i) {

        auto downedKey = downedKeys[i];

        bool heldDown = true;

        for(unsigned j = currentInputIndex; j < inputs.size() && inputs[j]->time - downedKey->time < tapThreshold; ++j) {
            
            //found the release button within the given time frame, it's a tap so don't hold it down
            if(downedKey->isSameButtonReleased(*inputs[j])) {
                heldDown = false;
            }
        }

        if(heldDown)
            keysToPressAfterUnpause.push_back(downedKey);
    }
}