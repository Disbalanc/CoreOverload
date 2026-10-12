#pragma once

#include "engine/core/Types.h"
#include <vector>
#include <string>

namespace engine {

class CombatEntity {
public:
    CombatEntity(int maxHp);
    virtual ~CombatEntity() = default;

    // ── Геттеры состояния ─────────────────────────────────
    int getHp() const { return m_hp; }
    int getMaxHp() const { return m_maxHp; }
    int getBlock() const { return m_block; }
    bool isAlive() const { return m_hp > 0; }

    // ── Боевые методы ─────────────────────────────────────
    void gainBlock(int amount);
    void resetBlock();

    // Получение урона. Возвращает РЕАЛЬНО нанесенный урон по HP
    int takeDamage(int amount, bool ignoreBlock = false);

    void heal(int amount);

    // ── Работа со статусами (Баффы/Дебаффы) ───────────────
    const std::vector<StatusInstance>& getStatuses() const { return m_statuses; }
    int getStatusStacks(const std::string& statusId) const;

    void applyStatus(const std::string& statusId, int stacks);
    void removeStatus(const std::string& statusId);

    // Вызывается в разные фазы хода для применения эффектов статусов (например, урон от Утечки)
    // Возвращает урон, полученный от тика статусов (если он есть)
    virtual int tickStatuses(TurnPhase phase);

protected:
    int m_maxHp;
    int m_hp;
    int m_block = 0; // Наш Файрвол

    std::vector<StatusInstance> m_statuses;
};

} // namespace engine
