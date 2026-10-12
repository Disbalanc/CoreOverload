#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <vector>
#include <memory>
#include <engine/entities/Player.h>
#include <engine/core/Random.h>
#include <engine/cards/Card.h>

class CardWidget;

class ShopScreen : public QWidget {
    Q_OBJECT

public:
    explicit ShopScreen(engine::Player& player, engine::Random& rng, QWidget* parent = nullptr);

signals:
    void shopExited();

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onBuyCard(int index);
    void onRemoveFromDeck();
    void onExit();

private:
    void generateShop();
    void rebuildUI();

    engine::Player& m_player;
    engine::Random& m_rng;

    struct ShopItem {
        std::unique_ptr<engine::Card> card;
        int price;
        bool sold = false;
    };

    std::vector<ShopItem> m_items;
    QWidget* m_cardsContainer = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_hpLabel = nullptr;
    QPushButton* m_removeBtn = nullptr;
    QPushButton* m_exitBtn = nullptr;
};
