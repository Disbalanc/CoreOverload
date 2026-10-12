#pragma once
// ============================================================
// EnemyDatabase — реестр врагов, загружаемых из JSON
// ============================================================

#include "engine/entities/Enemy.h"
#include "engine/combat/CombatManager.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace engine {

// Шаблон врага
struct EnemyTemplate {
    std::string id;
    std::string name;
    int hpMin;
    int hpMax;
    std::vector<EnemyIntent> pattern;
};

// Шаблон встречи (группа врагов)
struct EncounterTemplate {
    std::string id;
    std::string type;              // "basic", "elite", "boss"
    std::vector<std::string> enemyIds; // ID врагов, которые будут в этой встрече
};

class EnemyDatabase {
public:
    static EnemyDatabase& instance();

    bool loadFromFile(const std::string& filePath);

    const EnemyTemplate* getEnemyTemplate(const std::string& id) const;
    const EncounterTemplate* getEncounterTemplate(const std::string& id) const;

    // Получить случайную встречу определённого типа
    const EncounterTemplate* getRandomEncounter(const std::string& type, class Random& rng) const;

    // Создать EnemyConfig для CombatManager из шаблона
    std::vector<EnemyConfig> buildEncounter(const std::string& encounterId, class Random& rng) const;

private:
    EnemyDatabase() = default;
    std::unordered_map<std::string, EnemyTemplate> m_enemies;
    std::unordered_map<std::string, EncounterTemplate> m_encounters;
};

} // namespace engine
