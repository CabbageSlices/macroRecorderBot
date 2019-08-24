#include "Macro.hpp"
#include <algorithm>

void Macro::beginPlayback(sf::Time startTime) {

    _mutex.lock();
    playbackStartTime = startTime;
    currentState = Playing;
    _mutex.unlock();
}

void Macro::updatePlayback(sf::Time currentTime) {
    sf::Time elapsedTime = elapsedPlaybackTime(currentTime);

    while(currentInputIndex < inputs.size() &&
        elapsedTime >= inputs[currentInputIndex]->time) {
        
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
    _mutex.unlock();
}

void Macro::pause(sf::Time currentTime) {
    _mutex.lock();
    pausedTime = currentTime;

    currentState = Paused;
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

void Macro::save(string fileName) {
}

void Macro::insertInput(shared_ptr<Input> input) {
    _mutex.lock();
    inputs.push_back(input);
    _mutex.unlock();
}

void Macro::reset() {
    _mutex.lock();

    inputs.clear();
    currentInputIndex = 0;
    currentState = Stopped;
    name = "";

    _mutex.unlock();
}

sf::Time Macro::elapsedPlaybackTime(sf::Time currentTime) {
    return currentTime - playbackStartTime;
}