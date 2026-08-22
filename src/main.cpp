#include "../include/Game.hpp"

Game::Game()
        : window(sf::VideoMode(1200, 900), "Proyecto Base"),
            player("assets/images/S0.png", 7, 0.20f, 1, true) {
        player.setPosition(600.f, 450.f);
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
            nextAnimation();
        }
    }
}

void Game::nextAnimation() {
    static const char* imagePaths[] = {
        "assets/images/S0.png",
        "assets/images/S1.png",
        "assets/images/S2.png",
        "assets/images/S0.png",
        "assets/images/S3.png",
        "assets/images/S4.png",
    };
    static const unsigned int frameCounts[] = {7, 8, 6, 7, 8, 8};
    static const unsigned int firstFrames[] = {1, 1, 1, 1, 0, 0};
    static const bool loopAnimations[] = {true, false, false, true, false, false};

    animationIndex = (animationIndex + 1) % 6;
    player.setAnimation(imagePaths[animationIndex], frameCounts[animationIndex], firstFrames[animationIndex], loopAnimations[animationIndex]);
}

void Game::update(float deltaTime) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        player.move(-200.f * deltaTime, 0.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        player.move(200.f * deltaTime, 0.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        player.move(0.f, -200.f * deltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        player.move(0.f, 200.f * deltaTime);
    }

    player.update(deltaTime);
    if ((animationIndex == 2 || animationIndex == 4) && player.hasCompletedCycle()) {
        nextAnimation();
    }
}

void Game::render() {
    window.clear(sf::Color::Black);
    window.draw(player.getSprite());
    window.display();
}

void Game::run() {
    while (window.isOpen()) {
        processEvents();
        float deltaTime = clock.restart().asSeconds();
        update(deltaTime);
        render();
    }
}

int main() {
    Game game;
    game.run();
    return 0;
}
