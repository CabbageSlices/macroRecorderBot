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
        hotkeys.push_back(loadKey);
        hotkeys.push_back(pauseKey);
        stop();
    }

void MacroRecorder::processInput(shared_ptr<Input> input) {

    for(unsigned i = 0; i < hotkeys.size(); ++i) {
        if(hotkeys[i]->isSameButtonReleased(*input))
            return;
    }

    switch(currentState) {
        case Playing: 
            if(input->isSameInput(*stopKey)) {
                stop();
            }

            if(input->isSameInput(*pauseKey)) {
                pause();
            }
            break;//can't let this fall through because it might continue recording
        
        case Recording: {
            
            if(input->isSameInput(*stopKey)) {
                stop();
                break;
            }

            //ignore hotkeys while recording
            bool ignoreKey = false;
            for(unsigned i = 0; i < hotkeys.size(); ++i) {
                if(hotkeys[i]->isSameInput(*input)) {
                    ignoreKey = true;
                    break;
                }
            }

            if(ignoreKey) {
                break;
            }

            //input times are relative to the start of the program, and startTime is also relative to the start of the program
            //need to change input time to be relative to the start time
            input->time -= startTime;
            currentMacro.insertInput(input);

            break;
        }

        case Idle: {

            //if play was pressed
            if(input->isSameInput(*playKey)) {
                
                //printKeys();
                play();
                break;
            }

            //if record was pressed
            if(input->isSameInput(*recordKey)) {
               
                startRecording();
                break;
            }

            if(input->isSameInput(*saveKey)) {
                save();
                break;;
            }

            if(input->isSameInput(*loadKey)) {
                load();
                break;;
            }
            
            //if clear was pressed 
            // clearMacro();
            break;
        }

        case Paused: {
            if(pauseKey->isSameInput(*input) || playKey->isSameInput(*input)) {
                unpause();
                break;
            }

            if(stopKey->isSameInput(*input)) {
                stop();
                break;
            }
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

        //playback finished naturally, so repeat it if wanted
        if(currentMacro.updatePlayback(currentTime)) {
            stop();
            play();
            return;
        }

        if(currentMacro.getCurrentState() == Macro::Paused) {
            return;
        }
    }
    
    //the check inside the loop is to enable repeat, this is to change state when macro naturally finishes
    if(currentMacro.getCurrentState() == Macro::Stopped)
        stop();
}

void MacroRecorder::printKeys() {
    cout << endl << endl <<"\n\nPRINTING KEYS +++++++++" << endl;
    for(unsigned i = 0; i < inputs.size() ; ++i) {
        inputs[i]->print();
    }
}

void MacroRecorder::startRecording() {

    //clear previous recording
    //blocks on it's own so no need to lock the mutex
    clearMacro();

    mutex.lock();

    currentState = Recording;
    startTime = globalClock->getElapsedTime();

    mutex.unlock();

    cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << endl;
    cout << "Recording. Press F3 at anytime to stop recording" << endl;
}

void MacroRecorder::cleanupInputs() {


    mutex.lock();
    sort(inputs.begin(), inputs.end(), compareInputPointers);



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

void MacroRecorder::play() {

    cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << endl;
    cout << "Playing macro, press F3 at anytime to stop" << endl;

    mutex.lock();

    currentInputIndex = 0;
    currentState = Playing;
    startTime = globalClock->getElapsedTime();

    mutex.unlock();

        
    currentMacro.beginPlayback(startTime);

    thread updateThread(MacroRecorder::update, this);
    updateThread.detach();
}

void MacroRecorder::stop() {

    mutex.lock();

    currentInputIndex = 0;
    currentState = Idle;

    mutex.unlock();

    currentMacro.stop();
    currentMacro.sort();

    cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << endl;
    cout << "Press the following buttons at anytime on any window to perform the indicated action" << endl;
    cout << "f2: Begin recording macro (this will override the current recording)" << endl;
    cout << "f4: Play macro" << endl;
    cout << "f7: save macro" << endl;
    cout << "f8: load macro" << endl;
}

void MacroRecorder::pause() {

    mutex.lock();

    currentMacro.pause(globalClock->getElapsedTime());
    currentState = Paused;

    mutex.unlock();

    cout << "Paused playback, press f4 (play key) to resume" << endl; 

}

void MacroRecorder::unpause() {
    mutex.lock();
    currentMacro.resume(globalClock->getElapsedTime());
    currentState = Playing;
    mutex.unlock();

    thread updateThread(MacroRecorder::update, this);
    updateThread.detach();
}

void MacroRecorder::save() {

    mutex.lock();

    currentState = Saving;

    cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << endl;
    cout << "Please type in the name of the macro and hit enter." << endl;

    string fileName;
    std::getline(cin, fileName);

    currentMacro.setName(fileName);

    //strip spaces
    for(unsigned i = 0; i < fileName.size();) {
        if(fileName[i] == ' ') {

            fileName.erase(fileName.begin() + i);
            continue;
        }
        ++i;
    }

    cout << "saving, please wait" << endl;

    if(!currentMacro.save(fileName + ".txt")) {
        cout << "FAILED TO SAVE MACRO" << endl;
    } else {
        cout << "save complete" << endl;
    }


    mutex.unlock();

    stop();
}

void MacroRecorder::load() {
    mutex.lock();

    currentState = Loading;
    cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << endl;
    cout << "Please type in the filename of the macro and hit enter(without the .txt extension)." << endl;

    string fileName;
    std::getline(cin, fileName);

    if(!currentMacro.load(fileName + ".txt")) {
        cout << "FAILED TO LOAD MACRO" << endl;
    } else {
        cout << "successfuly loaded macro" << endl;
    }

    mutex.unlock();
    stop();

}

void MacroRecorder::clearMacro() {

    mutex.lock();

    inputs.clear();

    mutex.unlock();

    currentMacro.reset();
}