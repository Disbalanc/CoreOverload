#pragma once

#include "engine/cards/Card.h"
#include "engine/core/Random.h"
#include <functional>
#include <vector>
#include <memory>

namespace engine {

class Deck {
public:
    Deck() = default;
    ~Deck() = default;

    // Не копируем колоду (чтобы случайно не сдублировать уникальные указатели)
    Deck(const Deck&) = delete;
    Deck& operator=(const Deck&) = delete;
    Deck(Deck&&) = default;
    Deck& operator=(Deck&&) = default;

    // ── Управление глобальной колодой (вне боя) ────────────
    void addToMasterDeck(std::unique_ptr<Card> card);
    const std::vector<std::unique_ptr<Card>>& getMasterDeck() const { return m_masterDeck; }
    void removeCardFromMasterDeck(const std::string& cardId);

    // ── Управление боем (Combat Loop) ─────────────────────

    // Инициализировать колоду перед боем (копирует masterDeck в Draw Pile и перемешивает)
    void initForCombat(Random& rng);

    // Взять N карт в руку
    void drawCards(int amount, Random& rng);

    // Сбросить всю руку в сброс (в конце хода игрока)
    void discardHand();

    // Сыграть карту из руки (перемещает её в сброс)
    void playCard(size_t handIndex);

    // Сжечь карту (Exhaust) — удалить её из боя совсем
    void exhaustCard(size_t handIndex);

    // Геттеры для боя
    const std::vector<Card*>& getHand() const { return m_hand; }
    const std::vector<Card*>& getDrawPile() const { return m_drawPile; }
    const std::vector<Card*>& getDiscardPile() const { return m_discardPile; }
    const std::vector<Card*>& getExhaustPile() const { return m_exhaustPile; }
    using ShuffleCallback = std::function<void(int cardsShuffled)>;
    void setShuffleCallback(ShuffleCallback cb) { m_shuffleCallback = cb; }

private:
    // Вспомогательный метод: перетасовать сброс обратно в колоду взятия
    void shuffleDiscardIntoDraw(Random& rng);

    // Глобальное владение всеми картами игрока
    std::vector<std::unique_ptr<Card>> m_masterDeck;

    // Ссылки на карты во время боя (карты физически лежат в m_masterDeck,
    // а здесь мы просто жонглируем указателями на них)
    std::vector<Card*> m_drawPile;
    std::vector<Card*> m_hand;
    std::vector<Card*> m_discardPile;
    std::vector<Card*> m_exhaustPile;
    ShuffleCallback m_shuffleCallback;
};

} // namespace engine
