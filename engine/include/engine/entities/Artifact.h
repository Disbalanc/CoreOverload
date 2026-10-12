#pragma once
// ============================================================
// Artifact — пассивный предмет игрока (аналог реликвий в StS)
// ============================================================

#include "engine/core/Types.h"
#include <string>
#include <vector>

namespace engine {

enum class ArtifactTrigger {
    OnCombatStart,     // Срабатывает в начале боя
    OnTurnStart,       // Срабатывает в начале хода игрока
    OnCardPlayed,      // Срабатывает при розыгрыше любой карты
    OnDamageReceived,  // При получении урона
    OnEnemyDied,       // При убийстве врага
    Passive            // Пассивный эффект (не триггерится, просто есть)
};

enum class ArtifactEffectType {
    GainBlock,         // Дать файрвол
    GainEnergy,        // Дать энергию
    HealHp,            // Восстановить HP
    DrawCards,         // Взять карты
    IncreaseMaxEnergy, // +1 к max энергии навсегда
    IncreaseMaxHp,     // +N к max HP
    ReduceSelfDamage,  // Уменьшить урон от разгона
    ExtraDrawPerTurn   // +1 карта каждый ход (стакается)
};

struct ArtifactEffect {
    ArtifactEffectType type;
    int value = 0;
};

struct ArtifactTemplate {
    std::string id;
    std::string name;
    std::string description;
    CardRarity rarity;
    ArtifactTrigger trigger;
    std::vector<ArtifactEffect> effects;
};

class Artifact {
public:
    Artifact(const ArtifactTemplate& tmpl);

    const std::string& getId() const { return m_data.id; }
    const std::string& getName() const { return m_data.name; }
    const std::string& getDescription() const { return m_data.description; }
    CardRarity getRarity() const { return m_data.rarity; }
    ArtifactTrigger getTrigger() const { return m_data.trigger; }
    const std::vector<ArtifactEffect>& getEffects() const { return m_data.effects; }

private:
    ArtifactTemplate m_data;
};

} // namespace engine
