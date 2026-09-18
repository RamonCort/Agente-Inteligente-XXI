#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include "../src/CharacterAnimation.hpp"
#include "AStar.hpp"

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
    void setMovementAnimation(const std::string& imagePath, unsigned int frameCount, unsigned int firstFrame = 0, float height = 152.f);
    void playS2Animation();
    void setupItems();
    void updateItems(float deltaTime);
    void placeRandomly(sf::Sprite& item);
    void updateBatteryProgress();
    void resetBatteryProgress();
    void restoreStageIdle();
    bool touchesItemCenter(const sf::Sprite& item) const;
    void setStageImage(const std::string& imagePath);
    void resetCycle();
    void loadNavigationMap();
    void setDestination(const sf::Vector2f& destination);
    void updateAutomaticMovement(float deltaTime);
    sf::Vector2i worldToCell(const sf::Vector2f& position) const;
    sf::Vector2f cellToWorld(sf::Vector2i cell) const;
    sf::Vector2f getFeetPosition() const;
    void setFeetPosition(const sf::Vector2f& feetPosition);

    sf::RenderWindow window;
    sf::Texture backgroundTexture;
    sf::Sprite background;
    sf::Texture stageTexture;
    sf::Sprite stageImage;
    sf::Music fondoMusic;
    sf::Music laboratorioMusic;
    sf::Music evolucionMusic;
    sf::Music desmoronamientoMusic;
    sf::Texture bateriaTexture;
    sf::Texture bateriasProgressTexture;
    sf::Texture librosTexture;
    sf::Texture charcoTexture;
    std::vector<sf::Sprite> baterias;
    std::vector<sf::Sprite> libros;
    std::vector<sf::Sprite> charcos;
    std::vector<bool> charcosEnContacto;
    std::vector<sf::Clock> charcosRespawnClocks;
    std::vector<bool> charcosVisibles;
    sf::Sprite bateriasProgress;
    unsigned int bateriasRecolectadas = 0;
    unsigned int charcosRecolectados = 0;
    bool movementLocked = false;
    bool oxidadoPlaying = false;
    bool oxidadoIdle = false;
    bool finalStagePending = false;
    bool finalStageActive = false;
    unsigned int animationBeforeOxidado = 0;
    unsigned int pendingAnimationIndex = 4;
    bool oxidadoCounterPending = false;
    sf::Clock oxidadoCounterClock;
    sf::Clock e3Clock;
    bool e3TimerActive = false;
    CharacterAnimation player;
    sf::Clock clock;
    unsigned int animationIndex = 0;
    AStar navigator;
    AStar::Grid navigationGrid;
    std::vector<sf::Vector2i> currentPath;
    std::size_t pathIndex = 0;
    sf::Image navigationImage;
    static constexpr unsigned int navigationCellSize = 16;
};
