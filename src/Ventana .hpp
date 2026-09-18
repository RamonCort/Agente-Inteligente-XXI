#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>

class CharacterAnimation {
public:
    CharacterAnimation(const std::string& imagePath, unsigned int frameCount, float frameDuration, unsigned int newFirstFrame = 0, bool shouldLoop = false)
        : frameCount(frameCount), animationFrameCount(frameCount - newFirstFrame), firstFrame(newFirstFrame), loopAnimation(shouldLoop), frameDuration(frameDuration) {
        texture.loadFromFile(imagePath);
        sprite.setTexture(texture);
        sprite.setOrigin(
            static_cast<float>(texture.getSize().x / frameCount) / 2.f,
            static_cast<float>(texture.getSize().y) / 2.f
        );
        updateFrameRect();
    }

    void update(float deltaTime) {
        if (animationCompleted) {
            return;
        }

        elapsedTime += deltaTime;

        while (elapsedTime >= frameDuration) {
            elapsedTime -= frameDuration;
            if (currentFrame == animationFrameCount - 1) {
                if (loopAnimation) {
                    currentFrame = 0;
                } else {
                    animationCompleted = true;
                    elapsedTime = 0.f;
                }
            } else {
                ++currentFrame;
            }
            updateFrameRect();

            if (animationCompleted) {
                break;
            }
        }
    }

    bool hasCompletedCycle() const {
        return animationCompleted;
    }

    void showLastFrame() {
        showFrame(animationFrameCount - 1);
    }

    void showFrame(unsigned int frame) {
        currentFrame = frame < animationFrameCount ? frame : animationFrameCount - 1;
        animationCompleted = true;
        elapsedTime = 0.f;
        updateFrameRect();
    }

    void setScaleToHeight(float height) {
        const float scale = height / texture.getSize().y;
        sprite.setScale(scale, scale);
    }

    void setAnimation(const std::string& imagePath, unsigned int newFrameCount, unsigned int newFirstFrame = 0, bool shouldLoop = false) {
        texture.loadFromFile(imagePath);
        sprite.setTexture(texture);
        frameCount = newFrameCount;
        firstFrame = newFirstFrame;
        animationFrameCount = frameCount - firstFrame;
        loopAnimation = shouldLoop;
        sprite.setScale(1.f, 1.f);
        currentFrame = 0;
        animationCompleted = false;
        elapsedTime = 0.f;
        sprite.setOrigin(
            static_cast<float>(texture.getSize().x / frameCount) / 2.f,
            static_cast<float>(texture.getSize().y) / 2.f
        );
        updateFrameRect();
    }

    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
    }

    void move(float offsetX, float offsetY) {
        sprite.move(offsetX, offsetY);
    }

    const sf::Sprite& getSprite() const {
        return sprite;
    }

private:
    void updateFrameRect() {
        const float frameWidth = static_cast<float>(texture.getSize().x) / frameCount;
        const unsigned int displayedFrame = firstFrame + currentFrame;
        const unsigned int left = static_cast<unsigned int>(std::round(displayedFrame * frameWidth));
        const unsigned int right = static_cast<unsigned int>(std::round((displayedFrame + 1) * frameWidth));

        sprite.setTextureRect(sf::IntRect(
            static_cast<int>(left),
            0,
            static_cast<int>(right - left),
            static_cast<int>(texture.getSize().y)
        ));
    }

    sf::Texture texture;
    sf::Sprite sprite;
    unsigned int frameCount;
    unsigned int animationFrameCount;
    unsigned int firstFrame = 0;
    unsigned int currentFrame = 0;
    bool animationCompleted = false;
    bool loopAnimation = false;
    float frameDuration;
    float elapsedTime = 0.f;
};