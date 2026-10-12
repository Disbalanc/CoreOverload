#pragma once
// ============================================================
// CardDatabase.h — Реестр всех карт игры (загрузка из JSON)
// ============================================================

#include "engine/cards/Card.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace engine {

// Шаблон карты (данные для создания экземпляров)
struct CardTemplate {
    std::string id;
    std::string name;
    std::string description;
    std::string upgradedName;        // Имя после апгрейда (например, "DDoS-удар+")
    std::string upgradedDescription;
    CardType type;
    CardRarity rarity;
    CardTarget target;
    int cost;
    int upgradedCost;                // -1 если не меняется
    std::vector<CardEffect> effects;
    std::vector<CardEffect> upgradedEffects;
};

class CardDatabase {
public:
    static CardDatabase& instance();

    // Загрузить все карты из JSON-файла
    bool loadFromFile(const std::string& filePath);

    // Создать новый экземпляр карты по ID
    std::unique_ptr<Card> createCard(const std::string& templateId, int uniqueSuffix = 0) const;

    // Получить шаблон карты (для просмотра)
    const CardTemplate* getTemplate(const std::string& id) const;

    // Получить все карты определённой редкости/типа (для наград и магазина)
    std::vector<std::string> getIdsByRarity(CardRarity rarity) const;
    std::vector<std::string> getAllIds() const;

    size_t getTemplateCount() const { return m_templates.size(); }

private:
    CardDatabase() = default;
    std::unordered_map<std::string, CardTemplate> m_templates;
};

} // namespace engine
