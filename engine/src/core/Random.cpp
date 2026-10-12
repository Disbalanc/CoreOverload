// ============================================================
// Random.cpp — Реализация генератора случайных чисел
// ============================================================

#include "engine/core/Random.h"
#include <chrono>

namespace engine {

Random::Random(uint32_t seed)
    : m_seed(seed)
    , m_engine(seed)
{
    // m_engine инициализируется seed'ом.
    // При одинаковом seed → одинаковая последовательность.
    // Это критично для seed-based забегов!
}

Random::Random()
    : Random(static_cast<uint32_t>(
          std::chrono::steady_clock::now()
              .time_since_epoch().count()))
{
    // Делегируем конструктору с seed.
    // Seed берём из системных часов — каждый запуск уникален.
}

uint32_t Random::getSeed() const {
    return m_seed;
}

int Random::rollInt(int min, int max) {
    // std::uniform_int_distribution даёт равномерное распределение
    // в диапазоне [min, max] (оба включительно)
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_engine);
}

float Random::rollFloat(float min, float max) {
    // std::uniform_real_distribution даёт [min, max)
    // (max НЕ включён, но для float это неважно)
    std::uniform_real_distribution<float> dist(min, max);
    return dist(m_engine);
}

bool Random::rollChance(float probability) {
    // Генерируем float [0, 1) и сравниваем с порогом
    return rollFloat(0.0f, 1.0f) < probability;
}

} // namespace engine
