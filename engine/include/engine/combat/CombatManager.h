#pragma once
// ============================================================
// CombatManager.h — Главный контроллер боя
//
// Отвечает за:
//   - Инициализацию боя (создание врагов, подготовка колоды)
//   - Управление фазами хода (игрок → враги → проверка смерти)
//   - Розыгрыш карт и разрешение их эффектов
//   - Публикацию событий для UI через EventBus
//
// UI вызывает только публичные методы этого класса.
// Вся логика боя инкапсулирована здесь.
// ============================================================

#include "engine/core/Types.h"
#include "engine/core/EventBus.h"
#include "engine/core/Random.h"
#include "engine/entities/Player.h"
#include "engine/entities/Enemy.h"

#include <vector>
#include <memory>
#include <string>

namespace engine {

// ── Конфигурация врага для создания ───────────────────────
// Простая структура, чтобы не тащить JSON-парсер в движок.
// Позже будем заполнять из JSON, пока — вручную.
struct EnemyConfig {
    std::string id;
    std::string name;
    int hp;
    std::vector<EnemyIntent> pattern; // Цикл поведения
};

class CombatManager {
public:
    CombatManager(EventBus& eventBus, Random& rng);
    ~CombatManager() = default;

    // ── Инициализация боя ─────────────────────────────────
    // Создаёт врагов по конфигу, инициализирует колоду игрока.
    // Вызывается один раз при входе в бой.
    void startCombat(Player& player, const std::vector<EnemyConfig>& enemies);

    // ── Действия игрока (вызываются из UI) ────────────────

    // Сыграть карту из руки на цель.
    // cardIndex — индекс в руке (0..4)
    // targetIndex — индекс врага (для SingleEnemy), -1 для Self/All/None
    // Возвращает true если карта успешно сыграна.
    bool playCard(int cardIndex, int targetIndex = -1);

    // Закончить ход игрока. Запускает фазу врагов.
    void endPlayerTurn();

    // ── Запросы состояния (для UI) ────────────────────────
    TurnPhase getPhase() const { return m_phase; }
    int getTurnNumber() const { return m_turnNumber; }
    bool isPlayerTurn() const { return m_phase == TurnPhase::PlayerAction; }
    bool isCombatOver() const { return m_phase == TurnPhase::CombatEnd; }
    bool didPlayerWin() const { return m_playerWon; }

    const Player& getPlayer() const { return *m_player; }
    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const { return m_enemies; }

    // Получить список живых врагов (удобно для UI)
    std::vector<const Enemy*> getAliveEnemies() const;

private:
    // ── Внутренние фазы хода ──────────────────────────────
    void startPlayerTurn();
    void executeEnemyTurns();
    void tickAllStatuses();
    void checkCombatEnd();

    // ── Разрешение эффектов карты ─────────────────────────
    // Проходит по списку эффектов карты и применяет каждый
    void resolveCardEffects(const Card& card, int targetIndex);

    // Вспомогательные
    Enemy* getAliveEnemy(int index);
    int countAliveEnemies() const;

    // ── Данные ────────────────────────────────────────────
    EventBus& m_eventBus;
    Random& m_rng;

    Player* m_player = nullptr; // Не владеем — ссылка на игрока извне
    std::vector<std::unique_ptr<Enemy>> m_enemies;

    TurnPhase m_phase = TurnPhase::CombatStart;
    int m_turnNumber = 0;
    bool m_playerWon = false;
};

} // namespace engine
