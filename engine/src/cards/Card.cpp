#include "engine/cards/Card.h"

namespace engine {

Card::Card(std::string id,
           std::string name,
           std::string description,
           CardType type,
           CardRarity rarity,
           CardTarget target,
           int cost)
    : m_id(std::move(id))
    , m_name(std::move(name))
    , m_description(std::move(description))
    , m_type(type)
    , m_rarity(rarity)
    , m_target(target)
    , m_baseCost(cost)
    , m_currentCost(cost)
{
}

void Card::addEffect(CardEffect effect) {
    m_effects.push_back(effect);
}

void Card::upgrade(const std::string& newName, int newCost, const std::vector<CardEffect>& newEffects) {
    if (m_upgraded) return; // Карту можно улучшить только один раз

    m_upgraded = true;
    m_name = newName;
    m_baseCost = newCost;
    m_currentCost = newCost;
    m_effects = newEffects;
}

void Card::upgradeFromTemplate(const std::string& newName,
                              const std::string& newDescription,
                              int newCost,
                              const std::vector<CardEffect>& newEffects)
{
    if (m_upgraded) return;
    m_upgraded = true;
    m_name = newName + "+";  // Знак "+" показывает улучшение
    m_description = newDescription;
    if (newCost >= 0) {
        m_baseCost = newCost;
        m_currentCost = newCost;
    }
    if (!newEffects.empty()) {
        m_effects = newEffects;
    }
}

} // namespace engine
