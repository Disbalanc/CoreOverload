#pragma once

#include "engine/map/MapNode.h"
#include <vector>
#include <unordered_map>

namespace engine {

class WorldMap {
public:
    WorldMap() = default;

    void clear();
    void addNode(MapNode node);

    // ── Навигация ─────────────────────────────────────────
    bool moveToNode(int nodeId);
    const MapNode* getCurrentNode() const;
    const MapNode* getNode(int nodeId) const;

    // Список узлов, доступных для выбора прямо сейчас
    std::vector<const MapNode*> getAvailableNodes() const;

    // Все этажи для отрисовки UI
    const std::vector<std::vector<int>>& getFloors() const { return m_floors; }
    const std::unordered_map<int, MapNode>& getAllNodes() const { return m_nodes; }

    int getCurrentFloor() const { return m_currentFloor; }
    int getTotalFloors() const { return static_cast<int>(m_floors.size()); }
    bool isAtBoss() const;

    // Инициализация стартовой доступности (вызывается в начале забега)
    void unlockStartingNodes();

private:
    void updateAvailability();

    int m_currentNodeId = -1;
    int m_currentFloor = -1;

    // m_floors[floorIndex] = список node ID на этом этаже
    std::vector<std::vector<int>> m_floors;

    // Быстрый доступ к узлу по ID
    std::unordered_map<int, MapNode> m_nodes;
};

} // namespace engine
