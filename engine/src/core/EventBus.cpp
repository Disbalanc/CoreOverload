// ============================================================
// EventBus.cpp — Реализация шины событий
// ============================================================

#include "engine/core/EventBus.h"
#include <algorithm>

namespace engine {

SubscriptionId EventBus::subscribe(EventType type, EventCallback callback) {
    SubscriptionId id = m_nextSubscriptionId++;
    m_subscribers[type].push_back({id, std::move(callback)});
    return id;
}

void EventBus::unsubscribe(SubscriptionId id) {
    // Ищем подписку во всех типах событий
    for (auto& [type, subs] : m_subscribers) {
        auto it = std::find_if(subs.begin(), subs.end(),
            [id](const Subscription& s) { return s.id == id; });

        if (it != subs.end()) {
            subs.erase(it);
            return; // ID уникален, дальше искать не нужно
        }
    }
}

void EventBus::unsubscribeAll() {
    m_subscribers.clear();
}

void EventBus::publish(const GameEvent& event) {
    // Находим всех подписчиков на этот тип события
    auto it = m_subscribers.find(event.type);
    if (it == m_subscribers.end()) {
        return; // Никто не слушает — тихо выходим
    }

    // Вызываем каждый колбэк
    // Копируем вектор на случай, если колбэк отпишется внутри
    auto subs = it->second;
    for (const auto& sub : subs) {
        if (sub.callback) {
            sub.callback(event);
        }
    }
}

void EventBus::publish(EventType type) {
    publish(GameEvent{type, {}});
}

void EventBus::queue(const GameEvent& event) {
    m_pendingEvents.push_back(event);
}

void EventBus::flush() {
    // Копируем и очищаем ДО отправки,
    // чтобы новые queue() внутри колбэков не зациклились
    auto pending = std::move(m_pendingEvents);
    m_pendingEvents.clear();

    for (const auto& event : pending) {
        publish(event);
    }
}

} // namespace engine
