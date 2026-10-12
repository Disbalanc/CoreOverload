#pragma once

#include "engine/core/Types.h"
#include <string>
#include <vector>

namespace engine {

class Card {
public:
    // Конструктор
    Card(std::string id,
         std::string name,
         std::string description,
         CardType type,
         CardRarity rarity,
         CardTarget target,
         int cost);

    // Геттеры
    const std::string& getId() const { return m_id; }
    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }
    CardType getType() const { return m_type; }
    CardRarity getRarity() const { return m_rarity; }
    CardTarget getTarget() const { return m_target; }

    int getBaseCost() const { return m_baseCost; }
    int getCurrentCost() const { return m_currentCost; }

    // Управление стоимостью в бою (например, если карта временно дешевеет)
    void setCost(int newCost) { m_currentCost = std::max(0, newCost); }
    void resetCost() { m_currentCost = m_baseCost; }

    // Управление эффектами
    void addEffect(CardEffect effect);
    const std::vector<CardEffect>& getEffects() const { return m_effects; }

    // Улучшение карты (upgrade)
    bool isUpgraded() const { return m_upgraded; }
    void upgrade(const std::string& newName, int newCost, const std::vector<CardEffect>& newEffects);
    void upgradeFromTemplate(const std::string& newName,
                            const std::string& newDescription,
                            int newCost,
                            const std::vector<CardEffect>& newEffects);

private:
    std::string m_id;
    std::string m_name;
    std::string m_description;
    CardType m_type;
    CardRarity m_rarity;
    CardTarget m_target;

    int m_baseCost;
    int m_currentCost;
    bool m_upgraded = false;

    std::vector<CardEffect> m_effects;
};

} // namespace engine
