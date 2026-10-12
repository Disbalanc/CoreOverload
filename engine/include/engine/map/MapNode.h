#pragma once
// ============================================================
// MapNode.h — Узел сетевой топологии завода
// ============================================================

#include "engine/core/Types.h"
#include <vector>
#include <string>

namespace engine {

struct MapNode {
    int id = 0;                     // Уникальный ID узла
    NodeType type = NodeType::Battle;
    int floor = 0;                  // Номер слоя/этажа (0..N)
    int lane = 0;                   // Позиция по горизонтали (дорожка 0..M)

    std::vector<int> nextNodeIds;   // ID узлов следующего этажа, куда есть путь

    bool visited = false;           // Был ли игрок уже здесь
    bool available = false;         // Доступен ли узел для перехода прямо сейчас

    std::string encounterId = "";   // ID набора врагов или события (для загрузки из базы)

    // Получить киберпанк-название для UI
    std::string getDisplayName() const {
        switch (type) {
            case NodeType::Battle:       return "Цех сборки";
            case NodeType::EliteBattle:  return "Серверный IDS-узел";
            case NodeType::Rest:         return "Терминал отладки";
            case NodeType::Event:        return "Неизвестный порт";
            case NodeType::Shop:         return "Чёрный рынок кремния";
            case NodeType::BossBattle:   return "ЯДРО ЛИТОГРАФИИ";
            default:                     return "Узел данных";
        }
    }
};

} // namespace engine
