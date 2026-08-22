#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include "../src/CharacterAnimation.hpp"

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float deltaTime);
    void render();
    void nextAnimation();

    sf::RenderWindow window;
    CharacterAnimation player;
    sf::Clock clock;
    unsigned int animationIndex = 0;
};
