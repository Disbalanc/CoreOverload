#include "engine/entities/EnemyDatabase.h"
#include "engine/core/Random.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace engine {

using json = nlohmann::json;

static IntentType parseIntentType(const std::string& s) {
    if (s == "attack")             return IntentType::Attack;
    if (s == "defend")             return IntentType::Defend;
    if (s == "buff")               return IntentType::Buff;
    if (s == "debuff")             return IntentType::Debuff;
    if (s == "attack_and_defend")  return IntentType::AttackAndDefend;
    if (s == "summon")             return IntentType::Summon;
    return IntentType::Unknown;
}

EnemyDatabase& EnemyDatabase::instance() {
    static EnemyDatabase inst;
    return inst;
}

bool EnemyDatabase::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "EnemyDatabase: не удалось открыть " << filePath << "\n";
        return false;
    }

    try {
        json data;
        file >> data;

        // Враги
        if (data.contains("enemies")) {
            for (const auto& item : data["enemies"]) {
                EnemyTemplate t;
                t.id = item.at("id").get<std::string>();
                t.name = item.at("name").get<std::string>();
                t.hpMin = item.value("hp_min", 20);
                t.hpMax = item.value("hp_max", 25);

                for (const auto& intentItem : item["pattern"]) {
                    EnemyIntent intent;
                    intent.type = parseIntentType(intentItem.value("type", "attack"));
                    intent.value = intentItem.value("value", 0);
                    intent.hitCount = intentItem.value("hit_count", 1);
                    t.pattern.push_back(intent);
                }

                m_enemies[t.id] = t;
            }
        }

        // Встречи
        if (data.contains("encounters")) {
            for (const auto& item : data["encounters"]) {
                EncounterTemplate t;
                t.id = item.at("id").get<std::string>();
                t.type = item.value("type", "basic");
                for (const auto& e : item["enemies"]) {
                    t.enemyIds.push_back(e.get<std::string>());
                }
                m_encounters[t.id] = t;
            }
        }

        std::cout << "EnemyDatabase: загружено " << m_enemies.size()
                  << " врагов и " << m_encounters.size() << " встреч\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "EnemyDatabase JSON ошибка: " << e.what() << "\n";
        return false;
    }
}

const EnemyTemplate* EnemyDatabase::getEnemyTemplate(const std::string& id) const {
    auto it = m_enemies.find(id);
    return (it != m_enemies.end()) ? &it->second : nullptr;
}

const EncounterTemplate* EnemyDatabase::getEncounterTemplate(const std::string& id) const {
    auto it = m_encounters.find(id);
    return (it != m_encounters.end()) ? &it->second : nullptr;
}

const EncounterTemplate* EnemyDatabase::getRandomEncounter(const std::string& type, Random& rng) const {
    std::vector<const EncounterTemplate*> matching;
    for (const auto& [id, e] : m_encounters) {
        if (e.type == type) {
            matching.push_back(&e);
        }
    }
    if (matching.empty()) return nullptr;
    int idx = rng.rollInt(0, static_cast<int>(matching.size()) - 1);
    return matching[idx];
}

std::vector<EnemyConfig> EnemyDatabase::buildEncounter(const std::string& encounterId, Random& rng) const {
    std::vector<EnemyConfig> result;
    const EncounterTemplate* enc = getEncounterTemplate(encounterId);
    if (!enc) return result;

    int idx = 0;
    for (const std::string& enemyId : enc->enemyIds) {
        const EnemyTemplate* et = getEnemyTemplate(enemyId);
        if (!et) continue;

        EnemyConfig cfg;
        cfg.id = enemyId + "_" + std::to_string(idx++);
        cfg.name = et->name;
        cfg.hp = rng.rollInt(et->hpMin, et->hpMax);
        cfg.pattern = et->pattern;
        result.push_back(cfg);
    }
    return result;
}

} // namespace engine
