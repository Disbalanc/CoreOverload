#include "engine/entities/ArtifactDatabase.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace engine {

using json = nlohmann::json;

static CardRarity parseRarity(const std::string& s) {
    if (s == "common")    return CardRarity::Common;
    if (s == "uncommon")  return CardRarity::Uncommon;
    if (s == "rare")      return CardRarity::Rare;
    if (s == "legendary") return CardRarity::Legendary;
    return CardRarity::Common;
}

static ArtifactTrigger parseTrigger(const std::string& s) {
    if (s == "on_combat_start")    return ArtifactTrigger::OnCombatStart;
    if (s == "on_turn_start")      return ArtifactTrigger::OnTurnStart;
    if (s == "on_card_played")     return ArtifactTrigger::OnCardPlayed;
    if (s == "on_damage_received") return ArtifactTrigger::OnDamageReceived;
    if (s == "on_enemy_died")      return ArtifactTrigger::OnEnemyDied;
    return ArtifactTrigger::Passive;
}

static ArtifactEffectType parseEffectType(const std::string& s) {
    if (s == "gain_block")           return ArtifactEffectType::GainBlock;
    if (s == "gain_energy")          return ArtifactEffectType::GainEnergy;
    if (s == "heal_hp")              return ArtifactEffectType::HealHp;
    if (s == "draw_cards")           return ArtifactEffectType::DrawCards;
    if (s == "increase_max_energy")  return ArtifactEffectType::IncreaseMaxEnergy;
    if (s == "increase_max_hp")      return ArtifactEffectType::IncreaseMaxHp;
    if (s == "reduce_self_damage")   return ArtifactEffectType::ReduceSelfDamage;
    if (s == "extra_draw_per_turn")  return ArtifactEffectType::ExtraDrawPerTurn;
    return ArtifactEffectType::GainBlock;
}

ArtifactDatabase& ArtifactDatabase::instance() {
    static ArtifactDatabase inst;
    return inst;
}

bool ArtifactDatabase::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "ArtifactDatabase: не удалось открыть " << filePath << "\n";
        return false;
    }

    try {
        json data;
        file >> data;

        for (const auto& item : data["artifacts"]) {
            ArtifactTemplate t;
            t.id = item.at("id").get<std::string>();
            t.name = item.at("name").get<std::string>();
            t.description = item.value("description", "");
            t.rarity = parseRarity(item.value("rarity", "common"));
            t.trigger = parseTrigger(item.value("trigger", "passive"));

            if (item.contains("effects")) {
                for (const auto& eff : item["effects"]) {
                    ArtifactEffect e;
                    e.type = parseEffectType(eff.value("type", "gain_block"));
                    e.value = eff.value("value", 0);
                    t.effects.push_back(e);
                }
            }

            m_templates[t.id] = t;
        }

        std::cout << "ArtifactDatabase: загружено " << m_templates.size() << " артефактов\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "ArtifactDatabase JSON ошибка: " << e.what() << "\n";
        return false;
    }
}

const ArtifactTemplate* ArtifactDatabase::getTemplate(const std::string& id) const {
    auto it = m_templates.find(id);
    return (it != m_templates.end()) ? &it->second : nullptr;
}

std::vector<std::string> ArtifactDatabase::getAllIds() const {
    std::vector<std::string> result;
    for (const auto& [id, t] : m_templates) {
        result.push_back(id);
    }
    return result;
}

std::vector<std::string> ArtifactDatabase::getIdsByRarity(CardRarity rarity) const {
    std::vector<std::string> result;
    for (const auto& [id, t] : m_templates) {
        if (t.rarity == rarity) {
            result.push_back(id);
        }
    }
    return result;
}

std::unique_ptr<Artifact> ArtifactDatabase::createArtifact(const std::string& id) const {
    const ArtifactTemplate* t = getTemplate(id);
    if (!t) return nullptr;
    return std::make_unique<Artifact>(*t);
}

} // namespace engine
