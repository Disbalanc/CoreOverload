#pragma once
// ============================================================
// EventBus.h — Шина событий (паттерн Observer/Pub-Sub)
//
// Как работает:
//   1. Движок ПУБЛИКУЕТ событие:
//      eventBus.publish({EventType::DamageDealt, {{"amount", 6}}});
//
//   2. UI ПОДПИСЫВАЕТСЯ на событие:
//      eventBus.subscribe(EventType::DamageDealt, [](const GameEvent& e) {
//          showDamageAnimation(e.getInt("amount"));
//      });
//
// Зачем это нужно:
//   - Движок НЕ знает о Qt, анимациях, виджетах
//   - UI НЕ лезет внутрь боевой логики
//   - Можно подключить хоть 10 слушателей к одному событию
//   - Легко добавлять логирование, достижения, звуки
// ============================================================

#include "engine/core/Types.h"

#include <functional>
#include <vector>
#include <unordered_map>

namespace engine {

// Тип колбэка — функция, принимающая событие
using EventCallback = std::function<void(const GameEvent&)>;

// ID подписки — чтобы можно было отписаться
using SubscriptionId = uint32_t;

class EventBus {
public:
    EventBus() = default;
    ~EventBus() = default;

    // Не копируем (синглтон-подобное поведение)
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    // ── Подписка ──────────────────────────────────────────
    // Подписаться на конкретный тип события.
    // Возвращает ID подписки (для отписки).
    //
    // Пример:
    //   auto id = bus.subscribe(EventType::EnemyDied, [](const GameEvent& e) {
    //       std::cout << "Враг " << e.getString("name") << " мёртв!\n";
    //   });
    SubscriptionId subscribe(EventType type, EventCallback callback);

    // ── Отписка ───────────────────────────────────────────
    // Удалить подписку по ID (например, при уничтожении виджета)
    void unsubscribe(SubscriptionId id);

    // Отписаться от ВСЕХ событий (удобно при очистке)
    void unsubscribeAll();

    // ── Публикация ────────────────────────────────────────
    // Отправить событие всем подписчикам.
    // Вызовы синхронные — колбэки выполняются сразу.
    //
    // Пример:
    //   bus.publish({EventType::DamageDealt, {
    //       {"amount", 6},
    //       {"target_id", 42},
    //       {"source", std::string("ddos_strike")}
    //   }});
    void publish(const GameEvent& event);

    // Упрощённая версия — только тип, без данных
    void publish(EventType type);

    // ── Отложенная публикация ─────────────────────────────
    // Иногда нельзя публиковать событие прямо сейчас
    // (например, мы внутри итерации по врагам и не хотим
    //  удалять врага посреди цикла).
    // queue() сохраняет событие, flush() отправляет все.
    void queue(const GameEvent& event);
    void flush();

private:
    struct Subscription {
        SubscriptionId id;
        EventCallback callback;
    };

    // Подписчики, сгруппированные по типу события
    std::unordered_map<EventType, std::vector<Subscription>> m_subscribers;

    // Очередь отложенных событий
    std::vector<GameEvent> m_pendingEvents;

    // Счётчик для генерации уникальных ID подписки
    SubscriptionId m_nextSubscriptionId = 1;
};

} // namespace engine
