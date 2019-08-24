#include "MacroRecorder.hpp"
#include <iostream>
#include <algorithm>
#include <thread>

using namespace std;

using std::thread;
using std::sort;
using std::mutex;
using std::lock_guard;

MacroRecorder::MacroRecorder(sf::Clock *_globalClock) :
    globalClock(_globalClock), currentState(Idle)
    {
        hotkeys.push_back(playKey);
        hotkeys.push_back(stopKey);
        hotkeys.push_back(recordKey);
        hotkeys.push_back(saveKey);
        stop(true);
    }

void MacroRecorder::processInput(shared_ptr<Input> input) {

    //ignore key releases for hotkeys
    // if(stopKey->isSameButtonReleased(*input) || recordKey->isSameButtonReleased(*input) || playKey->isSameButtonReleased(*input)) {
    //     return;
    // }

    for(unsigned i = 0; i < hotkeys.size(); ++i) {
        if(hotkeys[i]->isSameButtonReleased(*input))
            return;
    }

    switch(currentState) {
        case Playing: 
            if(input->isSameInput(*stopKey)) {
                stop(true);
            }
            break;//can't let this fall through because it might continue recording
        
        case Recording: {
            
            if(input->isSameInput(*stopKey)) {
                stop(true);
                break;
            }

            if(input->isSameInput(*playKey) || input->isSameInput(*recordKey) || input->isSameInput(*saveKey)) {
                break;//don't record the play or record button, otherwise you'll get weird bugs
            }

            //input times are relative to the start of the program, and startTime is also relative to the start of the program
            //need to change input time to be relative to the start time
            input->time -= startTime;
            currentMacro.insertInput(input);

            // mutex.lock();

            // //input, record the input
            // inputs.push_back(input);
            // currentMacro.insertInput(input);
            // mutex.unlock();
            break;
        }

        case Idle: {

            //if play was pressed
            if(input->isSameInput(*playKey)) {
                // std::lock_guard<std::mutex> lock(mutex);
                // startTime = input->time;
                //printKeys();
                play(true);
                break;
            }

            //if record was pressed
            if(input->isSameInput(*recordKey)) {
                // std::lock_guard<std::mutex> lock(mutex);
                // startTime = input->time;
                startRecording(true);
                break;
            }

            if(input->isSameInput(*saveKey)) {
                save(true);
            }
            
            //if clear was pressed 
            // clearMacro();
            break;
        }
        default:
            break;
        //do nothhing by default

    }
}
void MacroRecorder::update() {

    while(currentState == Playing && currentMacro.getCurrentState() == Macro::Playing) {

        sf::Time currentTime = globalClock->getElapsedTime();

        if(currentMacro.getTimeToNextInput(currentTime) > sf::milliseconds(7))
            Sleep(1);

        // sf::Time timeSincePlayBegan = globalClock->getElapsedTime() - startTime;

        // mutex.lock();
        currentMacro.updatePlayback(currentTime);
        // mutex.unlock();

        //finished playing so its stopped
        if(currentMacro.getCurrentState() != Macro::Playing) {
            stop(true);
            return;
        }

        //playing
        // while(currentInputIndex < inputs.size() &&
        //     timeSincePlayBegan >= inputs[currentInputIndex]->time) {
            
        //     mutex.lock();
        //     inputs[currentInputIndex]->sendToSystem();
        //     currentInputIndex++;
        //     mutex.unlock();
        // }

        // if(currentInputIndex >= inputs.size()) {
        //     stop(true);
        //     return;
        // }

        //prevent taking system resources if the next input isn't coming up anytime soon
    
    }
}

void MacroRecorder::printKeys(bool block) {
    cout << endl << endl <<"\n\nPRINTING KEYS +++++++++" << endl;
    for(unsigned i = 0; i < inputs.size() ; ++i) {
        inputs[i]->print();
    }
}

void MacroRecorder::startRecording(bool block) {

    if(block)
        mutex.lock();

    //clear previous recording
    clearMacro();

    currentState = Recording;
    startTime = globalClock->getElapsedTime();

    if(block)
        mutex.unlock();

    cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << endl;
    cout << "Recording. Press F3 at anytime to stop recording" << endl;
}

void MacroRecorder::cleanupInputs(bool block) {

    if(block)
        mutex.lock();
    sort(inputs.begin(), inputs.end(), compareInputPointers);


    if(block)
        mutex.unlock();

    currentMacro.sort();

    //if key is pressed and released witin certain interval, make system send the two events in a single sendinput call to avoid interruption
    //combine button down and button up events

    // sf::Time buttonMergeInterval = sf::milliseconds(130);

    // for(unsigned i = 0; i < inputs.size(); i++) {

    //     if(inputs[i]->getType() == Input::ButtonUp) {
    //         continue;
    //     }

    //     auto downedButton = inputs[i];

    //     //button down, find corresponding button up
    //     //only combine if the button up event corresponding ot this button occured within a certain time interval            
    //     for(unsigned j = i + 1; j < inputs.size();) {

    //         auto toTest = inputs[j];

    //         //array sorted by time, no matter when the buttonup occurs from this point forward, it will never be within the merge interval so just stop
    //         if(toTest->time - downedButton->time > buttonMergeInterval) {
    //             break;
    //         }

    //         //VERY IMPORTANt, when you hold a button down it will generate multiple buttondown events for the same button:
    //         //down1, down2, down3, down4, up
    //         //since the button repeat interval is smaller than the button merge interval you will merge a button down with a button up, but THERE WILL BE BUTTON DOWNS REMAINING
    //         //WITH NO MATCHING BUTTON UP
    //         //ex: down2 and up will be merged, but down3, and down4 will remain.
    //         //thus if you encounter a buton down for the same key, you have to stop searching for a button up to prevent merging too early.
    //         if(downedButton->isSameInput(*toTest))
    //             break;

    //         //release code, and it's within merge interval, combine the two and replace the button down
    //         if(downedButton->isSameButtonReleased(*toTest)) {
    //             downedButton->setType(Input::ButtonDownUp);
    //             inputs.erase(inputs.begin() + j);
    //             break; //nothing elft to merge since you can only merge 1 buttondown/up
    //         }

    //         //some other button so just leave
    //         ++j;
    //     }
    // }
}

void MacroRecorder::play(bool block) {

    if(block)
        mutex.lock();

    cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << endl;
    cout << "Playing macro, press F3 at anytime to stop" << endl;
    currentInputIndex = 0;
    currentState = Playing;
    startTime = globalClock->getElapsedTime();

    if(block)
        mutex.unlock();

        
    currentMacro.beginPlayback(startTime);

    thread updateThread(MacroRecorder::update, this);
    updateThread.detach();
}

void MacroRecorder::stop(bool block) {

    if(block)
        mutex.lock();

    currentInputIndex = 0;
    currentState = Idle;

    if(block)
        mutex.unlock();

    currentMacro.stop();
    currentMacro.sort();

    cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << endl;
    cout << "Press the following buttons at anytime on any window to perform the indicated action" << endl;
    cout << "f2: Begin recording macro (this will override the current recording)" << endl;
    // cout << "f3: Stop recording/ playback of macro" << endl;
    cout << "f4: Play macro" << endl;
    cout << "f5: save macro" << endl;
    cout << "f6: load macro" << endl;

    // cleanupInputs();
}

void MacroRecorder::save(bool block) {

    if(block) {
        mutex.lock();
    }

    currentState = saving;
    cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << endl;
    cout << "Please type in the name of the macro and hit enter." << endl;

    string fileName;
    std::getline(cin, fileName);

    //strip spaces
    for(unsigned i = 0; i < fileName.size();) {
        if(fileName[i] == ' ') {

            fileName.erase(fileName.begin() + i);
            continue;
        }
        ++i;
    }

    cout << "saving, please wait" << endl;
    currentMacro.save(fileName + ".txt");

    cout << "save complete" << endl;
    if(block) {
        mutex.unlock();
    }
}

void MacroRecorder::clearMacro(bool block) {

    if(block)
        mutex.lock();

    inputs.clear();

    
    if(block)
        mutex.unlock();

    currentMacro.reset();

}