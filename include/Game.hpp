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
    void showDirection(bool facingRight);

    sf::RenderWindow window;
    sf::Texture backgroundTexture;
    sf::Sprite background;
    sf::Music fondoMusic;
    sf::Music laboratorioMusic;
    sf::Music evolucionMusic;
    CharacterAnimation player;
    sf::Clock clock;
    unsigned int animationIndex = 0;
};
