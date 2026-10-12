#pragma once
// ============================================================
// Random.h — Обёртка над Mersenne Twister
//
// Зачем не использовать rand()?
//   1. rand() непереносим (разные результаты на разных ОС)
//   2. rand() имеет плохое распределение
//   3. Нам нужен seed для повторяемости забегов
//
// Этот класс:
//   - Детерминирован при одинаковом seed
//   - Потокобезопасен (каждый поток — свой экземпляр)
//   - Даёт равномерное распределение для int, float, bool
// ============================================================

#include <random>
#include <cstdint>
#include <algorithm>

namespace engine {

class Random {
public:
    // Конструктор с явным seed (для повторяемости)
    explicit Random(uint32_t seed);

    // Конструктор со случайным seed (для обычных забегов)
    Random();

    // Получить текущий seed (чтобы сохранить/показать игроку)
    uint32_t getSeed() const;

    // ── Основные методы ──────────────────────────────────

    // Случайное целое в диапазоне [min, max] (включительно!)
    // Пример: rollInt(1, 6) → число от 1 до 6
    int rollInt(int min, int max);

    // Случайное вещественное в диапазоне [min, max)
    // Пример: rollFloat(0.0f, 1.0f) → число от 0 до ~1
    float rollFloat(float min, float max);

    // Случайный bool с заданной вероятностью true
    // Пример: rollChance(0.5f) → 50% шанс true
    bool rollChance(float probability);

    // Случайный элемент из контейнера
    // Пример: rollChoice(myVector) → случайный элемент
    template <typename Container>
    auto& rollChoice(Container& container) {
        int index = rollInt(0, static_cast<int>(container.size()) - 1);
        return container[index];
    }

    // Перетасовать контейнер (Fisher-Yates shuffle)
    template <typename Container>
    void shuffle(Container& container) {
        // std::shuffle принимает генератор напрямую
        std::shuffle(container.begin(), container.end(), m_engine);
    }

private:
    uint32_t m_seed;
    std::mt19937 m_engine;  // Mersenne Twister 19937
};

} // namespace engine
