#include "RuneNavigator.hpp"

vector<string> RuneNavigator::playerImageNames = {
    "idleLeft.png",
    "idleRight.png",
    "breathingLeft.png",
    "breathingRight.png",
};

RuneNavigator::RuneNavigator(const string &pathToRuneImages, const string &pathToPlayerImages) {

    playerFinder.loadTargetImages(pathToPlayerImages, playerImageNames);
    
    vector<string> imageNames = {"rune1.png", "rune2.png"};
    runeFinder.loadTargetImages(pathToRuneImages, imageNames);
}

void RuneNavigator::navigateToRune(cv::Mat &screenshot) {

    sf::Vector2i runePos = runeFinder.getObjectLocation(screenshot);
    sf::Vector2i playerPos = playerFinder.getObjectLocation(screenshot);

    
}