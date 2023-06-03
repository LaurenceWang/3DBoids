#pragma once

#include <vector>
#include "Family.hpp"
#include "Fish.hpp"
#include "p6/p6.h"

using ObstacleHandler = std::function<void(Obstacle const&)>;

class Boids {
private:
    std::vector<Fish> _fishpack;
    Family            _family;
    void              generateFish(int nbFish, int fam);
    int               getSizeFishpack();

public:
    Boids();
    Boids(const Boids& b) = default;
    explicit Boids(std::vector<Fish>& fish);
    Boids(int nbFish, int fam);

    std::vector<Fish> getFishPack() const;
    void              adjustBoidsNb(int nbFish);
    void              resizeBoids(float fishSize);
    void              runBoids(Params p, std::function<void(ObstacleHandler)> const& for_each_obstacle, std::vector<Food>& food);
};