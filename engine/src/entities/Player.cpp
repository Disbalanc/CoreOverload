#include "engine/entities/Player.h"

namespace engine {

Player::Player(int maxHp, int maxEnergy)
    : CombatEntity(maxHp)
    , m_maxEnergy(maxEnergy)
    , m_energy(maxEnergy)
{
}

void Player::gainEnergy(int amount) {
    if (amount <= 0) return;
    m_energy = std::min(m_maxEnergy + 5, m_energy + amount); // Ограничим оверклокинг +5 от макса
}

bool Player::useEnergy(int amount) {
    if (amount < 0) return false;
    if (m_energy >= amount) {
        m_energy -= amount;
        return true;
    }
    return false; // Не хватает частоты процессора!
}

void Player::resetEnergy() {
    m_energy = m_maxEnergy;
}

void Player::startTurn(Random& rng) {
    resetBlock();       // Файрвол обнуляется в начале нашего хода
    resetEnergy();      // ЦПУ восстанавливается
    m_deck.drawCards(5, rng); // Тянем 5 карт
}

void Player::endTurn() {
    m_deck.discardHand(); // Всю несыгранную руку в сброс
}

void Player::addArtifact(std::unique_ptr<Artifact> artifact) {
    if (!artifact) return;

    // Применяем пассивные эффекты сразу
    if (artifact->getTrigger() == ArtifactTrigger::Passive) {
        for (const auto& eff : artifact->getEffects()) {
            if (eff.type == ArtifactEffectType::IncreaseMaxHp) {
                m_maxHp += eff.value;
                m_hp += eff.value;
            }
            else if (eff.type == ArtifactEffectType::IncreaseMaxEnergy) {
                m_maxEnergy += eff.value;
            }
        }
    }

    m_artifacts.push_back(std::move(artifact));
}

int Player::getArtifactBonus(ArtifactEffectType type) const {
    int total = 0;
    for (const auto& a : m_artifacts) {
        if (a->getTrigger() == ArtifactTrigger::Passive) {
            for (const auto& eff : a->getEffects()) {
                if (eff.type == type) {
                    total += eff.value;
                }
            }
        }
    }
    return total;
}

} // namespace engine
