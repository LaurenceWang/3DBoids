#pragma once

#include <vector>
#include "Family.hpp"
#include "Obstacle.hpp"
#include "Params.hpp"
#include "Speed.hpp"
#include "p6/p6.h"

class Fish {
private:
    glm::vec2 _pos;
    Family    _family;

public:
    Fish();
    Fish(const Fish& f) = default;
    Fish(glm::vec2 position, Family fam);
    ~Fish() = default;

    void              drawFish(p6::Context& context) const;
    void              updatePosition(glm::vec2 position);
    void              move();
    void              resize(float newSize);
    std::vector<Fish> getNeighbors(const std::vector<Fish>& boids, float& radius) const;
    glm::vec2         getPos() const;
    glm::vec2         separationForce(std::vector<Fish> const& boids, float& radius) const;
    glm::vec2         alignmentForce(std::vector<Fish> const& boids, float& radius) const;
    glm::vec2         cohesionForce(std::vector<Fish> const& boids, float& radius) const;
    void              applyForces(std::vector<Fish> const& boids, Params& p);
    void              applyObstacleForces(std::vector<Obstacle> const& obstacles);
    glm::vec2         obstacleForces(std::vector<Obstacle> const& obstacle);
};
