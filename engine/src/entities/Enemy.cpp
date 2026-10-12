#include "engine/entities/Enemy.h"

namespace engine {

// ── Реализация Enemy ──────────────────────────────────────

Enemy::Enemy(std::string id, std::string name, int maxHp)
    : CombatEntity(maxHp)
    , m_id(std::move(id))
    , m_name(std::move(name))
{
}

Enemy::~Enemy() = default;

void Enemy::setAI(std::unique_ptr<EnemyAI> ai) {
    m_ai = std::move(ai);
}

void Enemy::updateIntent(int turnNumber) {
    if (m_ai) {
        m_intent = m_ai->calculateIntent(turnNumber);
    }
}

void Enemy::executeIntent(Player& player) {
    if (m_ai && isAlive()) {
        m_ai->execute(*this, player, m_intent);
    }
}

// ── Реализация PatternAI (Движения по кругу) ─────────────

PatternAI::PatternAI(std::vector<EnemyIntent> pattern)
    : m_pattern(std::move(pattern))
{
}

EnemyIntent PatternAI::calculateIntent(int turnNumber) {
    if (m_pattern.empty()) return EnemyIntent{};
    // Выбираем действие по остатку от деления номера хода
    size_t index = static_cast<size_t>(turnNumber) % m_pattern.size();
    return m_pattern[index];
}

void PatternAI::execute(Enemy& self, Player& player, const EnemyIntent& intent) {
    switch (intent.type) {
        case IntentType::Attack:
            for (int i = 0; i < intent.hitCount; ++i) {
                player.takeDamage(intent.value);
            }
            break;

        case IntentType::Defend:
            self.gainBlock(intent.value);
            break;

        case IntentType::AttackAndDefend:
            player.takeDamage(intent.value); // Здесь value используем как урон
            self.gainBlock(intent.value / 2); // А половину в блок
            break;

        case IntentType::Debuff:
            player.applyStatus("leak", intent.value); // Навешиваем утечку
            break;

        default:
            break;
    }
}

} // namespace engine
