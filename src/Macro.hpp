#include "Input.hpp"
#include <mutex>
#include <vector>
#include <SFML/System.hpp>
#include <string>

using std::mutex;
using std::string;
using std::vector;

class Macro {

public:

    enum State {
        Stopped,
        Playing,
        Paused,
    };

    void beginPlayback(sf::Time startTime);
    bool updatePlayback(sf::Time currentTime);//returns true if playback finished naturally (no more inputs)
    void stop();
    void pause(sf::Time currentTime);
    void resume(sf::Time currentTime);

    sf::Time getTimeToNextInput(sf::Time currentTime); //return the time left from now until the next input needs to be played

    void sort();

    bool save(string fileName);
    bool load(string fileName);
    void insertInput(shared_ptr<Input> input);
    void reset();
    void setName(string _name) {
        name = _name;
    }

    State getCurrentState() {
        return currentState;
    }

private:

    sf::Time elapsedPlaybackTime(sf::Time currentTime);
    void getDownedKeysWhilePaused(vector<shared_ptr<Input> > &downedKeys);
    void releaseDownedButtonsforPause(sf::Time currentTime);
    void duplicateHeldKeysForUnpause(sf::Time currentTime);

    State currentState;
    sf::Time playbackStartTime;//global time
    sf::Time pausedTime;//global time

    string name;
    vector<shared_ptr<Input> > inputs;
    vector<shared_ptr<Input> > keysToPressAfterUnpause;

    unsigned int currentInputIndex;
    mutex _mutex;
};