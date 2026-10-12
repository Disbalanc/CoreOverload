#include "engine/cards/CardDatabase.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace engine {

using json = nlohmann::json;

// Хелперы для парсинга enum'ов
static CardType parseCardType(const std::string& s) {
    if (s == "attack") return CardType::Attack;
    if (s == "skill")  return CardType::Skill;
    if (s == "power")  return CardType::Power;
    if (s == "curse")  return CardType::Curse;
    return CardType::Skill;
}

static CardRarity parseCardRarity(const std::string& s) {
    if (s == "common")    return CardRarity::Common;
    if (s == "uncommon")  return CardRarity::Uncommon;
    if (s == "rare")      return CardRarity::Rare;
    if (s == "legendary") return CardRarity::Legendary;
    if (s == "curse")     return CardRarity::Curse;
    return CardRarity::Common;
}

static CardTarget parseCardTarget(const std::string& s) {
    if (s == "single_enemy") return CardTarget::SingleEnemy;
    if (s == "all_enemies")  return CardTarget::AllEnemies;
    if (s == "self")         return CardTarget::Self;
    if (s == "none")         return CardTarget::None;
    return CardTarget::None;
}

static CardEffectType parseEffectType(const std::string& s) {
    if (s == "damage")              return CardEffectType::Damage;
    if (s == "damage_ignore_block") return CardEffectType::DamageIgnoreBlock;
    if (s == "block")               return CardEffectType::Block;
    if (s == "gain_energy")         return CardEffectType::GainEnergy;
    if (s == "draw_cards")          return CardEffectType::DrawCards;
    if (s == "apply_status")        return CardEffectType::ApplyStatus;
    if (s == "self_damage")         return CardEffectType::SelfDamage;
    return CardEffectType::Damage;
}

static std::vector<CardEffect> parseEffects(const json& arr) {
    std::vector<CardEffect> effects;
    for (const auto& e : arr) {
        CardEffect eff;
        eff.type = parseEffectType(e.value("type", "damage"));
        eff.value = e.value("value", 0);
        eff.statusId = e.value("status_id", "");
        effects.push_back(eff);
    }
    return effects;
}

CardDatabase& CardDatabase::instance() {
    static CardDatabase inst;
    return inst;
}

bool CardDatabase::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "CardDatabase: не удалось открыть " << filePath << "\n";
        return false;
    }

    try {
        json data;
        file >> data;

        for (const auto& item : data["cards"]) {
            CardTemplate tmpl;
            tmpl.id = item.at("id").get<std::string>();
            tmpl.name = item.at("name").get<std::string>();
            tmpl.description = item.value("description", "");
            tmpl.upgradedName = item.value("upgraded_name", tmpl.name + "+");
            tmpl.upgradedDescription = item.value("upgraded_description", tmpl.description);
            tmpl.type = parseCardType(item.value("type", "skill"));
            tmpl.rarity = parseCardRarity(item.value("rarity", "common"));
            tmpl.target = parseCardTarget(item.value("target", "none"));
            tmpl.cost = item.value("cost", 1);
            tmpl.upgradedCost = item.value("upgraded_cost", -1);

            if (item.contains("effects")) {
                tmpl.effects = parseEffects(item["effects"]);
            }
            if (item.contains("upgraded_effects")) {
                tmpl.upgradedEffects = parseEffects(item["upgraded_effects"]);
            } else {
                tmpl.upgradedEffects = tmpl.effects; // По умолчанию те же эффекты
            }

            m_templates[tmpl.id] = tmpl;
        }

        std::cout << "CardDatabase: загружено " << m_templates.size() << " карт из " << filePath << "\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "CardDatabase JSON ошибка: " << e.what() << "\n";
        return false;
    }
}

std::unique_ptr<Card> CardDatabase::createCard(const std::string& templateId, int uniqueSuffix) const {
    auto it = m_templates.find(templateId);
    if (it == m_templates.end()) {
        std::cerr << "CardDatabase: шаблон не найден: " << templateId << "\n";
        return nullptr;
    }

    const CardTemplate& t = it->second;
    std::string uniqueId = t.id + "_" + std::to_string(uniqueSuffix);

    auto card = std::make_unique<Card>(
        uniqueId, t.name, t.description,
        t.type, t.rarity, t.target, t.cost
    );

    for (const auto& eff : t.effects) {
        card->addEffect(eff);
    }

    return card;
}

const CardTemplate* CardDatabase::getTemplate(const std::string& id) const {
    auto it = m_templates.find(id);
    return (it != m_templates.end()) ? &it->second : nullptr;
}

std::vector<std::string> CardDatabase::getIdsByRarity(CardRarity rarity) const {
    std::vector<std::string> result;
    for (const auto& [id, tmpl] : m_templates) {
        if (tmpl.rarity == rarity) {
            result.push_back(id);
        }
    }
    return result;
}

std::vector<std::string> CardDatabase::getAllIds() const {
    std::vector<std::string> result;
    for (const auto& [id, tmpl] : m_templates) {
        result.push_back(id);
    }
    return result;
}

} // namespace engine
