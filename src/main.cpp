#include "../include/Game.hpp"

#include <algorithm>
#include <cmath>
#include <random>

Game::Game()
        : window(sf::VideoMode(1200, 900), "Proyecto Base"),
        player("assets/images/S1.png", 8, 0.20f, 0, false) {
    window.setKeyRepeatEnabled(false);
    backgroundTexture.loadFromFile("assets/images/Laboratorio.jpg");
    background.setTexture(backgroundTexture);
    background.setScale(
        1200.f / backgroundTexture.getSize().x,
        900.f / backgroundTexture.getSize().y
    );
    fondoMusic.openFromFile("assets/music/Fondo.ogg");
    fondoMusic.setLoop(true);
    fondoMusic.play();
    laboratorioMusic.openFromFile("assets/music/Laboratorio.ogg");
    laboratorioMusic.setLoop(true);
    laboratorioMusic.play();
    evolucionMusic.openFromFile("assets/music/Evolucion.ogg");
    desmoronamientoMusic.openFromFile("assets/music/Desmoronamiento.ogg");
    setupItems();
    setStageImage("assets/images/Et1.jpg");
    player.showFrame(0);
    player.setPosition(600.f, 450.f);
    for (std::size_t charcoIndex = 0; charcoIndex < charcos.size(); ++charcoIndex) {
        charcosEnContacto[charcoIndex] = touchesItemCenter(charcos[charcoIndex]);
    }
}

void Game::setStageImage(const std::string& imagePath) {
    stageTexture.loadFromFile(imagePath);
    stageImage.setTexture(stageTexture);
    stageImage.setPosition(0.f, 0.f);
    stageImage.setScale(
        280.f / stageTexture.getSize().x,
        239.f / stageTexture.getSize().y
    );
}

void Game::setupItems() {
    bateriaTexture.loadFromFile("assets/images/Bateria.png");
    bateriasProgressTexture.loadFromFile("assets/images/Baterias.png");
    bateriasProgress.setTexture(bateriasProgressTexture);
    bateriasProgress.setTextureRect(sf::IntRect(0, 0, 768, 528));
    bateriasProgress.setScale(80.f / 768.f, 80.f / 768.f);
    bateriasProgress.setPosition(1100.f, 20.f);
    bateriasProgress.setColor(sf::Color(255, 255, 255, 0));
    librosTexture.loadFromFile("assets/images/Libros.png");
    charcoTexture.loadFromFile("assets/images/Charco.png");

    for (unsigned int itemIndex = 0; itemIndex < 4; ++itemIndex) {
        baterias.emplace_back(bateriaTexture);
        libros.emplace_back(librosTexture);
    }
    for (unsigned int itemIndex = 0; itemIndex < 2; ++itemIndex) {
        charcos.emplace_back(charcoTexture);
    }
    charcosEnContacto.assign(charcos.size(), false);
    charcosRespawnClocks.resize(charcos.size());
    charcosVisibles.assign(charcos.size(), true);

    for (sf::Sprite& item : baterias) {
        const sf::Vector2u size = bateriaTexture.getSize();
        const float largestSide = static_cast<float>(std::max(size.x, size.y));
        item.setOrigin(size.x / 2.f, size.y / 2.f);
        item.setScale(60.f / largestSide, 60.f / largestSide);
        placeRandomly(item);
    }
    for (sf::Sprite& item : libros) {
        const sf::Vector2u size = librosTexture.getSize();
        const float largestSide = static_cast<float>(std::max(size.x, size.y));
        item.setOrigin(size.x / 2.f, size.y / 2.f);
        item.setScale(60.f / largestSide, 60.f / largestSide);
        placeRandomly(item);
    }
    for (std::size_t itemIndex = 0; itemIndex < charcos.size(); ++itemIndex) {
        sf::Sprite& item = charcos[itemIndex];
        const sf::Vector2u size = charcoTexture.getSize();
        const float largestSide = static_cast<float>(std::max(size.x, size.y));
        item.setOrigin(size.x / 2.f, size.y / 2.f);
        item.setScale(100.f / largestSide, 100.f / largestSide);
        item.setPosition(itemIndex == 0 ? 600.f : 900.f, 450.f);
    }
}

void Game::placeRandomly(sf::Sprite& item) {
    static std::random_device randomDevice;
    static std::mt19937 generator(randomDevice());
    static std::uniform_real_distribution<float> horizontal(30.f, 1170.f);
    static std::uniform_real_distribution<float> vertical(30.f, 870.f);
    item.setPosition(horizontal(generator), vertical(generator));
}

void Game::updateItems(float deltaTime) {
    for (std::size_t charcoIndex = 0; charcoIndex < charcos.size(); ++charcoIndex) {
        sf::Sprite& item = charcos[charcoIndex];
        if (!charcosVisibles[charcoIndex] &&
            charcosRespawnClocks[charcoIndex].getElapsedTime().asSeconds() >= 5.f) {
            charcosVisibles[charcoIndex] = true;
            item.setColor(sf::Color::White);
        }
        if (!charcosVisibles[charcoIndex]) {
            continue;
        }
        const bool touchingCharco = touchesItemCenter(item);
        if (!touchingCharco) {
            charcosEnContacto[charcoIndex] = false;
        }
        if (!movementLocked && !oxidadoPlaying && touchingCharco && !charcosEnContacto[charcoIndex]) {
            if (charcosRecolectados < 2) {
                ++charcosRecolectados;
            }
            charcosEnContacto[charcoIndex] = true;
            animationBeforeOxidado = animationIndex;
            player.setAnimation("assets/images/Oxidado.png", 3, 0, false);
            setStageImage("assets/images/Oxid.jpg");
            player.setScaleToHeight(152.f);
            movementLocked = true;
            oxidadoPlaying = true;
            oxidadoIdle = false;
            charcosVisibles[charcoIndex] = false;
            charcosRespawnClocks[charcoIndex].restart();
            item.setColor(sf::Color(255, 255, 255, 0));
        }
    }
    for (auto item = baterias.begin(); item != baterias.end();) {
        item->rotate(90.f * deltaTime);
        if (!oxidadoPlaying && !oxidadoIdle && touchesItemCenter(*item)) {
            if (bateriasRecolectadas < 3) {
                ++bateriasRecolectadas;
                updateBatteryProgress();
            }
            item->setRotation(0.f);
            placeRandomly(*item);
            ++item;
        } else {
            ++item;
        }
    }
    for (auto item = libros.begin(); item != libros.end();) {
        item->rotate(90.f * deltaTime);
        if (!movementLocked && !oxidadoPlaying && !oxidadoIdle && touchesItemCenter(*item)) {
            if (animationIndex == 6 && charcosRecolectados < 2) {
                ++item;
                continue;
            }
            if (bateriasRecolectadas < 3) {
                ++item;
                continue;
            }
            evolucionMusic.stop();
            evolucionMusic.play();
            if (animationIndex < 4 && animationIndex != 2) {
                pendingAnimationIndex = 4;
                movementLocked = true;
                playS2Animation();
            } else if (animationIndex >= 4) {
                pendingAnimationIndex = animationIndex >= 8 ? 4 : animationIndex + 1;
                movementLocked = true;
                playS2Animation();
            }
            item->setRotation(0.f);
            placeRandomly(*item);
            ++item;
        } else {
            ++item;
        }
    }
}

bool Game::touchesItemCenter(const sf::Sprite& item) const {
    const sf::FloatRect localBounds = item.getLocalBounds();
    const sf::Vector2f center = item.getTransform().transformPoint(
        localBounds.left + localBounds.width / 2.f,
        localBounds.top + localBounds.height / 2.f
    );
    const sf::Vector2f scale = item.getScale();
    const float halfWidth = std::abs(localBounds.width * scale.x) * 0.20f;
    const float halfHeight = std::abs(localBounds.height * scale.y) * 0.20f;
    const sf::FloatRect centerArea(
        center.x - halfWidth,
        center.y - halfHeight,
        halfWidth * 2.f,
        halfHeight * 2.f
    );
    const sf::FloatRect playerBounds = player.getSprite().getGlobalBounds();
    const sf::FloatRect playerCenter(
        playerBounds.left + playerBounds.width * 0.25f,
        playerBounds.top + playerBounds.height * 0.25f,
        playerBounds.width * 0.5f,
        playerBounds.height * 0.5f
    );
    return playerCenter.intersects(centerArea);
}

void Game::updateBatteryProgress() {
    const int left = bateriasRecolectadas == 1 ? 0 :
        bateriasRecolectadas == 2 ? 768 : 1536;
    const int width = bateriasRecolectadas < 3 ? 768 : 384;
    bateriasProgress.setTextureRect(sf::IntRect(left, 0, width, 528));
    bateriasProgress.setScale(80.f / width, 80.f / width);
    bateriasProgress.setColor(sf::Color::White);
}

void Game::resetBatteryProgress() {
    bateriasRecolectadas = 0;
    bateriasProgress.setTextureRect(sf::IntRect(0, 0, 768, 528));
    bateriasProgress.setScale(80.f / 768.f, 80.f / 768.f);
    bateriasProgress.setColor(sf::Color(255, 255, 255, 0));
}

void Game::restoreStageIdle() {
    animationIndex = animationBeforeOxidado;
    if (animationIndex == 4) {
        player.setAnimation("assets/images/E1.png", 7, 0, false);
        player.showLastFrame();
    } else if (animationIndex == 5) {
        player.setAnimation("assets/images/E2.png", 5, 0, false);
        player.showFrame(4);
    } else if (animationIndex == 6) {
        player.setAnimation("assets/images/E3.png", 6, 0, false);
        player.showFrame(5);
    } else {
        player.setAnimation("assets/images/S1.png", 8, 1, false);
        player.showFrame(0);
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
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
            !movementLocked &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Left) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Right) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Up) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            if (animationIndex == 4) {
                player.setAnimation("assets/images/E1.png", 7, 0, false);
                player.showLastFrame();
            } else if (animationIndex == 5) {
                player.setAnimation("assets/images/E2.png", 5, 0, false);
                player.showFrame(4);
            } else if (animationIndex == 6) {
                player.setAnimation("assets/images/E3.png", 6, 0, false);
                player.showFrame(5);
            } else if (oxidadoIdle) {
                player.setAnimation("assets/images/Oxidado.png", 3, 0, false);
                player.setScaleToHeight(152.f);
                player.showFrame(2);
            } else {
                player.setAnimation("assets/images/S1.png", 8, 1, false);
                player.showFrame(0);
            }
        }
    }
}

void Game::playS2Animation() {
    animationIndex = 2;
    player.setAnimation("assets/images/S2.png", 6, 1, false);
}

void Game::showDirection(bool facingRight) {
    if (movementLocked) {
        return;
    }

    if (oxidadoIdle && facingRight) {
        setMovementAnimation("assets/images/COXD.png", 4);
    } else if (oxidadoIdle) {
        setMovementAnimation("assets/images/COXI.png", 5);
    } else if (animationIndex == 4 && facingRight) {
        setMovementAnimation("assets/images/C1.png", 7);
    } else if (animationIndex == 4) {
        setMovementAnimation("assets/images/CI1.png", 7, 0, 175.f);
    } else if (animationIndex == 5 && facingRight) {
        setMovementAnimation("assets/images/C2.png", 6);
    } else if (animationIndex == 5) {
        setMovementAnimation("assets/images/CI2.png", 7);
    } else if (animationIndex == 6 && facingRight) {
        setMovementAnimation("assets/images/C3.png", 6);
    } else if (animationIndex == 6) {
        setMovementAnimation("assets/images/CI3.png", 7);
    } else if (facingRight) {
        setMovementAnimation("assets/images/S0.png", 7, 1);
    } else {
        setMovementAnimation("assets/images/C0.png", 6);
    }
}

void Game::setMovementAnimation(const std::string& imagePath, unsigned int frameCount, unsigned int firstFrame, float height) {
    player.setAnimation(imagePath, frameCount, firstFrame, true);
    player.setScaleToHeight(height);
}

void Game::nextAnimation() {
    static const char* imagePaths[] = {
        "assets/images/S0.png",
        "assets/images/S1.png",
        "assets/images/S2.png",
        "assets/images/S0.png",
        "assets/images/E1.png",
        "assets/images/E2.png",
        "assets/images/E3.png",
        "assets/images/S3.png",
        "assets/images/S4.png",
    };
    static const unsigned int frameCounts[] = {7, 8, 6, 7, 7, 5, 6, 8, 8};
    static const unsigned int firstFrames[] = {1, 1, 1, 1, 0, 0, 0, 0, 0};
    static const bool loopAnimations[] = {true, false, false, true, false, false, false, false, false};

    if (animationIndex < 4) {
        animationIndex = 4;
    } else {
        ++animationIndex;
        if (animationIndex > 8) {
            animationIndex = 4;
        }
    }
    player.setAnimation(imagePaths[animationIndex], frameCounts[animationIndex], firstFrames[animationIndex], loopAnimations[animationIndex]);
    if (animationIndex == 4) {
        setStageImage("assets/images/Et2.jpg");
    } else if (animationIndex == 5) {
        setStageImage("assets/images/Et3.jpg");
    } else if (animationIndex == 6) {
        setStageImage("assets/images/Et4.jpg");
    } else if (animationIndex >= 7) {
        setStageImage("assets/images/Muerto.jpg");
    }
    if (animationIndex == 6) {
        e3Clock.restart();
        e3TimerActive = true;
    }
}

void Game::resetCycle() {
    animationIndex = 0;
    player.setAnimation("assets/images/S1.png", 8, 0, false);
    player.showFrame(0);
    setStageImage("assets/images/Et1.jpg");
    resetBatteryProgress();
    charcosRecolectados = 0;
    charcosEnContacto.assign(charcos.size(), false);
    for (std::size_t charcoIndex = 0; charcoIndex < charcos.size(); ++charcoIndex) {
        charcosVisibles[charcoIndex] = true;
        charcos[charcoIndex].setColor(sf::Color::White);
    }
    movementLocked = false;
    oxidadoPlaying = false;
    oxidadoIdle = false;
    oxidadoCounterPending = false;
    finalStagePending = false;
    finalStageActive = false;
    e3TimerActive = false;
}

void Game::update(float deltaTime) {
    const float movementSpeed = 200.f;

    if (!movementLocked) {
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
    }

    player.update(deltaTime);
    updateItems(deltaTime);
    if (e3TimerActive && animationIndex == 6 &&
        e3Clock.getElapsedTime().asSeconds() >= 5.f) {
        resetCycle();
        return;
    }
    if (oxidadoPlaying && player.hasCompletedCycle()) {
        oxidadoPlaying = false;
        if (finalStagePending) {
            finalStagePending = false;
            movementLocked = true;
            finalStageActive = true;
            animationIndex = 7;
            player.setAnimation("assets/images/S3.png", 8, 0, false);
            desmoronamientoMusic.stop();
            desmoronamientoMusic.play();
            setStageImage("assets/images/Muerto.jpg");
        } else {
            oxidadoIdle = true;
            player.setAnimation("assets/images/Oxidado.png", 3, 0, false);
            player.setScaleToHeight(152.f);
            player.showFrame(2);
            setStageImage("assets/images/Oxid.jpg");
            oxidadoCounterClock.restart();
            oxidadoCounterPending = true;
            movementLocked = false;
        }
        return;
    }
    if (animationIndex == 2 && player.hasCompletedCycle()) {
        if (finalStagePending) {
            finalStagePending = false;
            finalStageActive = true;
            movementLocked = true;
            animationIndex = 7;
            player.setAnimation("assets/images/S3.png", 8, 0, false);
            desmoronamientoMusic.stop();
            desmoronamientoMusic.play();
            setStageImage("assets/images/Muerto.jpg");
        } else {
            animationIndex = pendingAnimationIndex - 1;
            nextAnimation();
        }
        movementLocked = false;
        resetBatteryProgress();
    } else if (finalStageActive && animationIndex == 7 && player.hasCompletedCycle()) {
        animationIndex = 8;
        player.setAnimation("assets/images/S4.png", 8, 0, false);
        setStageImage("assets/images/Muerto.jpg");
        movementLocked = false;
    } else if (finalStageActive && animationIndex == 8 && player.hasCompletedCycle()) {
        finalStageActive = false;
        charcosRecolectados = 0;
        charcosEnContacto.assign(charcos.size(), false);
        for (std::size_t charcoIndex = 0; charcoIndex < charcos.size(); ++charcoIndex) {
            charcosVisibles[charcoIndex] = true;
            charcos[charcoIndex].setColor(sf::Color::White);
        }
        finalStagePending = false;
        oxidadoPlaying = false;
        oxidadoIdle = false;
        oxidadoCounterPending = false;
        animationIndex = 0;
        player.setAnimation("assets/images/S1.png", 8, 0, false);
        player.showFrame(0);
        setStageImage("assets/images/Et1.jpg");
        resetBatteryProgress();
        movementLocked = false;
    }
    if (oxidadoIdle && oxidadoCounterPending &&
        oxidadoCounterClock.getElapsedTime().asSeconds() >= 5.f) {
        oxidadoCounterPending = false;
        if (charcosRecolectados < 2) {
            ++charcosRecolectados;
        }
        if (charcosRecolectados == 2 && animationIndex < 7) {
            finalStagePending = false;
            finalStageActive = true;
            oxidadoIdle = false;
            movementLocked = true;
            animationIndex = 7;
            player.setAnimation("assets/images/S3.png", 8, 0, false);
            desmoronamientoMusic.stop();
            desmoronamientoMusic.play();
            resetBatteryProgress();
        }
    }
}

void Game::render() {
    window.clear(sf::Color::Black);
    window.draw(background);
    window.draw(stageImage);
    for (const sf::Sprite& item : baterias) {
        window.draw(item);
    }
    for (const sf::Sprite& item : libros) {
        window.draw(item);
    }
    for (const sf::Sprite& item : charcos) {
        window.draw(item);
    }
    window.draw(bateriasProgress);
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
