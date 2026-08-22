#include "../include/Game.hpp"

Game::Game()
        : window(sf::VideoMode(1200, 900), "Proyecto Base"),
            player("assets/images/S0.png", 7, 0.20f, 1, true) {
    window.setKeyRepeatEnabled(false);
    backgroundTexture.loadFromFile("assets/images/Laboratorio.jpg");
    background.setTexture(backgroundTexture);
    background.setScale(
        1200.f / backgroundTexture.getSize().x,
        900.f / backgroundTexture.getSize().y
    );
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
        if (event.type == sf::Event::KeyPressed &&
            (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::Up)) {
            showDirection(true);
        }
        if (event.type == sf::Event::KeyPressed &&
            (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Down)) {
            showDirection(false);
        }
        if (event.type == sf::Event::KeyReleased &&
            (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Right ||
             event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Left) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Right) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Up) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            if (animationIndex == 6) {
                player.setAnimation("assets/images/E1.png", 7, 0, false);
                player.showLastFrame();
            } else if (animationIndex == 7) {
                player.setAnimation("assets/images/E2.png", 5, 0, false);
                player.showFrame(4);
            } else if (animationIndex == 8) {
                player.setAnimation("assets/images/E3.png", 6, 0, false);
                player.showFrame(5);
            } else {
                player.setAnimation("assets/images/S1.png", 8, 1, false);
                player.showFrame(0);
            }
        }
    }
}

void Game::showDirection(bool facingRight) {
    if (animationIndex == 6 && facingRight) {
        player.setAnimation("assets/images/C1.png", 7, 0, true);
    } else if (animationIndex == 6) {
        player.setAnimation("assets/images/CI1.png", 7, 0, true);
    } else if (animationIndex == 7 && facingRight) {
        player.setAnimation("assets/images/C2.png", 6, 0, true);
    } else if (animationIndex == 7) {
        player.setAnimation("assets/images/CI2.png", 7, 0, true);
    } else if (animationIndex == 8 && facingRight) {
        player.setAnimation("assets/images/C3.png", 6, 0, true);
    } else if (animationIndex == 8) {
        player.setAnimation("assets/images/CI3.png", 7, 0, true);
    } else if (facingRight) {
        player.setAnimation("assets/images/S0.png", 7, 1, true);
    } else {
        player.setAnimation("assets/images/C0.png", 6, 0, true);
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
        "assets/images/E1.png",
        "assets/images/E2.png",
        "assets/images/E3.png",
    };
    static const unsigned int frameCounts[] = {7, 8, 6, 7, 8, 8, 7, 5, 6};
    static const unsigned int firstFrames[] = {1, 1, 1, 1, 0, 0, 0, 0, 0};
    static const bool loopAnimations[] = {true, false, false, true, false, false, false, false, false};

    animationIndex = (animationIndex + 1) % 9;
    player.setAnimation(imagePaths[animationIndex], frameCounts[animationIndex], firstFrames[animationIndex], loopAnimations[animationIndex]);
}

void Game::update(float deltaTime) {
    const float movementSpeed = 200.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        player.move(-movementSpeed * deltaTime, 0.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        player.move(movementSpeed * deltaTime, 0.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        player.move(0.f, -movementSpeed * deltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        player.move(0.f, movementSpeed * deltaTime);
    }

    player.update(deltaTime);
    if ((animationIndex == 2 || animationIndex == 4) && player.hasCompletedCycle()) {
        nextAnimation();
    }
}

void Game::render() {
    window.clear(sf::Color::Black);
    window.draw(background);
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
