#include "engine/entities/CombatEntity.h"
#include <algorithm>

namespace engine {

CombatEntity::CombatEntity(int maxHp)
    : m_maxHp(maxHp)
    , m_hp(maxHp)
{
}

void CombatEntity::gainBlock(int amount) {
    if (amount <= 0) return;
    m_block += amount;
}

void CombatEntity::resetBlock() {
    m_block = 0;
}

int CombatEntity::takeDamage(int amount, bool ignoreBlock) {
    if (amount <= 0) return 0;

    int damageToHp = amount;

    if (!ignoreBlock) {
        if (m_block >= damageToHp) {
            m_block -= damageToHp;
            damageToHp = 0;
        } else {
            damageToHp -= m_block;
            m_block = 0;
        }
    }

    m_hp = std::max(0, m_hp - damageToHp);
    return damageToHp; // Возвращаем чистый урон по структуре (HP)
}

void CombatEntity::heal(int amount) {
    if (amount <= 0 || !isAlive()) return;
    m_hp = std::min(m_maxHp, m_hp + amount);
}

int CombatEntity::getStatusStacks(const std::string& statusId) const {
    auto it = std::find_if(m_statuses.begin(), m_statuses.end(),
        [&statusId](const StatusInstance& s) { return s.id == statusId; });
    return (it != m_statuses.end()) ? it->stacks : 0;
}

void CombatEntity::applyStatus(const std::string& statusId, int stacks) {
    if (stacks <= 0) return;

    auto it = std::find_if(m_statuses.begin(), m_statuses.end(),
        [&statusId](const StatusInstance& s) { return s.id == statusId; });

    if (it != m_statuses.end()) {
        it->stacks += stacks;
    } else {
        m_statuses.push_back({statusId, stacks});
    }
}

void CombatEntity::removeStatus(const std::string& statusId) {
    m_statuses.erase(
        std::remove_if(m_statuses.begin(), m_statuses.end(),
            [&statusId](const StatusInstance& s) { return s.id == statusId; }),
        m_statuses.end()
    );
}

int CombatEntity::tickStatuses(TurnPhase phase) {
    int damageFromStatuses = 0;

    // В конце хода срабатывает статус "leak" (Утечка)
    if (phase == TurnPhase::PlayerTurnEnd || phase == TurnPhase::EnemyTurnEnd) {
        int leakStacks = getStatusStacks("leak");
        if (leakStacks > 0) {
            damageFromStatuses = takeDamage(leakStacks, true); // Утечка наносит чистый урон

            // Уменьшаем стек утечки на 1 каждый ход
            applyStatus("leak", -1);
            // Если стек стал 0, удаляем статус
            if (getStatusStacks("leak") <= 0) {
                removeStatus("leak");
            }
        }
    }

    return damageFromStatuses;
}

} // namespace engine
