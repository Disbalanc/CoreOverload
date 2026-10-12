#include "engine/map/MapGenerator.h"
#include <algorithm>
#include <cmath>

namespace engine {

WorldMap MapGenerator::generate(const MapConfig& config, Random& rng) {
    WorldMap map;
    int nextNodeId = 1;

    // Временная структура: [floor][lane] -> nodeId (-1 если узла в этой ячейке нет)
    std::vector<std::vector<int>> grid(config.totalFloors,
                                       std::vector<int>(config.lanesCount, -1));

    // ── 1. СОЗДАНИЕ УЗЛОВ В СЕТКЕ ─────────────────────────
    for (int f = 0; f < config.totalFloors; ++f) {

        // Последний этаж — ВСЕГДА один узел Босса по центру
        if (f == config.totalFloors - 1) {
            MapNode bossNode;
            bossNode.id = nextNodeId++;
            bossNode.type = NodeType::BossBattle;
            bossNode.floor = f;
            bossNode.lane = config.lanesCount / 2;
            bossNode.encounterId = "boss_lithography";

            grid[f][bossNode.lane] = bossNode.id;
            map.addNode(bossNode);
            continue;
        }

        // Предпоследний этаж — ВСЕГДА терминалы отдыха (починка)
        if (f == config.totalFloors - 2) {
            for (int l = 0; l < config.lanesCount; ++l) {
                // Создаем 2-3 терминала отдыха
                if (l == 1 || l == config.lanesCount - 2 || config.lanesCount <= 2) {
                    MapNode restNode;
                    restNode.id = nextNodeId++;
                    restNode.type = NodeType::Rest;
                    restNode.floor = f;
                    restNode.lane = l;

                    grid[f][l] = restNode.id;
                    map.addNode(restNode);
                }
            }
            continue;
        }

        // Обычные этажи: генерируем от 2 до lanesCount узлов
        int activeLanes = rng.rollInt(2, config.lanesCount);
        std::vector<int> laneIndices(config.lanesCount);
        for (int i = 0; i < config.lanesCount; ++i) laneIndices[i] = i;
        rng.shuffle(laneIndices);

        for (int i = 0; i < activeLanes; ++i) {
            int l = laneIndices[i];

            MapNode node;
            node.id = nextNodeId++;
            node.floor = f;
            node.lane = l;
            node.type = rollNodeType(f, config.totalFloors, rng);

            grid[f][l] = node.id;
            map.addNode(node);
        }
    }

    // ── 2. СОЕДИНЕНИЕ УЗЛОВ РЁБРАМИ (ШИНАМИ ДАННЫХ) ────────
    for (int f = 0; f < config.totalFloors - 1; ++f) {
        for (int l = 0; l < config.lanesCount; ++l) {
            int currentId = grid[f][l];
            if (currentId == -1) continue;

            auto* currentInMap = const_cast<MapNode*>(map.getNode(currentId));
            if (!currentInMap) continue;

            // Если следующий этаж — БОСС, соединяем напрямую с боссом
            if (f == config.totalFloors - 2) {
                int bossId = grid[f + 1][config.lanesCount / 2];
                if (bossId != -1) {
                    currentInMap->nextNodeIds.push_back(bossId);
                }
                continue;
            }

            // Ищем кандидатов на этаже (f + 1):
            // Можно соединяться с соседними линиями: lane - 1, lane, lane + 1
            std::vector<int> candidates;
            for (int nextL = std::max(0, l - 1); nextL <= std::min(config.lanesCount - 1, l + 1); ++nextL) {
                int nextId = grid[f + 1][nextL];
                if (nextId != -1) {
                    candidates.push_back(nextId);
                }
            }

            // Если прямого соединения нет (из-за пустоты), ищем ЛЮБОЙ ближайший узел
            if (candidates.empty()) {
                for (int nextL = 0; nextL < config.lanesCount; ++nextL) {
                    int nextId = grid[f + 1][nextL];
                    if (nextId != -1) {
                        candidates.push_back(nextId);
                        break;
                    }
                }
            }

            // Добавляем связи
            for (int cId : candidates) {
                currentInMap->nextNodeIds.push_back(cId);
            }
        }
    }

    // ── 3. УСТРАНЕНИЕ СИРОТСКИХ УЗЛОВ ──────────────────────
    // Проверяем, чтобы каждый узел на этаже > 0 имел хотя бы одного входящего предка
    for (int f = 1; f < config.totalFloors; ++f) {
        for (int l = 0; l < config.lanesCount; ++l) {
            int targetId = grid[f][l];
            if (targetId == -1) continue;

            // Проверяем, ведёт ли кто-то сюда с этажа f-1
            bool hasIncoming = false;
            for (int prevL = 0; prevL < config.lanesCount; ++prevL) {
                int prevId = grid[f - 1][prevL];
                if (prevId == -1) continue;

                const MapNode* prevNode = map.getNode(prevId);
                if (prevNode && std::find(prevNode->nextNodeIds.begin(),
                                          prevNode->nextNodeIds.end(),
                                          targetId) != prevNode->nextNodeIds.end()) {
                    hasIncoming = true;
                    break;
                }
            }

            // Если узел остался без входящей связи, принудительно цепляем его к ближайшему снизу
            if (!hasIncoming) {
                for (int prevL = 0; prevL < config.lanesCount; ++prevL) {
                    int prevId = grid[f - 1][prevL];
                    if (prevId != -1) {
                        auto* prevNode = const_cast<MapNode*>(map.getNode(prevId));
                        prevNode->nextNodeIds.push_back(targetId);
                        break;
                    }
                }
            }
        }
    }

    // Открываем стартовые узлы
    map.unlockStartingNodes();
    return map;
}

NodeType MapGenerator::rollNodeType(int floor, int totalFloors, Random& rng) {
    // 0-й этаж — всегда базовые бои
    if (floor == 0) {
        return NodeType::Battle;
    }

    // Rest на 2-м и на предпоследнем-1 этажах с высоким шансом
    if (floor == 2 || floor == totalFloors - 3) {
        if (rng.rollChance(0.5f)) return NodeType::Rest;
    }

    int roll = rng.rollInt(1, 100);

    // 40% Battle, 20% Event, 15% Rest, 15% Shop, 10% Elite (с 3-го этажа)
    if (floor >= 3 && roll <= 10) return NodeType::EliteBattle;
    if (roll <= 25) return NodeType::Event;
    if (roll <= 40) return NodeType::Shop;
    if (roll <= 55) return NodeType::Rest;  // Rest стал заметно чаще

    return NodeType::Battle;
}

} // namespace engine
