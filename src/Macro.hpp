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
    void updatePlayback(sf::Time currentTime);
    void stop();
    void pause(sf::Time currentTime);

    sf::Time getTimeToNextInput(sf::Time currentTime); //return the time left from now until the next input needs to be played

    void sort();

    void save(string fileName);
    void insertInput(shared_ptr<Input> input);
    void reset();
    

    State getCurrentState() {
        return currentState;
    }

private:

    sf::Time elapsedPlaybackTime(sf::Time currentTime);

    State currentState;
    sf::Time playbackStartTime;//global time
    sf::Time pausedTime;//global time

    string name;
    vector<shared_ptr<Input> > inputs;

    unsigned int currentInputIndex;
    mutex _mutex;
};