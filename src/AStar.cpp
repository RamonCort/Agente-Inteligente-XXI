#include "../include/AStar.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

void AStar::setGrid(const Grid& newGrid) {
    grid = newGrid;
}

bool AStar::isInside(sf::Vector2i cell) const {
    return cell.y >= 0 && cell.y < static_cast<int>(grid.size()) &&
        cell.x >= 0 && cell.x < static_cast<int>(grid[cell.y].size());
}

bool AStar::isWalkable(sf::Vector2i cell) const {
    return isInside(cell) && grid[cell.y][cell.x] == 1;
}

sf::Vector2i AStar::nearestWalkable(sf::Vector2i cell) const {
    if (isWalkable(cell)) {
        return cell;
    }

    if (grid.empty()) {
        return {-1, -1};
    }

    std::queue<sf::Vector2i> pending;
    std::vector<std::vector<bool>> visited(grid.size());
    for (std::size_t row = 0; row < grid.size(); ++row) {
        visited[row].assign(grid[row].size(), false);
    }
    if (!isInside(cell)) {
        cell.x = std::clamp(cell.x, 0, static_cast<int>(grid[0].size()) - 1);
        cell.y = std::clamp(cell.y, 0, static_cast<int>(grid.size()) - 1);
    }
    pending.push(cell);
    visited[cell.y][cell.x] = true;

    const int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    while (!pending.empty()) {
        const sf::Vector2i current = pending.front();
        pending.pop();
        for (const auto& direction : directions) {
            const sf::Vector2i next(current.x + direction[0], current.y + direction[1]);
            if (!isInside(next) || visited[next.y][next.x]) {
                continue;
            }
            if (isWalkable(next)) {
                return next;
            }
            visited[next.y][next.x] = true;
            pending.push(next);
        }
    }
    return {-1, -1};
}

std::vector<sf::Vector2i> AStar::findPath(sf::Vector2i start,
                                          sf::Vector2i target) const {
    if (!isWalkable(start) || !isWalkable(target)) {
        return {};
    }

    const int width = static_cast<int>(grid[0].size());
    const int height = static_cast<int>(grid.size());
    const int nodeCount = width * height;
    const int invalid = -1;
    const float infinity = std::numeric_limits<float>::infinity();
    std::vector<float> cost(nodeCount, infinity);
    std::vector<int> parent(nodeCount, invalid);
    auto index = [width](sf::Vector2i cell) { return cell.y * width + cell.x; };
    auto heuristic = [target](sf::Vector2i cell) {
        return static_cast<float>(std::abs(target.x - cell.x) + std::abs(target.y - cell.y));
    };
    struct Entry {
        float priority;
        sf::Vector2i cell;
        bool operator<(const Entry& other) const { return priority > other.priority; }
    };

    std::priority_queue<Entry> open;
    cost[index(start)] = 0.f;
    open.push({heuristic(start), start});
    const int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    while (!open.empty()) {
        const sf::Vector2i current = open.top().cell;
        open.pop();
        if (current == target) {
            break;
        }
        for (const auto& direction : directions) {
            const sf::Vector2i next(current.x + direction[0], current.y + direction[1]);
            if (!isWalkable(next)) {
                continue;
            }
            const int nextIndex = index(next);
            const float nextCost = cost[index(current)] + 1.f;
            if (nextCost < cost[nextIndex]) {
                cost[nextIndex] = nextCost;
                parent[nextIndex] = index(current);
                open.push({nextCost + heuristic(next), next});
            }
        }
    }

    if (start != target && parent[index(target)] == invalid) {
        return {};
    }
    std::vector<sf::Vector2i> path;
    for (int current = index(target); current != invalid; current = parent[current]) {
        path.push_back({current % width, current / width});
    }
    std::reverse(path.begin(), path.end());
    return path;
}