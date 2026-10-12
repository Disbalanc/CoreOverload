#include "engine/cards/Deck.h"
#include <algorithm>
#include <stdexcept>

namespace engine {

void Deck::addToMasterDeck(std::unique_ptr<Card> card) {
    m_masterDeck.push_back(std::move(card));
}

void Deck::removeCardFromMasterDeck(const std::string& cardId) {
    auto it = std::find_if(m_masterDeck.begin(), m_masterDeck.end(),
        [&cardId](const std::unique_ptr<Card>& c) { return c->getId() == cardId; });

    if (it != m_masterDeck.end()) {
        m_masterDeck.erase(it);
    }
}

void Deck::initForCombat(Random& rng) {
    m_drawPile.clear();
    m_hand.clear();
    m_discardPile.clear();
    m_exhaustPile.clear();

    // Заполняем колоду взятия указателями на карты из мастер-колоды
    for (const auto& card : m_masterDeck) {
        card->resetCost(); // На всякий случай сбрасываем баффы стоимости
        m_drawPile.push_back(card.get());
    }

    // Перемешиваем колоду
    rng.shuffle(m_drawPile);
}

void Deck::shuffleDiscardIntoDraw(Random& rng) {
    if (m_discardPile.empty()) return;

    int count = static_cast<int>(m_discardPile.size());

    m_drawPile = std::move(m_discardPile);
    m_discardPile.clear();
    rng.shuffle(m_drawPile);

    if (m_shuffleCallback) {
        m_shuffleCallback(count);
    }
}

void Deck::drawCards(int amount, Random& rng) {
    for (int i = 0; i < amount; ++i) {
        // Если колода взятия пуста, пытаемся перетасовать сброс
        if (m_drawPile.empty()) {
            shuffleDiscardIntoDraw(rng);
        }

        // Если и сброс пуст — карт в игре больше нет, прекращаем добор
        if (m_drawPile.empty()) {
            break;
        }

        // Берём верхнюю карту (с конца вектора — это O(1))
        Card* card = m_drawPile.back();
        m_drawPile.pop_back();

        m_hand.push_back(card);
    }
}

void Deck::discardHand() {
    // Переносим все карты из руки в сброс
    m_discardPile.insert(m_discardPile.end(), m_hand.begin(), m_hand.end());
    m_hand.clear();
}

void Deck::playCard(size_t handIndex) {
    if (handIndex >= m_hand.size()) {
        throw std::out_of_range("Invalid card index in hand!");
    }

    Card* card = m_hand[handIndex];
    m_hand.erase(m_hand.begin() + handIndex);
    m_discardPile.push_back(card);
}

void Deck::exhaustCard(size_t handIndex) {
    if (handIndex >= m_hand.size()) {
        throw std::out_of_range("Invalid card index in hand!");
    }

    Card* card = m_hand[handIndex];
    m_hand.erase(m_hand.begin() + handIndex);
    m_exhaustPile.push_back(card);
}

} // namespace engine
