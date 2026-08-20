#include "../include/Game.hpp"

Game::Game() : window(sf::VideoMode(1200, 900), "Proyecto Base") {
    player.setRadius(20.f);
    player.setFillColor(sf::Color::Green);
    player.setPosition(100.f, 100.f);
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
    }
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
}

void Game::render() {
    window.clear(sf::Color::Black);
    window.draw(player);
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
