#pragma once

#include "engine/entities/CombatEntity.h"
#include "Player.h"
#include <memory>

namespace engine {

// Опережающее объявление ИИ
class EnemyAI;

class Enemy : public CombatEntity {
public:
    Enemy(std::string id, std::string name, int maxHp);
    virtual ~Enemy();

    const std::string& getId() const { return m_id; }
    const std::string& getName() const { return m_name; }

    // Установка интеллекта врага
    void setAI(std::unique_ptr<EnemyAI> ai);

    // ── Система Намерений (Intent) ────────────────────────
    const EnemyIntent& getIntent() const { return m_intent; }

    // Рассчитать намерение на следующий ход
    void updateIntent(int turnNumber);

    // Выполнить запланированное намерение на игрока
    void executeIntent(Player& player);

private:
    std::string m_id;
    std::string m_name;
    EnemyIntent m_intent;
    std::unique_ptr<EnemyAI> m_ai;
};

// ── ИНТЕРФЕЙС ИИ ВРАГА (Паттерн Стратегия) ────────────────
class EnemyAI {
public:
    virtual ~EnemyAI() = default;

    // Рассчитать намерение
    virtual EnemyIntent calculateIntent(int turnNumber) = 0;

    // Выполнить действие
    virtual void execute(Enemy& self, Player& player, const EnemyIntent& intent) = 0;
};

// ── РЕАЛИЗАЦИЯ: Паттерн-ИИ (для Дронов) ───────────────────
// Ходит по жестко заданному циклу. Например: Атака -> Атака -> Блок -> Повтор.
class PatternAI : public EnemyAI {
public:
    PatternAI(std::vector<EnemyIntent> pattern);

    EnemyIntent calculateIntent(int turnNumber) override;
    void execute(Enemy& self, Player& player, const EnemyIntent& intent) override;

private:
    std::vector<EnemyIntent> m_pattern;
};

} // namespace engine
