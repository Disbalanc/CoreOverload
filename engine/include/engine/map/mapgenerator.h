#pragma once

#include "engine/map/WorldMap.h"
#include "engine/core/Random.h"

namespace engine {

struct MapConfig {
    int totalFloors = 10;   // Количество этажей в акте (включая босса)
    int lanesCount = 4;     // Максимальная ширина карты (дорожки)
    int actNumber = 1;      // Номер акта
};

class MapGenerator {
public:
    static WorldMap generate(const MapConfig& config, Random& rng);

private:
    static NodeType rollNodeType(int floor, int totalFloors, Random& rng);
};

} // namespace engine
