#pragma once
// ============================================================
// Types.h — Базовые типы и перечисления для всего движка
//
// Этот файл включается практически везде. Он содержит только
// enum'ы и простые struct'ы — никакой логики.
// ============================================================

#include <string>
#include <cstdint>
#include <variant>
#include <unordered_map>

namespace engine {

// ── Типы карт ─────────────────────────────────────────────
enum class CardType {
    Attack,   // Эксплойты, скрипты атаки
    Skill,    // Манипуляции, баффы, дебаффы
    Power,    // Перманентные эффекты (пока не используем)
    Curse     // Мусорные карты от босса
};

// ── Редкость карт ─────────────────────────────────────────
enum class CardRarity {
    Common,
    Uncommon,
    Rare,
    Legendary,
    Curse      // Для "Сбой памяти" и подобного мусора
};

// ── Цель карты ────────────────────────────────────────────
enum class CardTarget {
    SingleEnemy,   // Выбрать одного врага
    AllEnemies,    // Все враги
    Self,          // На себя (защита, баффы)
    None           // Не требует цели (взять карты и т.п.)
};

// ── Типы узлов на карте мира ──────────────────────────────
enum class NodeType {
    Battle,
    EliteBattle,
    BossBattle,
    Rest,
    Event,
    Shop,
    Treasure
};

// ── Фазы боя ──────────────────────────────────────────────
enum class TurnPhase {
    CombatStart,
    PlayerTurnStart,
    PlayerAction,     // Ждём кликов игрока
    PlayerTurnEnd,
    EnemyTurnStart,
    EnemyAction,
    EnemyTurnEnd,
    CombatEnd
};

// ── Типы намерений врага ──────────────────────────────────
enum class IntentType {
    Attack,
    Defend,
    Buff,
    Debuff,
    AttackAndDefend,
    Summon,
    Unknown
};

// ── Типы эффектов карт ────────────────────────────────────
enum class CardEffectType {
    Damage,             // Нанести урон
    DamageIgnoreBlock,  // Нанести чистый урон (пробитие брони)
    Block,              // Дать файрвол (броню)
    GainEnergy,         // Дать энергию
    DrawCards,          // Взять карты из колоды
    ApplyStatus,        // Наложить статус на врага (например, Утечка)
    SelfDamage          // Нанести урон себе (для Разгона)
};

// Структура одного эффекта (например: Damage, значение = 6)
struct CardEffect {
    CardEffectType type;
    int value = 0;
    std::string statusId = ""; // Используется только для ApplyStatus
};

// ── Типы событий для EventBus ─────────────────────────────
// Это "словарь" всех событий в игре.
// UI подписывается на нужные и реагирует.
enum class EventType {
    // Бой
    CombatStarted,
    CombatEnded,
    TurnStarted,
    TurnEnded,
    CardPlayed,
    CardDrawn,
    CardDiscarded,
    DamageDealt,
    DamageReceived,
    BlockGained,
    StatusApplied,
    StatusRemoved,
    EnemyDied,
    PlayerDied,
    EnergyChanged,
    DeckShuffled,   // Колода перетасована (сброс → draw pile)
        HandDiscarded,  // Рука сброшена в конце хода

    // Карта мира
    NodeSelected,
    MapGenerated,

    // Награды
    RewardOffered,
    RewardSelected,

    // Мета
    RunStarted,
    RunEnded,
    AchievementUnlocked
};

// ── Универсальное событие ─────────────────────────────────
// Данные события — словарь "ключ → значение".
// Значение может быть int, float или string.
// Пример: { "damage" → 6, "target_id" → "drone_0" }
using EventValue = std::variant<int, float, std::string, bool>;
using EventData = std::unordered_map<std::string, EventValue>;

struct GameEvent {
    EventType type;
    EventData data;

    // Удобные геттеры с приведением типа
    int getInt(const std::string& key, int defaultVal = 0) const {
        auto it = data.find(key);
        if (it != data.end() && std::holds_alternative<int>(it->second))
            return std::get<int>(it->second);
        return defaultVal;
    }

    std::string getString(const std::string& key,
                          const std::string& defaultVal = "") const {
        auto it = data.find(key);
        if (it != data.end() && std::holds_alternative<std::string>(it->second))
            return std::get<std::string>(it->second);
        return defaultVal;
    }

    bool getBool(const std::string& key, bool defaultVal = false) const {
        auto it = data.find(key);
        if (it != data.end() && std::holds_alternative<bool>(it->second))
            return std::get<bool>(it->second);
        return defaultVal;
    }
};

// ── Уникальный ID для сущностей ───────────────────────────
// Простой счётчик. Каждый враг/карта в бою получает свой ID.
using EntityId = uint32_t;

inline EntityId generateEntityId() {
    static EntityId counter = 0;
    return ++counter;
}

// ── Структура намерения врага ─────────────────────────────
struct EnemyIntent {
    IntentType type = IntentType::Unknown;
    int value = 0;      // Значение действия (урон или броня)
    int hitCount = 1;   // Количество ударов (например, 4 урона x 2 раза)
};

// ── Структура экземпляра статуса ──────────────────────────
struct StatusInstance {
    std::string id;     // "leak" (Утечка), "vulnerable" (Уязвимость) и т.д.
    int stacks = 0;     // Количество зарядов/стаков
};

} // namespace engine
