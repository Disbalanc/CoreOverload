#include "engine/map/WorldMap.h"
#include <algorithm>

namespace engine {

void WorldMap::clear() {
    m_nodes.clear();
    m_floors.clear();
    m_currentNodeId = -1;
    m_currentFloor = -1;
}

void WorldMap::addNode(MapNode node) {
    int f = node.floor;
    if (f >= static_cast<int>(m_floors.size())) {
        m_floors.resize(f + 1);
    }
    m_floors[f].push_back(node.id);
    m_nodes[node.id] = std::move(node);
}

void WorldMap::unlockStartingNodes() {
    if (m_floors.empty() || m_floors[0].empty()) return;

    m_currentNodeId = -1;
    m_currentFloor = -1;

    // Сбрасываем доступность всех узлов
    for (auto& [id, node] : m_nodes) {
        node.available = false;
        node.visited = false;
    }

    // Делаем все узлы 0-го этажа доступными для старта
    for (int startId : m_floors[0]) {
        m_nodes[startId].available = true;
    }
}

bool WorldMap::moveToNode(int nodeId) {
    auto it = m_nodes.find(nodeId);
    if (it == m_nodes.end()) return false;

    MapNode& target = it->second;

    // Можно пойти только в доступный узел
    if (!target.available) return false;

    // Фиксируем перемещение
    m_currentNodeId = target.id;
    m_currentFloor = target.floor;
    target.visited = true;

    // Пересчитываем доступность узлов на следующем этаже
    updateAvailability();
    return true;
}

void WorldMap::updateAvailability() {
    // 1. Блокируем вообще все узлы
    for (auto& [id, node] : m_nodes) {
        node.available = false;
    }

    if (m_currentNodeId == -1) return;

    // 2. Открываем только те узлы, к которым ведёт текущий
    const MapNode& current = m_nodes[m_currentNodeId];
    for (int nextId : current.nextNodeIds) {
        auto nextIt = m_nodes.find(nextId);
        if (nextIt != m_nodes.end()) {
            nextIt->second.available = true;
        }
    }
}

const MapNode* WorldMap::getCurrentNode() const {
    if (m_currentNodeId == -1) return nullptr;
    auto it = m_nodes.find(m_currentNodeId);
    return (it != m_nodes.end()) ? &it->second : nullptr;
}

const MapNode* WorldMap::getNode(int nodeId) const {
    auto it = m_nodes.find(nodeId);
    return (it != m_nodes.end()) ? &it->second : nullptr;
}

std::vector<const MapNode*> WorldMap::getAvailableNodes() const {
    std::vector<const MapNode*> result;
    for (const auto& [id, node] : m_nodes) {
        if (node.available) {
            result.push_back(&node);
        }
    }
    return result;
}

bool WorldMap::isAtBoss() const {
    const MapNode* curr = getCurrentNode();
    return curr && (curr->type == NodeType::BossBattle);
}

} // namespace engine
