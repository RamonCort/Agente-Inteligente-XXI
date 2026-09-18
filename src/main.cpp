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
    loadNavigationMap();
    setupItems();
    setStageImage("assets/images/Et1.jpg");
    player.showFrame(0);
    player.setPosition(600.f, 450.f);
    for (std::size_t charcoIndex = 0; charcoIndex < charcos.size(); ++charcoIndex) {
        charcosEnContacto[charcoIndex] = touchesItemCenter(charcos[charcoIndex]);
    }
}

void Game::loadNavigationMap() {
    if (!navigationImage.loadFromFile("assets/images/A_Estrella.jpg")) {
        return;
    }

    const unsigned int columns = 1200 / navigationCellSize;
    const unsigned int rows = 900 / navigationCellSize;
    navigationGrid.assign(rows, std::vector<int>(columns, 1));
    const sf::Vector2u imageSize = navigationImage.getSize();

    for (unsigned int row = 0; row < rows; ++row) {
        for (unsigned int column = 0; column < columns; ++column) {
            unsigned int blackPixels = 0;
            unsigned int sampledPixels = 0;
            const unsigned int left = column * imageSize.x / columns;
            const unsigned int right = (column + 1) * imageSize.x / columns;
            const unsigned int top = row * imageSize.y / rows;
            const unsigned int bottom = (row + 1) * imageSize.y / rows;
            for (unsigned int y = top; y < bottom; y += 2) {
                for (unsigned int x = left; x < right; x += 2) {
                    ++sampledPixels;
                    const sf::Color pixel = navigationImage.getPixel(x, y);
                    if (pixel.r < 80 && pixel.g < 80 && pixel.b < 80) {
                        ++blackPixels;
                    }
                }
            }
            navigationGrid[row][column] = blackPixels * 5 > sampledPixels ? 0 : 1;
        }
    }

    // Se amplían las paredes para que el sprite no las atraviese con sus bordes.
    AStar::Grid safeGrid = navigationGrid;
    const int horizontalMargin = 1;
    const int verticalMargin = 2;
    for (int row = 0; row < static_cast<int>(navigationGrid.size()); ++row) {
        for (int column = 0; column < static_cast<int>(navigationGrid[row].size()); ++column) {
            for (int y = -verticalMargin; y <= verticalMargin; ++y) {
                for (int x = -horizontalMargin; x <= horizontalMargin; ++x) {
                    const sf::Vector2i neighbor(column + x, row + y);
                    const bool inside = neighbor.y >= 0 &&
                        neighbor.y < static_cast<int>(navigationGrid.size()) &&
                        neighbor.x >= 0 &&
                        neighbor.x < static_cast<int>(navigationGrid[neighbor.y].size());
                    if (!inside || navigationGrid[neighbor.y][neighbor.x] == 0) {
                        safeGrid[row][column] = 0;
                    }
                }
            }
        }
    }
    navigator.setGrid(safeGrid);
}

sf::Vector2i Game::worldToCell(const sf::Vector2f& position) const {
    return {static_cast<int>(position.x / navigationCellSize),
            static_cast<int>(position.y / navigationCellSize)};
}

sf::Vector2f Game::cellToWorld(sf::Vector2i cell) const {
    return {(cell.x + 0.5f) * navigationCellSize,
            (cell.y + 0.5f) * navigationCellSize};
}

sf::Vector2f Game::getFeetPosition() const {
    const sf::FloatRect bounds = player.getSprite().getGlobalBounds();
    return {bounds.left + bounds.width / 2.f, bounds.top + bounds.height};
}

void Game::setFeetPosition(const sf::Vector2f& feetPosition) {
    const sf::FloatRect bounds = player.getSprite().getGlobalBounds();
    player.setPosition(feetPosition.x, feetPosition.y - bounds.height / 2.f);
}

void Game::setDestination(const sf::Vector2f& destination) {
    if (movementLocked || navigationGrid.empty()) {
        return;
    }

    AStar::Grid routeGrid = navigationGrid;
    for (std::size_t charcoIndex = 0; charcoIndex < charcos.size(); ++charcoIndex) {
        if (!charcosVisibles[charcoIndex]) {
            continue;
        }
        const sf::FloatRect charcoBounds = charcos[charcoIndex].getGlobalBounds();
        for (std::size_t row = 0; row < routeGrid.size(); ++row) {
            for (std::size_t column = 0; column < routeGrid[row].size(); ++column) {
                const sf::Vector2f cellCenter = cellToWorld({
                    static_cast<int>(column), static_cast<int>(row)});
                if (charcoBounds.contains(cellCenter)) {
                    routeGrid[row][column] = 0;
                }
            }
        }
    }
    navigator.setGrid(routeGrid);

    sf::Vector2i start = navigator.nearestWalkable(worldToCell(getFeetPosition()));
    sf::Vector2i target = navigator.nearestWalkable(worldToCell(destination));
    if (start.x < 0 || target.x < 0) {
        currentPath.clear();
        return;
    }
    currentPath = navigator.findPath(start, target);
    pathIndex = currentPath.size() > 1 ? 1 : 0;
    if (!currentPath.empty()) {
        showDirection(cellToWorld(currentPath.back()).x >= getFeetPosition().x);
    }
}

void Game::updateAutomaticMovement(float deltaTime) {
    if (movementLocked || pathIndex >= currentPath.size()) {
        return;
    }
    const sf::Vector2f position = getFeetPosition();
    const sf::Vector2f target = cellToWorld(currentPath[pathIndex]);
    const sf::Vector2f difference = target - position;
    const float distance = std::sqrt(difference.x * difference.x + difference.y * difference.y);
    const float step = 200.f * deltaTime;
    if (distance <= step) {
        setFeetPosition(target);
        ++pathIndex;
        if (pathIndex == currentPath.size()) {
            currentPath.clear();
            player.setAnimation("assets/images/S1.png", 8, 0, false);
            player.showFrame(0);
        }
        return;
    }
    player.move(difference.x / distance * step, difference.y / distance * step);
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
    for (unsigned int attempt = 0; attempt < 1000; ++attempt) {
        const sf::Vector2f position(horizontal(generator), vertical(generator));
        const sf::Vector2i cell = worldToCell(position);
        if (cell.y >= 0 && cell.y < static_cast<int>(navigationGrid.size()) &&
            cell.x >= 0 && cell.x < static_cast<int>(navigationGrid[cell.y].size()) &&
            navigationGrid[cell.y][cell.x] == 1) {
            item.setPosition(position);
            return;
        }
    }
    for (std::size_t row = 0; row < navigationGrid.size(); ++row) {
        for (std::size_t column = 0; column < navigationGrid[row].size(); ++column) {
            if (navigationGrid[row][column] == 1) {
                item.setPosition(cellToWorld({static_cast<int>(column), static_cast<int>(row)}));
                return;
            }
        }
    }
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
        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left) {
            setDestination(window.mapPixelToCoords(
                {event.mouseButton.x, event.mouseButton.y}));
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
        updateAutomaticMovement(deltaTime);
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
