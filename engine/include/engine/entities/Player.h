#pragma once

#include "engine/entities/CombatEntity.h"
#include "engine/cards/Deck.h"
#include "engine/entities/Artifact.h"
#include <memory>

namespace engine {

class Player : public CombatEntity {
public:
    Player(int maxHp, int maxEnergy);

    // ── Управление Энергией (ЦПУ) ─────────────────────────
    int getEnergy() const { return m_energy; }
    int getMaxEnergy() const { return m_maxEnergy; }

    void gainEnergy(int amount);
    bool useEnergy(int amount);
    void resetEnergy();

    // Доступ к колоде
    Deck& getDeck() { return m_deck; }
    const Deck& getDeck() const { return m_deck; }

    // ── Игровой цикл хода ─────────────────────────────────
    void startTurn(Random& rng); // Сброс файрвола, восстановление ЦПУ, добор карт
    void endTurn();              // Сброс руки

    void addArtifact(std::unique_ptr<Artifact> artifact);
    const std::vector<std::unique_ptr<Artifact>>& getArtifacts() const { return m_artifacts; }
    int getArtifactBonus(ArtifactEffectType type) const; // Сумма пассивных эффектов

private:
    int m_maxEnergy;
    int m_energy;
    Deck m_deck;
    std::vector<std::unique_ptr<Artifact>> m_artifacts;
};

} // namespace engine
