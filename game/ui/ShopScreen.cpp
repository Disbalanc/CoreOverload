#include "ShopScreen.h"
#include "CardWidget.h"
#include "GameLog.h"
#include <engine/cards/CardDatabase.h>
#include <QPainter>

ShopScreen::ShopScreen(engine::Player& player, engine::Random& rng, QWidget* parent)
    : QWidget(parent)
    , m_player(player)
    , m_rng(rng)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(30, 20, 30, 20);
    root->setSpacing(15);

    auto* title = new QLabel("💾 КРЕМНИЕВЫЙ ЧЁРНЫЙ РЫНОК");
    title->setAlignment(Qt::AlignCenter);
    title->setFont(QFont("Courier New", 16, QFont::Bold));
    title->setStyleSheet("color: #FFD700; background-color: #16213E; padding: 12px; border: 2px solid #FFD700;");
    root->addWidget(title);

    auto* desc = new QLabel(
        "Торговец кивает: «Платёж принимается только в единицах здоровья.\n"
        "Каждый байт стоит крови. Выбирай с умом.»"
    );
    desc->setAlignment(Qt::AlignCenter);
    desc->setFont(QFont("Courier New", 10));
    desc->setStyleSheet("color: #C0C0C0; background: transparent; padding: 10px;");
    root->addWidget(desc);

    m_hpLabel = new QLabel();
    m_hpLabel->setAlignment(Qt::AlignCenter);
    m_hpLabel->setFont(QFont("Courier New", 11, QFont::Bold));
    m_hpLabel->setStyleSheet("color: #E94560; background: transparent;");
    root->addWidget(m_hpLabel);

    // Контейнер карт (ручное позиционирование)
    m_cardsContainer = new QWidget();
    m_cardsContainer->setMinimumHeight(CardWidget::CARD_HEIGHT + 80);
    m_cardsContainer->setStyleSheet("background: transparent;");
    root->addWidget(m_cardsContainer, 0, Qt::AlignCenter);

    m_statusLabel = new QLabel("Кликните на карту, чтобы купить её.");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setFont(QFont("Courier New", 9));
    m_statusLabel->setStyleSheet("color: #808080; background: transparent;");
    root->addWidget(m_statusLabel);

    // Кнопки удаления и выхода
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setAlignment(Qt::AlignCenter);
    btnLayout->setSpacing(20);

    m_removeBtn = new QPushButton("УДАЛИТЬ КАРТУ ИЗ КОЛОДЫ (-5 HP)");
    m_removeBtn->setFixedSize(350, 45);
    m_removeBtn->setFont(QFont("Courier New", 10, QFont::Bold));
    m_removeBtn->setStyleSheet(
        "QPushButton { background-color: #533483; color: #FFFFFF; border: 2px solid #533483; }"
        "QPushButton:hover { background-color: #FFFFFF; color: #533483; }"
    );
    connect(m_removeBtn, &QPushButton::clicked, this, &ShopScreen::onRemoveFromDeck);
    btnLayout->addWidget(m_removeBtn);

    m_exitBtn = new QPushButton("ПОКИНУТЬ РЫНОК");
    m_exitBtn->setFixedSize(220, 45);
    m_exitBtn->setFont(QFont("Courier New", 10, QFont::Bold));
    m_exitBtn->setStyleSheet(
        "QPushButton { background-color: #16213E; color: #808080; border: 1px solid #808080; }"
        "QPushButton:hover { background-color: #808080; color: #0A0A0F; }"
    );
    connect(m_exitBtn, &QPushButton::clicked, this, &ShopScreen::onExit);
    btnLayout->addWidget(m_exitBtn);

    root->addLayout(btnLayout);

    GameLog::instance().addMessage("💾 Вошли в чёрный рынок кремния", "#FFD700");

    generateShop();
    rebuildUI();
}

void ShopScreen::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor("#0A0A0F"));

    p.setPen(QPen(QColor(255, 215, 0, 8)));
    for (int y = 0; y < height(); y += 4) {
        p.drawLine(0, y, width(), y);
    }
}

void ShopScreen::generateShop() {
    m_items.clear();

    // Выбираем 4 случайные карты из базы (все кроме curse)
    auto allIds = engine::CardDatabase::instance().getAllIds();
    std::vector<std::string> buyable;
    for (const auto& id : allIds) {
        auto* t = engine::CardDatabase::instance().getTemplate(id);
        if (t && t->rarity != engine::CardRarity::Curse) {
            buyable.push_back(id);
        }
    }

    m_rng.shuffle(buyable);
    int count = std::min(4, static_cast<int>(buyable.size()));

    for (int i = 0; i < count; ++i) {
        ShopItem item;
        item.card = engine::CardDatabase::instance().createCard(buyable[i], 1000 + i);

        auto* t = engine::CardDatabase::instance().getTemplate(buyable[i]);
        if (!t || !item.card) continue;

        // Цена зависит от редкости
        switch (t->rarity) {
            case engine::CardRarity::Common:    item.price = 6; break;
            case engine::CardRarity::Uncommon:  item.price = 10; break;
            case engine::CardRarity::Rare:      item.price = 15; break;
            default:                             item.price = 8; break;
        }

        m_items.push_back(std::move(item));
    }
}

void ShopScreen::rebuildUI() {
    m_hpLabel->setText(QString("ЦЕЛОСТНОСТЬ: %1 / %2 HP").arg(m_player.getHp()).arg(m_player.getMaxHp()));

    // Удаляем старые виджеты
    for (auto* c : m_cardsContainer->findChildren<QWidget*>()) {
        c->deleteLater();
    }

    int cardW = CardWidget::CARD_WIDTH;
    int spacing = 25;
    int count = static_cast<int>(m_items.size());
    int totalW = count * cardW + (count - 1) * spacing;

    m_cardsContainer->setFixedWidth(totalW + 40);

    for (int i = 0; i < count; ++i) {
        auto& item = m_items[i];
        if (item.sold) continue;

        auto* cw = new CardWidget(item.card.get(), i, m_cardsContainer);
        connect(cw, &CardWidget::cardClicked, this, &ShopScreen::onBuyCard);
        int x = 20 + i * (cardW + spacing);
        int y = 15;
        cw->setBasePosition(QPoint(x, y));
        cw->move(x, y);
        cw->show();

        // Ярлык цены под картой
        auto* priceLabel = new QLabel(QString("ЦЕНА: %1 HP").arg(item.price), m_cardsContainer);
        priceLabel->setFont(QFont("Courier New", 10, QFont::Bold));
        priceLabel->setStyleSheet(
            m_player.getHp() > item.price
                ? "color: #FFD700; background-color: #0A0A0F; padding: 4px; border: 1px solid #FFD700;"
                : "color: #808080; background-color: #0A0A0F; padding: 4px; border: 1px solid #555;"
        );
        priceLabel->setAlignment(Qt::AlignCenter);
        priceLabel->move(x, y + CardWidget::CARD_HEIGHT + 10);
        priceLabel->setFixedWidth(cardW);
        priceLabel->show();
    }
}

void ShopScreen::onBuyCard(int index) {
    if (index < 0 || index >= static_cast<int>(m_items.size())) return;
    auto& item = m_items[index];
    if (item.sold || !item.card) return;

    if (m_player.getHp() <= item.price) {
        m_statusLabel->setText("⚠ НЕДОСТАТОЧНО HP ДЛЯ ПОКУПКИ!");
        m_statusLabel->setStyleSheet("color: #E94560;");
        return;
    }

    // Платим HP (не через takeDamage, т.к. это не атака)
    m_player.takeDamage(item.price, true);
    GameLog::instance().addMessage(
        QString("💰 Куплено: '%1' за %2 HP")
            .arg(QString::fromStdString(item.card->getName()))
            .arg(item.price),
        "#FFD700");

    // Добавляем карту в колоду игрока
    m_player.getDeck().addToMasterDeck(std::move(item.card));
    item.sold = true;

    m_statusLabel->setText(QString("✓ Куплено! Карта добавлена в колоду."));
    m_statusLabel->setStyleSheet("color: #00FF41;");

    rebuildUI();
}

void ShopScreen::onRemoveFromDeck() {
    if (m_player.getHp() <= 5) {
        m_statusLabel->setText("⚠ НЕДОСТАТОЧНО HP ДЛЯ УДАЛЕНИЯ!");
        m_statusLabel->setStyleSheet("color: #E94560;");
        return;
    }

    // Удаляем случайную карту (в идеале — выбор игрока, но для MVP сойдёт)
    auto& master = m_player.getDeck().getMasterDeck();
    if (master.empty()) return;

    int idx = m_rng.rollInt(0, static_cast<int>(master.size()) - 1);
    std::string removed = master[idx]->getName();
    std::string removedId = master[idx]->getId();

    m_player.getDeck().removeCardFromMasterDeck(removedId);
    m_player.takeDamage(5, true);

    GameLog::instance().addMessage(
        QString("🗑 Удалена карта: '%1' (-5 HP)")
            .arg(QString::fromStdString(removed)),
        "#533483");

    m_statusLabel->setText(QString("✓ Удалена: %1").arg(QString::fromStdString(removed)));
    m_statusLabel->setStyleSheet("color: #00FF41;");
    rebuildUI();
}

void ShopScreen::onExit() {
    GameLog::instance().addMessage("💾 Покинули чёрный рынок", "#808080");
    emit shopExited();
}
