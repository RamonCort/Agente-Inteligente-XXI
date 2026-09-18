#pragma once

#include <SFML/System/Vector2.hpp>
#include <vector>

class AStar {
public:
    using Grid = std::vector<std::vector<int>>;

    void setGrid(const Grid& grid);
    std::vector<sf::Vector2i> findPath(sf::Vector2i start,
                                       sf::Vector2i target) const;
    sf::Vector2i nearestWalkable(sf::Vector2i cell) const;
    bool isInside(sf::Vector2i cell) const;
    bool isWalkable(sf::Vector2i cell) const;

private:
    Grid grid;
};