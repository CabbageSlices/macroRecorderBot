#include "MacroRecorder.hpp"
#include <iostream>
#include <algorithm>
#include <thread>

using namespace std;

using std::thread;
using std::sort;
using std::mutex;
using std::lock_guard;

void MacroRecorder::processInput(shared_ptr<Input> input) {

    //ignore key releases for hotkeys
    if(stopKey->isSameButtonReleased(*input) || recordKey->isSameButtonReleased(*input) || playKey->isSameButtonReleased(*input)) {
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

            if(input->isSameInput(*playKey) || input->isSameInput(*recordKey)) {
                break;//don't record the play or record button, otherwise you'll get weird bugs
            }

            //input times are relative to the start of the program, and startTime is also relative to the start of the program
            //need to change input time to be relative to the start time
            input->time -= startTime;

            mutex.lock();

            //input, record the input
            inputs.push_back(input);
            mutex.unlock();
            break;
        }

        case Idle: {
            std::lock_guard<std::mutex> lock(mutex);

            //if play was pressed
            if(input->isSameInput(*playKey)) {
                startTime = input->time;
                //printKeys();
                play();
                break;
            }

            //if record was pressed
            if(input->isSameInput(*recordKey)) {
                startTime = input->time;
                startRecording();
                break;
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

    while(currentState == Playing) {

        sf::Time timeSincePlayBegan = globalClock->getElapsedTime() - startTime;

        //playing
        while(currentInputIndex < inputs.size() &&
            timeSincePlayBegan >= inputs[currentInputIndex]->time) {
            
            mutex.lock();
            inputs[currentInputIndex]->sendToSystem();
            currentInputIndex++;
            mutex.unlock();
        }

        if(currentInputIndex >= inputs.size()) {
            stop(true);
            return;
        }

        //prevent taking system resources if the next input isn't coming up anytime soon
        if(inputs[currentInputIndex]->time - timeSincePlayBegan > sf::milliseconds(10))
            Sleep(1);
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
}

void MacroRecorder::cleanupInputs() {

    sort(inputs.begin(), inputs.end(), compareInputPointers);

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

    currentInputIndex = 0;
    currentState = Playing;
    startTime = globalClock->getElapsedTime();

    if(block)
        mutex.unlock();

    thread updateThread(MacroRecorder::update, this);
    updateThread.detach();
}

void MacroRecorder::stop(bool block) {

    if(block)
        mutex.lock();

    currentInputIndex = 0;
    currentState = Idle;

    cleanupInputs();

    if(block)
        mutex.unlock();
}

void MacroRecorder::save(bool block) {

    if(block) {
        mutex.lock();
    }


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
}